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

#pragma once

#include "sequence_property.hpp"

using FMOperatorSequenceUnit = InstrumentSequenceBaseUnit;
using FMOperatorSequenceIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>;

enum class FMOperatorType
{
	All, Op1, Op2, Op3, Op4
};

using ArpeggioUnit = InstrumentSequenceBaseUnit;
using ArpeggioIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>;

using PitchUnit = InstrumentSequenceBaseUnit;
using PitchIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>;

constexpr int SEQ_PITCH_CENTER = 127;

using PanUnit = InstrumentSequenceBaseUnit;
using PanIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>;

namespace PanType
{
enum : uint8_t
{
	SILENCE = 0,
	RIGHT = 1,
	LEFT = 2,
	CENTER = RIGHT | LEFT
};
}

using SSGWaveformUnit = InstrumentSequenceExtendUnit;
using SSGWaveformIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceExtendUnit>::Iterator>;

class SSGWaveformType
{
public:
	enum : int
	{
		UNSET = SSGWaveformUnit::ERR_DATA,
		SQUARE = 0,
		TRIANGLE = 1,
		SAW = 2,
		INVSAW = 3,
		SQM_TRIANGLE = 4,
		SQM_SAW = 5,
		SQM_INVSAW = 6
	};

	static bool testHardEnvelopeOccupancity(int form)
	{
		switch (form) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
			return false;
		default:
			return true;
		}
	}

	SSGWaveformType() = delete;
};

using SSGToneNoiseUnit = InstrumentSequenceBaseUnit;
using SSGToneNoiseIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>;

using SSGEnvelopeUnit = InstrumentSequenceExtendUnit;
using SSGEnvelopeIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceExtendUnit>::Iterator>;

using ADPCMEnvelopeUnit = InstrumentSequenceBaseUnit;
using ADPCMEnvelopeIter = std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>;
