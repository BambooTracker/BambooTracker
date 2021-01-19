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

#include "lfo_fm.hpp"

namespace
{
const std::unordered_map<FMLFOParameter, int> DEF_PARAMS = {
	{ FMLFOParameter::FREQ, 0 },
	{ FMLFOParameter::PMS, 0 },
	{ FMLFOParameter::AMS, 0 },
	{ FMLFOParameter::AM1, 0 },
	{ FMLFOParameter::AM2, 0 },
	{ FMLFOParameter::AM3, 0 },
	{ FMLFOParameter::AM4, 0 },
	{ FMLFOParameter::Count, 0 }
};
}

LFOFM::LFOFM(int n)
	: AbstractInstrumentProperty (n)
{
	clearParameters();
}

std::unique_ptr<LFOFM> LFOFM::clone()
{
	std::unique_ptr<LFOFM> clone = std::make_unique<LFOFM>(*this);
	clone->clearUserInstruments();
	return clone;
}

void LFOFM::setParameterValue(FMLFOParameter param, int value)
{
	params_.at(param) = value;
}

int LFOFM::getParameterValue(FMLFOParameter param) const
{
	return params_.at(param);
}

bool LFOFM::isEdited() const
{
	return params_ != DEF_PARAMS;
}

void LFOFM::clearParameters()
{
	params_ = DEF_PARAMS;
}
