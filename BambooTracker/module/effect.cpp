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

#include "effect.hpp"
#include <unordered_map>
#include "step.hpp"

namespace
{
const std::unordered_map<char, int> CTOHEX = {
	{ '0', 0 },
	{ '1', 1 },
	{ '2', 2 },
	{ '3', 3 },
	{ '4', 4 },
	{ '5', 5 },
	{ '6', 6 },
	{ '7', 7 },
	{ '8', 8 },
	{ '9', 9 },
	{ 'A', 10 },
	{ 'B', 11 },
	{ 'C', 12 },
	{ 'D', 13 },
	{ 'E', 14 },
	{ 'F', 15 }
};
}

namespace effect_utils
{
EffectType validateEffectId(SoundSource src, const std::string& id)
{
	if (id == "00") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::Arpeggio;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "01") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::PortamentoUp;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "02") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::PortamentoDown;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "03") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::TonePortamento;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "04") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::Vibrato;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "07") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::Tremolo;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "08") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::RHYTHM:
		case SoundSource::ADPCM:
			return EffectType::Pan;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0A") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::VolumeSlide;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0B") {
		return EffectType::PositionJump;
	}
	else if (id == "0C") {
		return EffectType::SongEnd;
	}
	else if (id == "0D") {
		return EffectType::PatternBreak;
	}
	else if (id == "0F") {
		return EffectType::SpeedTempoChange;
	}
	else if (id == "0G") {
		return EffectType::NoteDelay;
	}
	else if (id == "0H") {
		switch (src) {
		case SoundSource::SSG:
			return EffectType::AutoEnvelope;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0I") {
		switch (src) {
		case SoundSource::SSG:
			return EffectType::HardEnvHighPeriod;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0J") {
		switch (src) {
		case SoundSource::SSG:
			return EffectType::HardEnvLowPeriod;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0K") {
		return EffectType::Retrigger;
	}
	else if (id == "0O") {
		return EffectType::Groove;
	}
	else if (id == "0P") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::Detune;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0Q") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::NoteSlideUp;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0R") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::NoteSlideDown;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0S") {
		return EffectType::NoteRelease;
	}
	else if (id == "0T") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::TransposeDelay;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0V") {
		switch (src) {
		case SoundSource::SSG:
			return EffectType::ToneNoiseMix;
		case SoundSource::RHYTHM:
			return EffectType::MasterVolume;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0W") {
		switch (src) {
		case SoundSource::SSG:
			return EffectType::NoisePitch;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0X") {
		return EffectType::RegisterAddress0;
	}
	else if (id == "0Y") {
		return EffectType::RegisterAddress1;
	}
	else if (id == "0Z") {
		return EffectType::RegisterValue;
	}
	else if (id == "B0") {
		switch (src) {
		case SoundSource::FM:
			return EffectType::Brightness;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "EA") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::XVolumeSlide;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "ES") {
		return EffectType::NoteCut;
	}
	else if (id == "FB") {
		switch (src) {
		case SoundSource::FM:
			return EffectType::FBControl;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "FP") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
		case SoundSource::ADPCM:
			return EffectType::FineDetune;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "ML") {
		switch (src) {
		case SoundSource::FM:
			return EffectType::MLControl;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "RR") {
		switch (src) {
		case SoundSource::FM:
			return EffectType::RRControl;
		default:
			return EffectType::NoEffect;
		}
	}
	else {
		switch (id.front()) {
		case 'A':
			switch (src) {
			case SoundSource::FM:
				return EffectType::ARControl;
			default:
				return EffectType::NoEffect;
			}
		case 'D':
			switch (src) {
			case SoundSource::FM:
				return EffectType::DRControl;
			default:
				return EffectType::NoEffect;
			}
		case 'M':
			return EffectType::VolumeDelay;
		case 'T':
			switch (src) {
			case SoundSource::FM:
				return EffectType::TLControl;
			default:
				return EffectType::NoEffect;
			}
		default:
			return EffectType::NoEffect;
		}
	}
}

Effect validateEffect(SoundSource src, const std::string& id, int value)
{
	if (value == Step::EFF_VAL_NONE) return { EffectType::NoEffect, Step::EFF_VAL_NONE };

	EffectType type = effect_utils::validateEffectId(src, id);

	int v;
	switch (type) {
	case EffectType::NoEffect:
		v = Step::EFF_VAL_NONE;
		break;
	case EffectType::VolumeDelay:
	case EffectType::TLControl:
	case EffectType::ARControl:
	case EffectType::DRControl:
		v = (CTOHEX.at(id[1]) << 8) | value;
		break;
	default:
		v = value;
	}

	return { type, v };
}
}
