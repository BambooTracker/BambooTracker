/*
 * Copyright (C) 2020-2023 Rerrah
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

#include "pvi_io.hpp"
#include <vector>
#include "instrument.hpp"
#include "file_io_error.hpp"

namespace io
{
PviIO::PviIO() : AbstractBankIO("pvi", "FMP PVI", true, false) {}

AbstractBank* PviIO::load(const BinaryContainer& ctr) const
{
	std::string ident = ctr.readString(0, 4);
	if (ident != "PVI1" && ident != "PVI2")
		throw FileCorruptionError(FileType::Bank, 0);
	uint16_t deltaN = ctr.readUint16(8);
	uint8_t cnt = ctr.readUint8(11);

	constexpr size_t sampOffs = 0x10 + 128 * 4;
	if (ctr.size() < sampOffs) throw FileCorruptionError(FileType::Bank, 0x10);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	size_t offs = 0;
	size_t addrPos = 0x10;
	for (size_t i = 0; i < cnt; ++i) {
		uint16_t start = ctr.readUint16(addrPos);
		addrPos += 2;
		uint16_t stop = ctr.readUint16(addrPos);
		addrPos += 2;

		if (start < stop) {
			if (ids.empty()) offs = start;
			ids.push_back(static_cast<int>(i));
			size_t st = sampOffs + static_cast<size_t>((start - offs) << 5);
			size_t sampSize = std::min<size_t>((stop + 1u - start) << 5, ctr.size() - st);
			samples.push_back(ctr.getSubcontainer(st, sampSize).toVector());
		}
	}
	/* if (ids.size() != cnt) throw FileCorruptionError(FileType::Bank, 11); */

	return new PviBank(ids, deltaN, samples);
}

AbstractInstrument* PviIO::loadInstrument(const std::vector<uint8_t>& sample, uint16_t deltaN,
										  std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int sampIdx = instManLocked->findFirstAssignableSampleADPCM();
	if (sampIdx < 0) throw FileCorruptionError(FileType::Bank, 0);

	InstrumentADPCM* adpcm = new InstrumentADPCM(instNum, "", instManLocked.get());
	adpcm->setSampleNumber(sampIdx);

	instManLocked->storeSampleADPCMRawSample(sampIdx, sample);
	instManLocked->setSampleADPCMRootKeyNumber(sampIdx, 60);	// o5c
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, deltaN);
	instManLocked->setSampleADPCMRepeatEnabled(sampIdx, false);
	instManLocked->setSampleADPCMRepeatrange(sampIdx, SampleRepeatRange(0, (sample.size() - 1) >> 5));

	return adpcm;
}
}
