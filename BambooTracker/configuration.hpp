#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <map>
#include <vector>
#include "misc.hpp"

class Configuration
{
public:
	Configuration();

	// Internal //
public:
	void setMainWindowWidth(int w);
	int getMainWindowWidth() const;
	void setMainWindowHeight(int h);
	int getMainWindowHeight() const;
	void setMainWindowMaximized(bool isMax);
	bool getMainWindowMaximized() const;
	void setMainWindowX(int x);
	int getMainWindowX() const;
	void setMainWindowY(int y);
	int getMainWindowY() const;
	void setInstrumentFMWindowWidth(int w);
	int getInstrumentFMWindowWidth() const;
	void setInstrumentFMWindowHeight(int h);
	int getInstrumentFMWindowHeight() const;
	void setInstrumentSSGWindowWidth(int w);
	int getInstrumentSSGWindowWidth() const;
	void setInstrumentSSGWindowHeight(int h);
	int getInstrumentSSGWindowHeight() const;
	void setFollowMode(bool enabled);
	bool getFollowMode() const;
	void setWorkingDirectory(std::string path);
	std::string getWorkingDirectory() const;
private:
	int mainW_, mainH_;
	bool mainMax_;
	int mainX_, mainY_;
	int instFMW_, instFMH_;
	int instSSGW_, instSSGH_;
	bool followMode_;
	std::string workDir_;

	// General //
	// General settings
public:
	void setWarpCursor(bool enabled);
	bool getWarpCursor() const;
	void setWarpAcrossOrders(bool enabled);
	bool getWarpAcrossOrders() const;
	void setShowRowNumberInHex(bool enabled);
	bool getShowRowNumberInHex() const;
	void setShowPreviousNextOrders(bool enabled);
	bool getShowPreviousNextOrders() const;
	void setBackupModules(bool enabled);
	bool getBackupModules() const;
	void setDontSelectOnDoubleClick(bool enabled);
	bool getDontSelectOnDoubleClick() const;
	void setReverseFMVolumeOrder(bool enabled);
	bool getReverseFMVolumeOrder() const;
	void setMoveCursorToRight(bool enabled);
	bool getMoveCursorToRight() const;
private:
	bool warpCursor_, warpAcrossOrders_;
	bool showRowNumHex_, showPrevNextOrders_;
	bool backupModules_, dontSelectOnDoubleClick_;
	bool reverseFMVolumeOrder_, moveCursorToRight_;

	// Edit settings
public:
	void setPageJumpLength(size_t length);
	size_t getPageJumpLength() const;
	void setEditableStep(size_t step);
	size_t getEditableStep() const;
private:
	size_t pageJumpLength_, editableStep_;

	// Keys
public:
	void setKeyOffKey(std::string key);
	std::string getKeyOffKey() const;
	void setOctaveUpKey(std::string key);
	std::string getOctaveUpKey() const;
	void setOctaveDownKey(std::string key);
	std::string getOctaveDownKey() const;
	void setEchoBufferKey(std::string key);
	std::string getEchoBufferKey() const;

private:
	std::string keyOffKey_, octUpKey_, octDownKey_, echoKey_;

	// Sound //
public:
	void setSoundDevice(std::string device);
	std::string getSoundDevice() const;
	void setUseSCCI(bool enabled);
	bool getUseSCCI() const;
	void setSampleRate(uint32_t rate);
	uint32_t getSampleRate() const;
	void setBufferLength(size_t length);
	size_t getBufferLength() const;
private:
	std::string sndDevice_;
	bool useSCCI_;
	uint32_t sampleRate_;
	size_t bufferLength_;

	// Mixer //
public:
	void setMixerVolumeMaster(int percentage);
	int getMixerVolumeMaster() const;
	void setMixerVolumeFM(double dB);
	double getMixerVolumeFM() const;
	void setMixerVolumeSSG(double dB);
	double getMixerVolumeSSG() const;
private:
	int mixerVolumeMaster_;
	double mixerVolumeFM_, mixerVolumeSSG_;

	// Input //
public:
	void setFMEnvelopeTextMap(std::map<std::string, std::vector<FMEnvelopeTextType>> map);
	std::map<std::string, std::vector<FMEnvelopeTextType>> getFMEnvelopeTextMap() const;

private:
	std::map<std::string, std::vector<FMEnvelopeTextType>> fmEnvelopeTextMap_;
};
