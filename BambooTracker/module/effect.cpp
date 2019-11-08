#include "effect.hpp"

EffectType Effect::toEffectType(SoundSource src, std::string id)
{
	if (id == "00") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::Arpeggio;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "01") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::PortamentoUp;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "02") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::PortamentoDown;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "03") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::TonePortamento;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "04") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::Vibrato;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "07") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::Tremolo;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "08") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::DRUM:
			return EffectType::Pan;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0A") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
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
	else if (id == "0O") {
		return EffectType::Groove;
	}
	else if (id == "0P") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::Detune;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0Q") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::NoteSlideUp;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0R") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::NoteSlideDown;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0S") {
		return EffectType::NoteCut;
	}
	else if (id == "0T") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			return EffectType::TransposeDelay;
		default:
			return EffectType::NoEffect;
		}
	}
	else if (id == "0V") {
		switch (src) {
		case SoundSource::SSG:
			return EffectType::ToneNoiseMix;
		case SoundSource::DRUM:
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
	else if (id == "FB") {
		switch (src) {
		case SoundSource::FM:
			return EffectType::FBControl;
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

Effect Effect::makeEffectData(SoundSource src, std::string id, int value)
{
	if (value == -1) return { EffectType::NoEffect, -1 };

	EffectType type = Effect::toEffectType(src, id);

	int v;
	switch (type) {
	case EffectType::NoEffect:
		v = -1;
		break;
	case EffectType::VolumeDelay:
	case EffectType::TLControl:
	case EffectType::ARControl:
	case EffectType::DRControl:
		v = (ctohex(id[1]) << 8) | value;
		break;
	default:
		v = value;
	}

	return { type, v };
}
