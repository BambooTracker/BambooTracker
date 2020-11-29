/*
 * Copyright (C) 2019-2020 Rerrah
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

#include "bank_io.hpp"
#include <algorithm>
#include "file_io_error.hpp"
#include "btb_io.hpp"
#include "wopn_io.hpp"
#include "ff_io.hpp"
#include "ppc_io.hpp"
#include "pvi_io.hpp"
#include "dat_io.hpp"

AbstractBank* AbstractBankIO::load(const BinaryContainer& ctr) const
{
	(void)ctr;
	throw FileUnsupportedError(FileIO::FileType::Bank);
}

void AbstractBankIO::save(BinaryContainer& ctr, const std::weak_ptr<InstrumentsManager> instMan,
				  const std::vector<int>& instNums) const
{
	(void)ctr;
	(void)instMan;
	(void)instNums;
	throw FileUnsupportedError(FileIO::FileType::Bank);
}

//------------------------------------------------------------

std::unique_ptr<BankIO> BankIO::instance_;

BankIO::BankIO()
{
	handler_.add(new BtbIO);
	handler_.add(new WopnIO);
	handler_.add(new FfIO);
	handler_.add(new PpcIO);
	handler_.add(new PviIO);
	handler_.add(new DatIO);
}

BankIO& BankIO::getInstance()
{
	if (!instance_) instance_.reset(new BankIO);
	return *instance_;
}

void BankIO::saveBank(BinaryContainer& ctr, const std::weak_ptr<InstrumentsManager> instMan,
					  const std::vector<int>& instNums)
{
	handler_.at("btb")->save(ctr, instMan, instNums);
}

AbstractBank* BankIO::loadBank(const BinaryContainer& ctr, const std::string& path)
{
	return handler_.at(FileIO::getExtension(path))->load(ctr);
}

void BankIO::extractADPCMSamples(const BinaryContainer& ctr, size_t addrPos, size_t sampOffs,
								 int maxCnt, std::vector<int>& ids,
								 std::vector<std::vector<uint8_t>>& samples)
{
	size_t ofs = 0;
	for (int i = 0; i < maxCnt; ++i) {
		uint16_t start = ctr.readUint16(addrPos);
		addrPos += 2;
		uint16_t stop = ctr.readUint16(addrPos);
		addrPos += 2;

		if (stop && start <= stop) {
			if (ids.empty()) ofs = start;
			ids.push_back(i);
			size_t st = sampOffs + static_cast<size_t>((start - ofs) << 5);
			size_t sampSize = std::min(static_cast<size_t>((stop + 1 - start) << 5), ctr.size() - st);
			samples.push_back(ctr.getSubcontainer(st, sampSize).toVector());
		}
	}
}
