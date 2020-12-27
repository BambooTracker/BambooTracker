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

#include "instrument_io.hpp"

namespace io
{
class BtiIO final : public AbstractInstrumentIO
{
public:
	BtiIO();
	AbstractInstrument* load(const BinaryContainer& ctr, const std::string& fileName,
							 std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;
	void save(BinaryContainer& ctr,
			  const std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	static size_t loadInstrumentPropertyOperatorSequenceForInstrument(
			FMEnvelopeParameter param, size_t instMemCsr,
			std::shared_ptr<InstrumentsManager>& instManLocked,
			const BinaryContainer& ctr, InstrumentFM* inst, int idx, uint32_t version);
};
}
