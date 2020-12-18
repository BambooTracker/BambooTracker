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

#include "step.hpp"

const std::string Step::EFF_ID_NONE = "--";

Step::Step()
	: note_(NOTE_NONE),
	  inst_(INST_NONE),
	  vol_(VOLUME_NONE)
{
	for (size_t i = 0; i < N_EFFECT; ++i) {
		eff_[i].id = EFF_ID_NONE;
		eff_[i].value = EFF_VAL_NONE;
	}
}

void Step::clear()
{
	clearNoteNumber();
	clearInstrumentNumber();
	clearVolume();
	for (size_t i = 0; i < N_EFFECT; ++i) {
		clearEffect(i);
	}
}

bool Step::hasEvent() const
{
	if (!isEmptyNote()) return true;
	if (hasInstrument()) return true;
	if (hasVolume()) return true;
	for (int i = 0; i < N_EFFECT; ++i) {
		if (hasEffectId(i)) return true;
		if (hasEffectValue(i)) return true;
	}
	return false;
}
