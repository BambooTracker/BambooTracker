/*
 * Copyright (C) 2020 Rerrah
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

#include "wopn_io.hpp"
#include "format/wopn_file.h"
#include "file_io_error.hpp"

WopnIO::WopnIO() : AbstractBankIO("wopn", "WOPN bank", true, false) {}

AbstractBank* WopnIO::load(const BinaryContainer& ctr) const
{
	struct WOPNDeleter {
		void operator()(WOPNFile *x) { WOPN_Free(x); }
	};

	std::unique_ptr<WOPNFile, WOPNDeleter> wopn;
	wopn.reset(WOPN_LoadBankFromMem(const_cast<char*>(ctr.getPointer()), ctr.size(), nullptr));
	if (!wopn)
		throw FileCorruptionError(FileIO::FileType::Bank, 0);

	WopnBank *bank = new WopnBank(wopn.get());
	wopn.release();
	return bank;
}
