/*
 * Copyright (C) 2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nuked_2608.hpp"
#include <cstdlib>
#include <algorithm>

namespace chip
{
namespace // SSG callbacks
{
void setSsgClock(void *param, int clock)
{
	auto state = reinterpret_cast<Nuked2608State*>(param);
	if (state->ssg) PSG_set_clock(state->ssg, clock);
}

void writeSsg(void *param, int address, int data)
{
	auto state = reinterpret_cast<Nuked2608State*>(param);
	if (state->ssg) PSG_writeIO(state->ssg, address, data);
}

int readSsg(void *param)
{
	auto state = reinterpret_cast<Nuked2608State*>(param);
	return (state->ssg ? PSG_readIO(state->ssg) : 0);
}

void resetSsg(void *param)
{
	auto state = reinterpret_cast<Nuked2608State*>(param);
	if (state->ssg) PSG_reset(state->ssg);
}

const struct OPN2mod_psg_callbacks SSG_INTF =
{
	setSsgClock,
	writeSsg,
	readSsg,
	resetSsg
};
}

Nuked2608::~Nuked2608()
{
	stopDevice();
}

int Nuked2608::startDevice(int clock, int& rateSsg, uint32_t dramSize)
{
	int clockSsg = clock / 4;
	rateSsg = clockSsg / 8;
	state_.ssg = PSG_new(clockSsg, rateSsg);
	if (!state_.ssg) return 0;
	PSG_setVolumeMode(state_.ssg, 1);	// YM2149 volume mode

	state_.chip = reinterpret_cast<ym3438_t*>(std::calloc(1, sizeof(ym3438_t)));
	if (!state_.chip) {
		stopDevice();
		return 0;
	}

	state_.clock = clock;
	state_.dramSize = dramSize;
	OPN2_Reset(state_.chip, clock, &SSG_INTF, &state_, dramSize);

	return clock / 144;	// FM synthesis rate is clock / 2 / 72
}

void Nuked2608::stopDevice()
{
	if (state_.chip) {
		OPN2_Destroy(state_.chip);
		std::free(state_.chip);
		state_.chip = nullptr;
	}
	if (state_.ssg)
	{
		PSG_delete(state_.ssg);
		state_.ssg = nullptr;
	}
}

void Nuked2608::resetDevice()
{
	OPN2_FlushBuffer(state_.chip);
	OPN2_Reset(state_.chip, state_.clock, &SSG_INTF, &state_, state_.dramSize);
}

void Nuked2608::writeAddressToPortA(uint8_t address)
{
	OPN2_WriteBuffered(state_.chip, 0, address);
}

void Nuked2608::writeAddressToPortB(uint8_t address)
{
	OPN2_WriteBuffered(state_.chip, 2, address);
}

void Nuked2608::writeDataToPortA(uint8_t data)
{
	OPN2_WriteBuffered(state_.chip, 1, data);
}

void Nuked2608::writeDataToPortB(uint8_t data)
{
	OPN2_WriteBuffered(state_.chip, 3, data);
}

uint8_t Nuked2608::readData()
{
	return OPN2_Read(state_.chip, 1);
}

void Nuked2608::updateStream(sample** outputs, int nSamples)
{
	sample* bufl = outputs[STEREO_LEFT];
	sample* bufr = outputs[STEREO_RIGHT];

	for (int i = 0; i < nSamples; ++i) {
		sample lr[2];
		OPN2_Generate(state_.chip, lr);
		*bufl++ = lr[0];
		*bufr++ = lr[1];
	}
}

void Nuked2608::updateSsgStream(sample** outputs, int nSamples)
{
	if (state_.ssg) {
		sample* bufl = outputs[STEREO_LEFT];
		sample* bufr = outputs[STEREO_RIGHT];
		for (int i = 0; i < nSamples; ++i) {
			int16_t s = PSG_calc(state_.ssg) << 1;
			*bufl++ = s;
			*bufr++ = s;
		}
	}
	else {
		std::fill_n(outputs[STEREO_LEFT], nSamples, 0);
		std::fill_n(outputs[STEREO_RIGHT], nSamples, 0);
	}
}
}
