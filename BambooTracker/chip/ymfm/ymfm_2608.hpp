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

#pragma once

#include "../2608_interface.hpp"
#include <vector>
#include <memory>
#include "ymfm.h"
#include "ymfm_opn.h"

namespace chip
{
class Ymfm2608 final : public Ym2608Interface
{
public:
	~Ymfm2608() override;
	int startDevice(int clock, int& rateSsg, uint32_t dramSize) override;
	void stopDevice() override;
	void resetDevice() override;
	void writeAddressToPortA(uint8_t address) override;
	void writeAddressToPortB(uint8_t address) override;
	void writeDataToPortA(uint8_t data) override;
	void writeDataToPortB(uint8_t data) override;
	uint8_t readData() override;
	void updateStream(sample** outputs, int nSamples) override;
	void updateSsgStream(sample** outputs, int nSamples) override;

private:
	class YmfmInterface final : public ymfm::ymfm_interface
	{
	public:
		YmfmInterface(uint32_t dramSize);
		uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override;
		void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data) override;

	private:
		std::vector<uint8_t> dram_;
	};

	std::unique_ptr<ymfm::ym2608> ymfm_;
	std::unique_ptr<YmfmInterface> ymfmIntf_;
};
}
