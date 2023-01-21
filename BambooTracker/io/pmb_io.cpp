/*
 * Copyright (C) 2022-2023 Rerrah
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

#include "pmb_io.hpp"
#include <vector>
#include <limits>
#include "instrument.hpp"
#include "file_io_error.hpp"
#include "chip/codec/ymb_codec.hpp"

namespace io
{
PmbIO::PmbIO() : AbstractBankIO("pmb", "FM Towns PMB", true, false) {}

AbstractBank* PmbIO::load(const BinaryContainer& ctr) const
{
	/* no identifier & weird header inconsistencies across samples, just don't bother with the header */
	constexpr size_t HEADER_SIZE = 0x1008;
	if (ctr.size() < HEADER_SIZE) throw FileCorruptionError(FileType::Bank, 0);

	std::vector<int> ids;
	std::vector<std::string> names;
	std::vector<std::vector<uint8_t>> samples;
	size_t globCsr = HEADER_SIZE;
	constexpr int MAX_CNT = 32;
	for (int i = 0; i < MAX_CNT; ++i) {
		// cannot determine how many samples there will be, just stop when we've reached EOF
		if (globCsr == ctr.size()) break;
		std::string name = ctr.readString(globCsr, 8);
		globCsr += 8;
		// ignore sample ID, they may override each other or disagree with the ones in the header
		// uint32_t id = ctr.readUint32(globCsr);
		globCsr += 4;
		uint32_t len = ctr.readUint32(globCsr);
		globCsr += 4;

		// unknown data
		globCsr += 16;

		ids.push_back(i);
		names.push_back(name);

		std::vector<uint8_t>&& smp = ctr.getSubcontainer(globCsr, len).toVector();

		std::vector<int16_t> buf(smp.size());
		std::transform(smp.begin(), smp.end(), buf.begin(), [globCsr](uint8_t v) {
			// first convert from RF5C68 encoding to more regular unsigned 8-bit
			// summarised from the datasheet:
			//        0x00 -> 0x80 (not mentioned in datasheet but seems to work like this in banks)
			// 0x01 - 0x7F -> 0x7F - 0x01
			// 0x80 - 0xFE -> 0x80 - 0xFE (unchanged)
			//        0xFF -> trigger for jumping into loop. unsure if occurs in PMB banks, loops not handled by us anyway. error if encountered
			if (v == 0xFF) throw FileCorruptionError (FileType::Bank, globCsr);
			uint8_t regular = (v < 0x80) ? (0x80 - v) : v;

			// now convert this into the format ADPCM conversion requires
			uint16_t grown = static_cast<uint16_t>(regular + 0x80) << 8;
			return *(reinterpret_cast<int16_t *> (&grown));
		});
		smp.resize((smp.size() + 1) / 2);
		smp.shrink_to_fit();
		smp.back() = 0;	// Clear last data
		codec::ymb_encode(buf.data(), smp.data(), buf.size());
		samples.push_back(std::move(smp));
		globCsr += len;
	}

	return new PmbBank(ids, names, samples);
}

AbstractInstrument* PmbIO::loadInstrument(const std::vector<uint8_t>& sample,
											std::string name,
										  std::weak_ptr<InstrumentsManager> instMan,
										  int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int sampIdx = instManLocked->findFirstAssignableSampleADPCM();
	if (sampIdx < 0) throw FileCorruptionError(FileType::Bank, 0);

	InstrumentADPCM* adpcm = new InstrumentADPCM(instNum, name, instManLocked.get());
	adpcm->setSampleNumber(sampIdx);

	instManLocked->storeSampleADPCMRawSample(sampIdx, sample);
	instManLocked->setSampleADPCMRootKeyNumber(sampIdx, 67);	// o5g
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, 0x49cd); // 16000Hz
	instManLocked->setSampleADPCMRepeatEnabled(sampIdx, false);
	instManLocked->setSampleADPCMRepeatrange(sampIdx, SampleRepeatRange(0, (sample.size() - 1) >> 5));

	return adpcm;
}
}
