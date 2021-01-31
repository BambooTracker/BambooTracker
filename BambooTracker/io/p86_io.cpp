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

#include "p86_io.hpp"
#include <vector>
#include "instrument.hpp"
#include "file_io_error.hpp"
#include "chip/codec/ymb_codec.hpp"

namespace io
{
namespace
{
inline uint32_t readUint24(const BinaryContainer& ctr, size_t offset)
{
	return ctr.readUint8(offset) | (ctr.readUint8(offset + 1) << 8u) | (ctr.readUint8(offset + 2) << 16u);
}
}

P86IO::P86IO() : AbstractBankIO("p86", "PMD P86", true, false) {}

AbstractBank* P86IO::load(const BinaryContainer& ctr) const
{
	using namespace std::literals::string_literals;

	std::string id = ctr.readString(0, 12);
	if (id != "PCM86 DATA\n\0"s)
		throw FileCorruptionError(FileType::Bank, 0);
	uint32_t size = readUint24(ctr, 13);
	if (size != ctr.size())
		throw FileCorruptionError(FileType::Bank, 13);

	constexpr size_t SAMP_OFFS = 0x610;
	if (ctr.size() < SAMP_OFFS) throw FileCorruptionError(FileType::Bank, 0x10);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	size_t globCsr = 0x10;
	size_t ofs = 0;
	constexpr int MAX_CNT = 256;
	for (int i = 0; i < MAX_CNT; ++i) {
		uint32_t start = readUint24(ctr, globCsr);
		globCsr += 3;
		uint32_t len = readUint24(ctr, globCsr);
		globCsr += 3;

		if (len) {
			if (ids.empty()) ofs = start;
			ids.push_back(i);

			std::vector<uint8_t>&& smp = ctr.getSubcontainer(SAMP_OFFS + start - ofs, len).toVector();
			std::vector<int16_t> buf(smp.size());
			std::transform(smp.begin(), smp.end(), buf.begin(),
						   [](uint8_t v) { return static_cast<int16_t>(v) << 8; });
			smp.resize((smp.size() + 1) / 2);
			smp.shrink_to_fit();
			smp.back() = 0;	// Clear last data
			codec::ymb_encode(buf.data(), smp.data(), buf.size());
			samples.push_back(std::move(smp));
		}
	}

	return new P86Bank(ids, samples);
}

AbstractInstrument* P86IO::loadInstrument(const std::vector<uint8_t>& sample,
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
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, 0x4a0d);	// 16540Hz

	return adpcm;
}
}
