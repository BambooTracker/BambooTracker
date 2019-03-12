#pragma once
#include "RtMidi/RtMidi.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <stdint.h>

class MidiInterface
{
public:
	static MidiInterface &instance();
	~MidiInterface();

private:
	MidiInterface();

public:
	RtMidi::Api currentApi() const;
	void switchApi(RtMidi::Api api);
	bool supportsVirtualPort() const;
	std::vector<std::string> getRealInputPorts();
	std::vector<std::string> getRealOutputPorts();

	void closeInputPort();
	void closeOutputPort();
	void openInputPort(unsigned port);
	void openOutputPort(unsigned port);
	void openInputPortByName(const std::string &portName);
	void openOutputPortByName(const std::string &portName);

	typedef void (InputHandler)(double, const uint8_t *, size_t, void *);
	void installInputHandler(InputHandler *handler, void *userData);
	void uninstallInputHandler(InputHandler *handler, void *userData);

	void sendMessage(const uint8_t *data, size_t length);

private:
	static void onMidiError(RtMidiError::Type type, const std::string &text, void *userData);
	static void onMidiInput(double timestamp, std::vector<unsigned char> *message, void *userData);

	std::unique_ptr<RtMidiIn> inputClient_;
	std::unique_ptr<RtMidiOut> outputClient_;
	bool hasOpenInputPort_ = false;
	bool hasOpenOutputPort_ = false;
	std::mutex inputHandlersMutex_;
	std::vector<std::pair<InputHandler *, void *>> inputHandlers_;

	static std::unique_ptr<MidiInterface> instance_;
};
