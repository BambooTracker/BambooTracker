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

#include "ppc_io.hpp"
#include <vector>
#include "instrument.hpp"
#include "file_io_error.hpp"

namespace io
{
PpcIO::PpcIO() : AbstractBankIO("ppc", "PMD PPC", true, false) {}

AbstractBank* PpcIO::load(const BinaryContainer& ctr) const
{
	size_t globCsr = 0;
	if (ctr.readString(globCsr, 30) != "ADPCM DATA for  PMD ver.4.4-  ")
		throw FileCorruptionError(FileType::Bank, globCsr);
	globCsr += 30;
	uint16_t nextAddr = ctr.readUint16(globCsr);
	if ((nextAddr - 0x26u) * 0x20u + 0x420u != ctr.size())	// File size check
		throw FileCorruptionError(FileType::Bank, globCsr);
	globCsr += 2;

	size_t sampOffs = globCsr + 256 * 4;
	if (ctr.size() < sampOffs) throw FileCorruptionError(FileType::Bank, globCsr);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	size_t offs = 0;
	constexpr int MAX_CNT = 256;
	for (int i = 0; i < MAX_CNT; ++i) {
		uint16_t start = ctr.readUint16(globCsr);
		globCsr += 2;
		uint16_t stop = ctr.readUint16(globCsr);
		globCsr += 2;

		if (start < stop) {
			if (ids.empty()) offs = start;
			ids.push_back(i);
			size_t st = sampOffs + static_cast<size_t>((start - offs) << 5);
			size_t sampSize = std::min<size_t>((stop + 1u - start) << 5, ctr.size() - st);
			samples.push_back(ctr.getSubcontainer(st, sampSize).toVector());
		}
	}

	return new PpcBank(ids, samples);
}

AbstractInstrument* PpcIO::loadInstrument(const std::vector<uint8_t>& sample,
										  std::weak_ptr<InstrumentsManager> instMan,
										  int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int sampIdx = instManLocked->findFirstAssignableSampleADPCM();
	if (sampIdx < 0) throw FileCorruptionError(FileType::Bank, 0);

	InstrumentADPCM* adpcm = new InstrumentADPCM(instNum, "", instManLocked.get());
	adpcm->setSampleNumber(sampIdx);

	instManLocked->storeSampleADPCMRawSample(sampIdx, sample);
	instManLocked->setSampleADPCMRootKeyNumber(sampIdx, 67);	// o5g
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, 0x49cd);	// 16000Hz
	instManLocked->setSampleADPCMRepeatEnabled(sampIdx, false);
	instManLocked->setSampleADPCMRepeatrange(sampIdx, SampleRepeatRange(0, (sample.size() - 1) >> 5));

	return adpcm;
}
}
