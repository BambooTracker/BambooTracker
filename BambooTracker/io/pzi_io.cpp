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

#include "pzi_io.hpp"
#include <vector>
#include <limits>
#include "instrument.hpp"
#include "file_io_error.hpp"
#include "chip/codec/ymb_codec.hpp"

namespace io
{
PziIO::PziIO() : AbstractBankIO("pzi", "FMP PZI", true, false) {}

AbstractBank* PziIO::load(const BinaryContainer& ctr) const
{
	std::string ident = ctr.readString(0, 4);
	if (ident != "PZI0" && ident != "PZI1")
		throw FileCorruptionError(FileType::Bank, 0);

	constexpr size_t SAMP_OFFS = 0x920;
	if (ctr.size() < SAMP_OFFS) throw FileCorruptionError(FileType::Bank, 0x20);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	std::vector<bool> isRepeatedList;
	std::vector<int> deltaNs;
	size_t globCsr = 0x20;
	constexpr int MAX_CNT = 128;
	for (int i = 0; i < MAX_CNT; ++i) {
		uint32_t start = ctr.readUint32(globCsr);
		globCsr += 4;
		uint32_t len = ctr.readUint32(globCsr);
		globCsr += 4;
		uint32_t loopStart = ctr.readUint32(globCsr);
		globCsr += 4;
		uint32_t loopEnd = ctr.readUint32(globCsr);
		globCsr += 4;
		uint16_t sr = ctr.readUint16(globCsr);
		globCsr += 2;

		// only support loop within entire region
		bool isRepeated = (!loopStart && len == loopEnd);

		if (len) {
			ids.push_back(i);
			isRepeatedList.push_back(isRepeated);
			deltaNs.push_back(SampleADPCM::calculateADPCMDeltaN(sr));

			std::vector<uint8_t>&& smp = ctr.getSubcontainer(SAMP_OFFS + start, len).toVector();
			std::vector<int16_t> buf(smp.size());
			std::transform(smp.begin(), smp.end(), buf.begin(), [](uint8_t v) {
				// Centering
				return (static_cast<int16_t>(v) - std::numeric_limits<int8_t>::max()) << 8;
			});
			smp.resize((smp.size() + 1) / 2);
			smp.shrink_to_fit();
			smp.back() = 0;	// Clear last data
			codec::ymb_encode(buf.data(), smp.data(), buf.size());
			samples.push_back(std::move(smp));
		}
	}

	return new PziBank(ids, deltaNs, isRepeatedList, samples);
}

AbstractInstrument* PziIO::loadInstrument(const std::vector<uint8_t>& sample,
										  int deltaN, bool isRepeated,
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
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, deltaN);
	instManLocked->setSampleADPCMRepeatEnabled(sampIdx, isRepeated);

	return adpcm;
}
}
