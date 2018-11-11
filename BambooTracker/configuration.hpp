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

private:

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
