/*
 * Copyright (C) 2022 Rerrah
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

#include "ymfm_2608.hpp"

extern const unsigned char YM2608_ADPCM_ROM[0x2000];

namespace chip
{
Ymfm2608::YmfmInterface::YmfmInterface(uint32_t dramSize) : dram_(dramSize) {}

uint8_t Ymfm2608::YmfmInterface::ymfm_external_read(ymfm::access_class type, uint32_t address)
{
	switch (type) {
	case ymfm::access_class::ACCESS_ADPCM_A:
		return address < 0x2000 ? YM2608_ADPCM_ROM[address] : 0;
	case ymfm::access_class::ACCESS_ADPCM_B:
		return address < dram_.size() ? dram_[address] : 0;
	default:
		return 0;
	}
}

void Ymfm2608::YmfmInterface::ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data)
{
	if (type == ymfm::access_class::ACCESS_ADPCM_B && address < dram_.size()) {
		dram_[address] = data;
	}
}

//**************************************************
Ymfm2608::~Ymfm2608()
{
	stopDevice();
}

int Ymfm2608::startDevice(int clock, int& rateSsg, uint32_t dramSize)
{
	ymfmIntf_ = std::make_unique<YmfmInterface>(dramSize);
	ymfm_ = std::make_unique<ymfm::ym2608>(*ymfmIntf_);
	// Prescale = 6
	// Set FM output rate
	ymfm_->set_fidelity(ymfm::opn_fidelity::OPN_FIDELITY_MIN);
	rateSsg = clock / 32;

	ymfm_->reset();

	return clock / 144;	// FM synthesis rate is clock / 2 / 72
}

void Ymfm2608::stopDevice()
{
	ymfm_.reset();
}

void Ymfm2608::resetDevice()
{
	ymfm_->reset();
}

void Ymfm2608::writeAddressToPortA(uint8_t address)
{
	ymfm_->write_address(address);
}

void Ymfm2608::writeAddressToPortB(uint8_t address)
{
	ymfm_->write_address_hi(address);
}

void Ymfm2608::writeDataToPortA(uint8_t data)
{
	ymfm_->write_data(data);
}

void Ymfm2608::writeDataToPortB(uint8_t data)
{
	ymfm_->write_data_hi(data);
}

uint8_t Ymfm2608::readData()
{
	return ymfm_->read_data();
}

void Ymfm2608::updateStream(sample** outputs, int nSamples)
{
	sample* bufl = outputs[STEREO_LEFT];
	sample* bufr = outputs[STEREO_RIGHT];

	ymfm::ym2608::output_data data;
	for (int i = 0; i < nSamples; ++i) {
		ymfm_->generate_fm_adpcm(&data);
		// Raise volume
		*bufl++ = data.data[0] << 1;
		*bufr++ = data.data[1] << 1;
	}
}

void Ymfm2608::updateSsgStream(sample** outputs, int nSamples)
{
	sample* bufl = outputs[STEREO_LEFT];
	sample* bufr = outputs[STEREO_RIGHT];

	ymfm::ym2608::output_data data;
	for (int i = 0; i < nSamples; ++i) {
		ymfm_->generate_ssg(&data);
		// Modify volume
		int32_t s = data.data[2] * 3 / 4;
		*bufl++ = s;
		*bufr++ = s;
	}
}
}
