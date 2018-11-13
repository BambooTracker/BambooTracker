#include "configuration.hpp"

Configuration::Configuration()
{
	// General //
	// General settings
	warpCursor_ = true;
	warpAcrossOrders_ = true;
	showRowNumHex_ = true;
	showPrevNextOrders_ = true;
	backupModules_ = true;

	// Edit settings
	pageJumpLength_ = 4;

	// Sonud //
	sampleRate_ = 44100;
	bufferLength_ = 40;
}

// General //
// General settings
void Configuration::setWarpCursor(bool enabled)
{
	warpCursor_ = enabled;
}

bool Configuration::getWarpCursor() const
{
	return warpCursor_;
}

void Configuration::setWarpAcrossOrders(bool enabled)
{
	warpAcrossOrders_ = enabled;
}

bool Configuration::getWarpAcrossOrders() const
{
	return warpAcrossOrders_;
}

void Configuration::setShowRowNumberInHex(bool enabled)
{
	showRowNumHex_ = enabled;
}

bool Configuration::getShowRowNumberInHex() const
{
	return showRowNumHex_;
}

void Configuration::setShowPrevNextOrders(bool enabled)
{
	showPrevNextOrders_ = enabled;
}

bool Configuration::getShowPrevNextOrders() const
{
	return showPrevNextOrders_;
}

void Configuration::setBackupModules(bool enabled)
{
	backupModules_ = enabled;
}

bool Configuration::getBackupModules() const
{
	return backupModules_;
}

void Configuration::setDontSelectOnDoubleClick(bool enabled)
{
	dontSelectOnDoubleClick_ = enabled;
}

bool Configuration::getDontSelectOnDoubleClick() const
{
	return dontSelectOnDoubleClick_;
}

// Edit settings
void Configuration::setPageJumpLength(size_t length)
{
	pageJumpLength_ = length;
}

size_t Configuration::getPageJumpLength() const
{
	return pageJumpLength_;
}

// Sound //
void Configuration::setSampleRate(uint32_t rate)
{
	sampleRate_ = rate;
}

uint32_t Configuration::getSampleRate() const
{
	return sampleRate_;
}

void Configuration::setBufferLength(size_t length)
{
	bufferLength_ = length;
}

size_t Configuration::getBufferLength() const
{
	return bufferLength_;
}
