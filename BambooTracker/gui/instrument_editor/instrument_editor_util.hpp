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

#ifndef INSTRUMENT_EDITOR_UTIL_HPP
#define INSTRUMENT_EDITOR_UTIL_HPP

#include <unordered_map>
#include <algorithm>
#include "enum_hash.hpp"
#include "command_sequence.hpp"
#include "gui/instrument_editor/visualized_instrument_macro_editor.hpp"

const std::unordered_map<VisualizedInstrumentMacroEditor::SequenceType, SequenceType> SEQ_TYPE_MAP = {
	{ VisualizedInstrumentMacroEditor::SequenceType::NoType, SequenceType::NO_SEQUENCE_TYPE },
	{ VisualizedInstrumentMacroEditor::SequenceType::FixedSequence, SequenceType::FIXED_SEQUENCE },
	{ VisualizedInstrumentMacroEditor::SequenceType::AbsoluteSequence, SequenceType::ABSOLUTE_SEQUENCE },
	{ VisualizedInstrumentMacroEditor::SequenceType::RelativeSequence, SequenceType::RELATIVE_SEQUENCE }
};

inline SequenceType convertSequenceTypeForData(VisualizedInstrumentMacroEditor::SequenceType type)
{
	return SEQ_TYPE_MAP.at(type);
}

inline VisualizedInstrumentMacroEditor::SequenceType convertSequenceTypeForUI(SequenceType type)
{
	return std::find_if(SEQ_TYPE_MAP.begin(), SEQ_TYPE_MAP.end(), [type](const auto& pair) {
		return (pair.second == type);
	})->first;
}

const std::unordered_map<VisualizedInstrumentMacroEditor::ReleaseType, ReleaseType> REL_TYPE_MAP = {
	{ VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE, ReleaseType::NoRelease },
	{ VisualizedInstrumentMacroEditor::ReleaseType::FIXED_RELEASE, ReleaseType::FixedRelease },
	{ VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE_RELEASE, ReleaseType::AbsoluteRelease },
	{ VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE_RELEASE, ReleaseType::RelativeRelease }
};

inline ReleaseType convertReleaseTypeForData(VisualizedInstrumentMacroEditor::ReleaseType type)
{
	return REL_TYPE_MAP.at(type);
}

inline VisualizedInstrumentMacroEditor::ReleaseType convertReleaseTypeForUI(ReleaseType type)
{
	return std::find_if(REL_TYPE_MAP.begin(), REL_TYPE_MAP.end(), [type](const auto& pair) {
		return (pair.second == type);
	})->first;
}

#endif // INSTRUMENT_EDITOR_UTIL_HPP
