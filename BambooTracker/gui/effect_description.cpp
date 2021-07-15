/*
 * Copyright (C) 2019-2021 Rerrah
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

#include "effect_description.hpp"
#include <unordered_map>
#include "enum_hash.hpp"

namespace effect_desc
{
namespace
{
struct EffectDetail
{
	const QString format;
	const char* desc;

	QString mergedString() const { return format + " - " + desc; }
};

const std::unordered_map<EffectType, EffectDetail> DETAILS = {
	{ EffectType::Arpeggio, { "00xy", QT_TRANSLATE_NOOP("EffectDescription", "Arpeggio, x: 2nd note (0-F), y: 3rd note (0-F)") } },
	{ EffectType::PortamentoUp, { "01xx", QT_TRANSLATE_NOOP("EffectDescription", "Portamento up, xx: depth (00-FF)") } },
	{ EffectType::PortamentoDown, { "02xx", QT_TRANSLATE_NOOP("EffectDescription", "Portamento down, xx: depth (00-FF)") } },
	{ EffectType::TonePortamento, { "03xx", QT_TRANSLATE_NOOP("EffectDescription", "Tone portamento, xx: depth (00-FF)") } },
	{ EffectType::Vibrato, { "04xy", QT_TRANSLATE_NOOP("EffectDescription", "Vibrato, x: period (0-F), y: depth (0-F)") } },
	{ EffectType::Tremolo, { "07xx", QT_TRANSLATE_NOOP("EffectDescription", "Tremolo, x: period (0-F), y: depth (0-F)") } },
	{ EffectType::Pan, { "08xx", QT_TRANSLATE_NOOP("EffectDescription", "Pan, xx: 00 = no sound, 01 = right, 02 = left, 03 = center") } },
	{ EffectType::VolumeSlide, { "0Axy", QT_TRANSLATE_NOOP("EffectDescription", "Volume slide, x: up (0-F), y: down (0-F)") } },
	{ EffectType::PositionJump, { "0Bxx", QT_TRANSLATE_NOOP("EffectDescription", "Jump to beginning of order xx") } },
	{ EffectType::SongEnd, { "0Cxx", QT_TRANSLATE_NOOP("EffectDescription", "End of song") } },
	{ EffectType::PatternBreak, { "0Dxx", QT_TRANSLATE_NOOP("EffectDescription", "Jump to step xx of next order") } },
	{ EffectType::SpeedTempoChange, { "0Fxx", QT_TRANSLATE_NOOP("EffectDescription", "Change speed (xx: 00-1F), change tempo (xx: 20-FF)") } },
	{ EffectType::NoteDelay, { "0Gxx", QT_TRANSLATE_NOOP("EffectDescription", "Note delay, xx: count (00-FF)") } },
	{ EffectType::AutoEnvelope, { "0Hxy", QT_TRANSLATE_NOOP("EffectDescription", "Auto envelope, x: shift amount (0-F), y: shape (0-F)") } },
	{ EffectType::HardEnvHighPeriod, { "0Ixx", QT_TRANSLATE_NOOP("EffectDescription", "Hardware envelope period 1, xx: high byte (00-FF)") } },
	{ EffectType::HardEnvLowPeriod, { "0Jxx", QT_TRANSLATE_NOOP("EffectDescription", "Hardware envelope period 2, xx: low byte (00-FF)") } },
	{ EffectType::Retrigger, { "0Kxy", QT_TRANSLATE_NOOP("EffectDescription", "Retrigger, x: volume slide (0-7: up, 8-F: down), y: tick (1-F)") } },
	{ EffectType::Groove, { "0Oxx", QT_TRANSLATE_NOOP("EffectDescription", "Set groove xx") } },
	{ EffectType::Detune, { "0Pxx", QT_TRANSLATE_NOOP("EffectDescription", "Detune, xx: pitch (00-FF)") } },
	{ EffectType::NoteSlideUp, { "0Qxy", QT_TRANSLATE_NOOP("EffectDescription", "Note slide up, x: count (0-F), y: seminote (0-F)") } },
	{ EffectType::NoteSlideDown, { "0Rxy", QT_TRANSLATE_NOOP("EffectDescription", "Note slide down, x: count (0-F), y: seminote (0-F)") } },
	{ EffectType::NoteCut, { "0Sxx", QT_TRANSLATE_NOOP("EffectDescription", "Note cut, xx: count (00-FF)") } },
	{ EffectType::TransposeDelay, { "0Txy", QT_TRANSLATE_NOOP("EffectDescription", "Transpose delay, x: count (0-7: up, 8-F: down), y: seminote (0-F)") } },
	{ EffectType::ToneNoiseMix, { "0Vxx", QT_TRANSLATE_NOOP("EffectDescription", "Tone/Noise mix, xx: 00 = no sound, 01 = tone, 02 = noise, 03 = tone & noise") } },
	{ EffectType::MasterVolume, { "0Vxx", QT_TRANSLATE_NOOP("EffectDescription", "Master volume, xx: volume (00-3F)") } },
	{ EffectType::NoisePitch, { "0Wxx", QT_TRANSLATE_NOOP("EffectDescription", "Noise pitch, xx: pitch (00-1F)") } },
	{ EffectType::RegisterAddress0, { "0Xxx", QT_TRANSLATE_NOOP("EffectDescription", "Register address bank 0, xx: address (00-6B)") } },
	{ EffectType::RegisterAddress1, { "0Yxx", QT_TRANSLATE_NOOP("EffectDescription", "Register address bank 1, xx: address (00-6B)") } },
	{ EffectType::RegisterValue, { "0Zxx", QT_TRANSLATE_NOOP("EffectDescription", "Register value set, xx: value (00-FF)") } },
	{ EffectType::ARControl, { "Axyy", QT_TRANSLATE_NOOP("EffectDescription", "AR control, x: operator (1-4), yy: attack rate (00-1F)") } },
	{ EffectType::Brightness, { "B0xx", QT_TRANSLATE_NOOP("EffectDescription", "Brightness, xx: relative value (01-FF)") } },
	{ EffectType::DRControl, { "Dxyy", QT_TRANSLATE_NOOP("EffectDescription", "DR control, x: operator (1-4), yy: decay rate (00-1F)") } },
	{ EffectType::EnvelopeReset, { "ESxx", QT_TRANSLATE_NOOP("EffectDescription", "Envelope reset, xx: count (00-FF)") } },
	{ EffectType::FBControl, { "FBxx", QT_TRANSLATE_NOOP("EffectDescription", "FB control, xx: feedback value (00-07)") } },
	{ EffectType::FineDetune, { "FPxx", QT_TRANSLATE_NOOP("EffectDescription", "Fine detune, xx: pitch (00-FF)") } },
	{ EffectType::MLControl, { "MLxy", QT_TRANSLATE_NOOP("EffectDescription", "ML control, x: operator (1-4), y: multiple (0-F)") } },
	{ EffectType::VolumeDelay, { "Mxyy", QT_TRANSLATE_NOOP("EffectDescription", "Volume delay, x: count (0-F), yy: volume (00-FF)") } },
	{ EffectType::RRControl, { "RRxy", QT_TRANSLATE_NOOP("EffectDescription", "RR control, x: operator (1-4), y: release rate (0-F)") } },
	{ EffectType::TLControl, { "Txyy", QT_TRANSLATE_NOOP("EffectDescription", "TL control, x: operator (1-4), yy: total level (00-7F)") } },
	{ EffectType::NoEffect, { "", QT_TRANSLATE_NOOP("EffectDescription", "Invalid effect") } }
};
}

QString getEffectFormat(const EffectType type)
{
	return DETAILS.at(type).format;
}
QString getEffectDescription(const EffectType type)
{
	return DETAILS.at(type).desc;
}

QString getEffectFormatAndDetailString(const EffectType type)
{
	if (type == EffectType::NoEffect) return DETAILS.at(EffectType::NoEffect).desc;
	else return DETAILS.at(type).mergedString();
}
}
