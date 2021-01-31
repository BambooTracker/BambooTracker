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

#include "pps_io.hpp"
#include <vector>
#include "instrument.hpp"
#include "file_io_error.hpp"
#include "chip/codec/ymb_codec.hpp"

namespace io
{
PpsIO::PpsIO() : AbstractBankIO("pps", "PMD PPS", true, false) {}

AbstractBank* PpsIO::load(const BinaryContainer& ctr) const
{
	constexpr size_t SAMP_OFFS = 0x54;
	if (ctr.size() < SAMP_OFFS) throw FileCorruptionError(FileType::Bank, 0);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	size_t globCsr = 0;
	constexpr int MAX_CNT = 14;
	for (int i = 0; i < MAX_CNT; ++i) {
		uint16_t start = ctr.readUint16(globCsr);
		globCsr += 2;
		uint16_t len = ctr.readUint16(globCsr);
		globCsr += 4;	// Skip 2 bytes

		if (len) {
			ids.push_back(i);

			std::vector<uint8_t>&& smp = ctr.getSubcontainer(start, len).toVector();
			std::vector<int16_t> buf(smp.size() * 2);
			for (size_t i = 0; i < smp.size(); ++i) {
				uint8_t sample = smp[i];
				buf[i] = (static_cast<int16_t>(sample >> 4) - 8) << 12;
				buf[i + 1] = (static_cast<int16_t>(sample & 0x0f) - 8) << 12;
			}
			codec::ymb_encode(buf.data(), smp.data(), buf.size());
			samples.push_back(std::move(smp));
		}
	}

	return new PpsBank(ids, samples);
}

AbstractInstrument* PpsIO::loadInstrument(const std::vector<uint8_t>& sample,
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

	return adpcm;
}
}
