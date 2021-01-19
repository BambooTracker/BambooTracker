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

#include "pvi_io.hpp"
#include <vector>
#include "instrument.hpp"
#include "file_io_error.hpp"
#include "io_utils.hpp"

namespace io
{
PviIO::PviIO() : AbstractBankIO("pvi", "FMP PVI", true, false) {}

AbstractBank* PviIO::load(const BinaryContainer& ctr) const
{
	size_t globCsr = 0;
	std::string ident = ctr.readString(globCsr, 4);
	if (ident != "PVI1" && ident != "PVI2")
		throw FileCorruptionError(FileType::Bank, globCsr);
	globCsr += 0x10;

	size_t sampOffs = globCsr + 128 * 4;
	if (ctr.size() < sampOffs) throw FileCorruptionError(FileType::Bank, globCsr);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	extractADPCMSamples(ctr, globCsr, sampOffs, 128, ids, samples);

	return new PviBank(ids, samples);
}

AbstractInstrument* PviIO::loadInstrument(const std::vector<uint8_t>& sample,
										  std::weak_ptr<InstrumentsManager> instMan,
										  int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int sampIdx = instManLocked->findFirstAssignableSampleADPCM();
	if (sampIdx < 0) throw FileCorruptionError(FileType::Bank, 0);

	InstrumentADPCM* adpcm = new InstrumentADPCM(instNum, "", instManLocked.get());
	adpcm->setSampleNumber(sampIdx);

	instManLocked->storeSampleADPCMRawSample(sampIdx, sample);
	instManLocked->setSampleADPCMRootKeyNumber(sampIdx, 60);	// o5c
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, 0x49cd);	// 16000Hz

	return adpcm;
}
}
