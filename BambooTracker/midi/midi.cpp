#include "midi.hpp"
#include "midi_def.h"
#include <stdio.h>

std::unique_ptr<MidiInterface> MidiInterface::instance_;

MidiInterface &MidiInterface::instance()
{
	if (instance_)
		return *instance_;

	MidiInterface *out = new MidiInterface;
	instance_.reset(out);
	return *out;
}

MidiInterface::MidiInterface()
	: hasInitializedMidiIn_(false),
	  hasInitializedMidiOut_(false),
	  hasOpenInputPort_(false),
	  hasOpenOutputPort_(false)
{
	switchApi(RtMidi::UNSPECIFIED);
}

MidiInterface::~MidiInterface()
{
}

RtMidi::Api MidiInterface::currentApi() const
{
	return inputClient_->getCurrentApi();
}

void MidiInterface::switchApi(RtMidi::Api api)
{
	if (inputClient_ && api == inputClient_->getCurrentApi())
		return;

	RtMidiIn *inputClient = nullptr;
	try {
		inputClient = new RtMidiIn(api, MIDI_INP_CLIENT_NAME, MidiBufferSize);
		hasInitializedMidiIn_ = true;
	}
	catch (RtMidiError &error) {
		hasInitializedMidiIn_ = false;
		fprintf(stderr, "Cannot initialize MIDI In.\n");
	}
	if (!inputClient)
		inputClient = new RtMidiIn(RtMidi::RTMIDI_DUMMY, MIDI_INP_CLIENT_NAME, MidiBufferSize);

	inputClient->ignoreTypes(MIDI_INP_IGNORE_SYSEX, MIDI_INP_IGNORE_TIME, MIDI_INP_IGNORE_SENSE);
	inputClient_.reset(inputClient);
	inputClient->setErrorCallback(&onMidiError, this);
	inputClient->setCallback(&onMidiInput, this);
	hasOpenInputPort_ = false;

	RtMidiOut *outputClient = nullptr;
	try {
		outputClient = new RtMidiOut(api, MIDI_OUT_CLIENT_NAME);
		hasInitializedMidiOut_ = false;
	}
	catch (RtMidiError &error) {
		hasInitializedMidiOut_ = true;
		fprintf(stderr, "Cannot initialize MIDI Out.\n");
	}
	if (!outputClient)
		outputClient = new RtMidiOut(RtMidi::RTMIDI_DUMMY, MIDI_OUT_CLIENT_NAME);

	outputClient_.reset(outputClient);
	outputClient->setErrorCallback(&onMidiError, this);
	hasOpenOutputPort_ = false;
}

bool MidiInterface::supportsVirtualPort() const
{
	switch (currentApi()) {
	case RtMidi::MACOSX_CORE: case RtMidi::LINUX_ALSA: case RtMidi::UNIX_JACK:
		return true;
	default:
		return false;
	}
}

std::vector<std::string> MidiInterface::getRealInputPorts()
{
	RtMidiIn &client = *inputClient_;
	unsigned count = client.getPortCount();

	std::vector<std::string> ports;
	ports.reserve(count);

	for (unsigned i = 0; i < count; ++i)
		ports.push_back(client.getPortName(i));
	return ports;
}

std::vector<std::string> MidiInterface::getRealOutputPorts()
{
	RtMidiOut &client = *outputClient_;
	unsigned count = client.getPortCount();

	std::vector<std::string> ports;
	ports.reserve(count);

	for (unsigned i = 0; i < count; ++i)
		ports.push_back(client.getPortName(i));
	return ports;
}

bool MidiInterface::hasInitializedInput() const
{
	return hasInitializedMidiIn_;
}

bool MidiInterface::hasInitializedOutput() const
{
	return hasInitializedMidiOut_;
}

void MidiInterface::closeInputPort()
{
	RtMidiIn &client = *inputClient_;
	if (hasOpenInputPort_) {
		client.closePort();
		hasOpenInputPort_ = false;
	}
}

void MidiInterface::closeOutputPort()
{
	RtMidiOut &client = *outputClient_;
	if (hasOpenOutputPort_) {
		client.closePort();
		hasOpenOutputPort_ = false;
	}
}

void MidiInterface::openInputPort(unsigned port)
{
	RtMidiIn &client = *inputClient_;
	closeInputPort();

	std::string name = MIDI_INP_PORT_NAME;
	if (port == ~0u) {
		client.openVirtualPort(name);
		hasOpenInputPort_ = true;
	}
	else {
		client.openPort(port, name);
		hasOpenInputPort_ = client.isPortOpen();
	}
}

void MidiInterface::openOutputPort(unsigned port)
{
	RtMidiOut &client = *outputClient_;
	closeOutputPort();

	std::string name = MIDI_OUT_PORT_NAME;
	if (port == ~0u) {
		client.openVirtualPort(name);
		hasOpenOutputPort_ = true;
	}
	else {
		client.openPort(port, name);
		hasOpenOutputPort_ = client.isPortOpen();
	}
}

void MidiInterface::openInputPortByName(const std::string &portName)
{
	std::vector<std::string> ports = getRealInputPorts();

	for (unsigned i = 0, n = ports.size(); i < n; ++i) {
		if (ports[i] == portName) {
			openInputPort(i);
			return;
		}
	}
}

void MidiInterface::openOutputPortByName(const std::string &portName)
{
	std::vector<std::string> ports = getRealOutputPorts();

	for (unsigned i = 0, n = ports.size(); i < n; ++i) {
		if (ports[i] == portName) {
			openOutputPort(i);
			return;
		}
	}
}

void MidiInterface::sendMessage(const uint8_t *data, size_t length)
{
	RtMidiOut &client = *outputClient_;
	if (hasOpenOutputPort_)
		client.sendMessage(data, length);
}

void MidiInterface::onMidiError(RtMidiError::Type type, const std::string &text, void *user_data)
{
	MidiInterface *self = reinterpret_cast<MidiInterface *>(user_data);

	(void)type;
	(void)self;

	fprintf(stderr, "[Midi Out] %s\n", text.c_str());
}

void MidiInterface::installInputHandler(InputHandler *handler, void *user_data)
{
	std::lock_guard<std::mutex> lock(inputHandlersMutex_);
	inputHandlers_.push_back(std::make_pair(handler, user_data));
}

void MidiInterface::uninstallInputHandler(InputHandler *handler, void *user_data)
{
	std::lock_guard<std::mutex> lock(inputHandlersMutex_);
	for (size_t i = 0, n = inputHandlers_.size(); i < n; ++i) {
		bool match = inputHandlers_[i].first == handler &&
			inputHandlers_[i].second == user_data;
		if (match) {
			inputHandlers_.erase(inputHandlers_.begin() + i);
			return;
		}
	}
}

void MidiInterface::onMidiInput(double timestamp, std::vector<unsigned char> *message, void *user_data)
{
	MidiInterface *self = reinterpret_cast<MidiInterface *>(user_data);

	std::unique_lock<std::mutex> lock(self->inputHandlersMutex_, std::try_to_lock);
	if (!lock.owns_lock())
		return;

	const uint8_t *msg = message->data();
	size_t len = message->size();

	for (size_t i = 0, n = self->inputHandlers_.size(); i < n; ++i)
		self->inputHandlers_[i].first(timestamp, msg, len, self->inputHandlers_[i].second);
}
