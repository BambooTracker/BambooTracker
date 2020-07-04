#pragma once
#include "RtMidi/RtMidi.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>

class MidiInterface
{
public:
	static MidiInterface &instance();
	~MidiInterface();

private:
	MidiInterface();

public:
	RtMidi::Api currentApi() const;
	std::string currentApiName() const;
	std::vector<std::string> getAvailableApi() const;
	void switchApi(std::string api);
	void switchApi(RtMidi::Api api);
	bool supportsVirtualPort() const;
	bool supportsVirtualPort(std::string api) const;
	std::vector<std::string> getRealInputPorts();
	std::vector<std::string> getRealInputPorts(const std::string& api);
	std::vector<std::string> getRealOutputPorts();

	bool hasInitializedInput() const;
	bool hasInitializedOutput() const;

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
	bool hasInitializedMidiIn_, hasInitializedMidiOut_;
	bool hasOpenInputPort_, hasOpenOutputPort_;
	std::mutex inputHandlersMutex_;
	std::vector<std::pair<InputHandler *, void *>> inputHandlers_;

	static std::unique_ptr<MidiInterface> instance_;
};
