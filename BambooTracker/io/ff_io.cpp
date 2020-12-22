/*
 * Copyright (C) 2020-2021 Rerrah
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

#include "ff_io.hpp"
#include <vector>
#include <algorithm>
#include "instrument.hpp"
#include "file_io_error.hpp"

namespace io
{
FfIO::FfIO() : AbstractBankIO("ff", "PMD FF", true, false) {}

AbstractBank* FfIO::load(const BinaryContainer& ctr) const
{
	size_t csr = 0;
	// File size check
	size_t ctrSize = ctr.size();
	if (!ctrSize || ctrSize & 0x1f || ctrSize > 0x2000)
		throw FileCorruptionError(FileType::Bank, csr);

	std::vector<int> ids;
	std::vector<std::string> names;
	std::vector<BinaryContainer> ctrs;
	int max = static_cast<int>(ctrSize / 0x20);
	for (int i = 0; i < max; ++i) {
		BinaryContainer block = ctr.getSubcontainer(csr, 25);
		csr += 25;
		std::string name = "";
		for (size_t j = 0; j < 7; ++j) {
			if (char c = ctr.readChar(csr + j)) name += c;
			else break;
		}
		csr += 7;

		// Empty
		if (std::all_of(block.begin(), block.end(),
						[](const char c) { return c == 0; }) && name.empty())
			continue;

		ids.push_back(i);
		names.push_back(name);
		ctrs.push_back(block);
	}

	return new FfBank(ids, names, ctrs);
}

AbstractInstrument* FfIO::loadInstrument(const BinaryContainer& instCtr,
										 const std::string& name,
										 std::weak_ptr<InstrumentsManager> instMan,
										 int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileType::Bank, 0);

	InstrumentFM* fm = new InstrumentFM(instNum, name, instManLocked.get());
	fm->setEnvelopeNumber(envIdx);

	uint8_t tmp;
	size_t initCsr[] = { 0, 2, 1, 3 };
	for (int op = 0; op < 4; ++op) {
		size_t csr = initCsr[op];
		auto& params = FM_OP_PARAMS[op];
		tmp = instCtr.readUint8(csr);
		uint8_t ssgeg = (tmp & 0x80) ? 8 : 0;
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DT), (tmp & 0x70) >> 4);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::ML), tmp & 0x0f);
		csr += 4;
		tmp = instCtr.readUint8(csr);
		if (tmp & 0x80) ssgeg |= 4;
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::TL), tmp & 0x7f);
		csr += 4;
		tmp = instCtr.readUint8(csr);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::KS), tmp >> 6);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::AR), tmp & 0x1f);
		csr += 4;
		tmp = instCtr.readUint8(csr);
		ssgeg |= ((tmp & 0x60) >> 5);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SSGEG),
											  (ssgeg & 8) ? ssgeg & 7 : -1);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DR), tmp & 0x1f);
		csr += 4;
		tmp = instCtr.readUint8(csr);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SR), tmp);
		csr += 4;
		tmp = instCtr.readUint8(csr);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SL), tmp >> 4);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::RR), tmp & 0x0f);
	}
	tmp = instCtr.readUint8(24);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, tmp >> 3);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, tmp & 0x07);

	return fm;
}
}
