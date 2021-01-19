/*
 * Copyright (C) 2018-2021 Rerrah
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

#include <unordered_map>
#include <memory>
#include "abstract_instrument_property.hpp"
#include "enum_hash.hpp"

enum class FMLFOParameter
{
	FREQ, AMS, PMS, Count,
	AM1, AM2, AM3, AM4
};

class LFOFM final : public AbstractInstrumentProperty
{
public:
	explicit LFOFM(int n);

	friend bool operator==(const LFOFM& a, const LFOFM& b) { return a.params_ == b.params_; }
	friend bool operator!=(const LFOFM& a, const LFOFM& b) { return !(a == b); }

	std::unique_ptr<LFOFM> clone();

	void setParameterValue(FMLFOParameter param, int value);
	int getParameterValue(FMLFOParameter param) const;

	bool isEdited() const override;
	void clearParameters() override;

private:
	std::unordered_map<FMLFOParameter, int> params_;
};
