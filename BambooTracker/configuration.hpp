#pragma once

#include <cstdint>
#include <cstddef>

class Configuration
{
public:
	Configuration();

	// General //
	// General settings
public:
	void setWarpCursor(bool enabled);
	bool getWarpCursor() const;
	void setWarpAcrossOrders(bool enabled);
	bool getWarpAcrossOrders() const;
	void setShowRowNumberInHex(bool enabled);
	bool getShowRowNumberInHex() const;
	void setShowPrevNextOrders(bool enabled);
	bool getShowPrevNextOrders() const;
	void setBackupModules(bool enabled);
	bool getBackupModules() const;
	void setDontSelectOnDoubleClick(bool enabled);
	bool getDontSelectOnDoubleClick() const;
private:
	bool warpCursor_, warpAcrossOrders_;
	bool showRowNumHex_, showPrevNextOrders_;
	bool backupModules_, dontSelectOnDoubleClick_;

	// Edit settings
public:
	void setPageJumpLength(size_t length);
	size_t getPageJumpLength() const;
private:
	size_t pageJumpLength_;

	// Sound //
public:
	void setSampleRate(uint32_t rate);
	uint32_t getSampleRate() const;
	void setBufferLength(size_t length);
	size_t getBufferLength() const;
private:
	uint32_t sampleRate_;
	size_t bufferLength_;
};
