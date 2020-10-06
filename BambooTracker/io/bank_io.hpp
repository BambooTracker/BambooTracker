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

#pragma once

#include <string>
#include <memory>
#include <vector>
#include "bank.hpp"
#include "module.hpp"
#include "binary_container.hpp"
#include "format/wopn_file.h"

class BankIO
{
public:
	static void saveBank(BinaryContainer& ctr, const std::vector<int>& instNums,
						 const std::weak_ptr<InstrumentsManager> instMan);
	static AbstractBank* loadBank(const BinaryContainer& ctr, const std::string& path);
	static AbstractBank* loadBTBFile(const BinaryContainer& ctr);
	static AbstractBank* loadWOPNFile(const BinaryContainer& ctr);
	static AbstractBank* loadFFFile(const BinaryContainer& ctr);
	static AbstractBank* loadPPCFile(const BinaryContainer& ctr);
	static AbstractBank* loadPVIFile(const BinaryContainer& ctr);
	static AbstractBank* loadMucom88File(const BinaryContainer& ctr);

private:
	BankIO();

	static void extractADPCMSamples(const BinaryContainer& ctr, size_t addrPos, size_t sampOffs, int maxCnt,
									std::vector<int>& ids, std::vector<std::vector<uint8_t>>& samples);
};
