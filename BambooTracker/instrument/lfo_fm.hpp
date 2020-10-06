/*
 * Copyright (C) 2018-2020 Rerrah
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

#include <memory>
#include "abstract_instrument_property.hpp"

enum class FMLFOParameter;


class LFOFM : public AbstractInstrumentProperty
{
public:
	explicit LFOFM(int n);
	LFOFM(const LFOFM& other);

	friend bool operator==(const LFOFM& a, const LFOFM& b);
	friend bool operator!=(const LFOFM& a, const LFOFM& b) { return !(a == b); }

	std::unique_ptr<LFOFM> clone();

	void setParameterValue(FMLFOParameter param, int value);
	int getParameterValue(FMLFOParameter param) const;

	bool isEdited() const override;
	void clearParameters() override;

private:
	int freq_;
	int pms_;
	int ams_;
	int amOp_[4];
	int cnt_;

	static constexpr int DEF_FREQ_ = 0;
	static constexpr int DEF_PMS_ = 0;
	static constexpr int DEF_AMS_ = 0;
	static constexpr int DEF_AM_OP_[4] = { 0, 0, 0, 0 };
	static constexpr int DEF_CNT_ = 0;
};

enum class FMLFOParameter
{
	FREQ, AMS, PMS, Count,
	AM1, AM2, AM3, AM4
};
