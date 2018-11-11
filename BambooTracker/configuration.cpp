#include "configuration.hpp"

Configuration::Configuration()
{
	// General //
	// General settings

	// Edit settings
	pageJumpLength_ = 4;

	// Sonud //
	sampleRate_ = 44100;
	bufferLength_ = 40;
}

// General //
// General settings

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
