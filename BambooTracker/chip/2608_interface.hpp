#pragma once

#include <stdint.h>
#include "chip_defs.h"

namespace chip
{
class Ym2608Interface
{
public:
	virtual ~Ym2608Interface() = default;
	virtual int startDevice(int clock, int& rateSsg, uint32_t dramSize) = 0;
	virtual void stopDevice() = 0;
	virtual void resetDevice() = 0;
	virtual void writeAddressToPortA(uint8_t address) = 0;
	virtual void writeAddressToPortB(uint8_t address) = 0;
	virtual void writeDataToPortA(uint8_t data) = 0;
	virtual void writeDataToPortB(uint8_t data) = 0;
	virtual uint8_t readData() = 0;
	virtual void updateStream(sample** outputs, int nSamples) = 0;
	virtual void updateSsgStream(sample** outputs, int nSamples) = 0;
};
}
