/*
 * Copyright (C) 2019-2022 Rerrah
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

#include <string>
#include "step.hpp"
#include "bamboo_tracker_defs.hpp"

enum class EffectType
{
	NoEffect,
	Arpeggio, PortamentoUp, PortamentoDown, TonePortamento, Vibrato, Tremolo, Pan, VolumeSlide, PositionJump,
	SongEnd, PatternBreak, SpeedTempoChange, NoteDelay, Groove, Detune, NoteSlideUp, NoteSlideDown,
	NoteRelease, TransposeDelay, MasterVolume, VolumeDelay, ToneNoiseMix, NoisePitch, HardEnvHighPeriod,
	HardEnvLowPeriod, AutoEnvelope, FBControl, TLControl, MLControl, ARControl, DRControl, RRControl,
	RegisterAddress0, RegisterAddress1, RegisterValue, Brightness, FineDetune, NoteCut, Retrigger, XVolumeSlide
};

struct Effect
{
	EffectType type;
	int value;
};

namespace effect_utils
{
EffectType validateEffectId(SoundSource src, const std::string& id);
Effect validateEffect(SoundSource src, const std::string& id, int value);
inline Effect validateEffect(SoundSource src, const Step::PlainEffect& plain)
{
	return validateEffect(src, plain.id, plain.value);
}

inline int reverseFmVolume(int volume, bool over0 = false) noexcept
{
	return (volume < bt_defs::NSTEP_FM_VOLUME) ? (bt_defs::NSTEP_FM_VOLUME - 1 - volume)
												 : over0 ? 0 : volume;
}

inline int reverseFmBrightness(int value) noexcept
{
	return (value > 0) ? (0xff - value + 1) : value;
}
}
