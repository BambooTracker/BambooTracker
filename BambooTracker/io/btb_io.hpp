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

#pragma once

#include "bank_io.hpp"

namespace io
{
class BtbIO final : public AbstractBankIO
{
public:
	BtbIO();
	AbstractBank* load(const BinaryContainer& ctr) const override;
	void save(BinaryContainer& ctr, const std::weak_ptr<InstrumentsManager> instMan,
			  const std::vector<int>& instNums) const override;

	static AbstractInstrument* loadInstrument(const BinaryContainer& instCtr,
											  const BinaryContainer& propCtr,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum,
											  uint32_t bankVersion);

private:
	static size_t getPropertyPosition(const BinaryContainer& propCtr,
									  uint8_t subsecType, uint8_t index);
};
}
