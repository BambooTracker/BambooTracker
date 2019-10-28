#include "effect_description.hpp"

EffectDescription::EffectDescription() {}

const std::map<EffectType, EffectDescription::EffectDetail> EffectDescription::details_ = {
	{ EffectType::Arpeggio, { "00xy", tr("Arpeggio, x: 2nd note (0-F), y: 3rd note (0-F)") } },
	{ EffectType::PortamentoUp, { "01xx", tr("Portamento up, xx: depth (00-FF)") } },
	{ EffectType::PortamentoDown, { "02xx", tr("Portamento down, xx: depth (00-FF)") } },
	{ EffectType::TonePortamento, { "03xx", tr("Tone portamento, xx: depth (00-FF)") } },
	{ EffectType::Vibrato, { "04xy", tr("Vibrato, x: period (0-F), y: depth (0-F)") } },
	{ EffectType::Tremolo, { "07xx", tr("Tremolo, x: period (0-F), y: depth (0-F)") } },
	{ EffectType::Pan, { "08xx", tr("Pan, xx: 00 = no sound, 01 = right, 02 = left, 03 = center") } },
	{ EffectType::VolumeSlide, { "0Axy", tr("Volume slide, x: up (0-F), y: down (0-F)") } },
	{ EffectType::PositionJump, { "0Bxx", tr("Jump to beginning of order xx") } },
	{ EffectType::SongEnd, { "0Cxx", tr("End of song") } },
	{ EffectType::PatternBreak, { "0Dxx", tr("Jump to step xx of next order") } },
	{ EffectType::SpeedTempoChange, { "0Fxx", tr("Change speed (xx: 00-1F), change tempo (xx: 20-FF)") } },
	{ EffectType::NoteDelay, { "0Gxx", tr("Note delay, xx: count (00-FF)") } },
	{ EffectType::AutoEnvelope, { "0Hxy", tr("Auto envelope, x: shift amount (0-F), y: shape (0-F)") } },
	{ EffectType::HardEnvHighPeriod, { "0Ixx", tr("Hardware envelope period 1, xx: high byte (00-FF)") } },
	{ EffectType::HardEnvLowPeriod, { "0Jxx", tr("Hardware envelope period 2, xx: low byte (00-FF)") } },
	{ EffectType::Groove, { "0Oxx", tr("Set groove xx") } },
	{ EffectType::Detune, { "0Pxx", tr("Detune, xx: pitch (00-FF)") } },
	{ EffectType::NoteSlideUp, { "0Qxy", tr("Note slide up, x: count (0-F), y: seminote (0-F)") } },
	{ EffectType::NoteSlideDown, { "0Rxy", tr("Note slide down, x: count (0-F), y: seminote (0-F)") } },
	{ EffectType::NoteCut, { "0Sxx", tr("Note cut, xx: count (01-FF)") } },
	{ EffectType::TransposeDelay, { "0Txy", tr("Transpose delay, x: count (1-7: up, 9-F: down), y: seminote (0-F)") } },
	{ EffectType::ToneNoiseMix, { "0Vxx", tr("Tone/Noise mix, xx: 00 = no sound, 01 = tone, 02 = noise, 03 = tone & noise") } },
	{ EffectType::MasterVolume, { "0Vxx", tr("Master volume, xx: volume (00-3F)") } },
	{ EffectType::NoisePitch, { "0Wxx", tr("Noise pitch, xx: pitch (00-1F)") } },
	{ EffectType::RegisterAddress0, { "0Xxx", tr("Register address bank 0, xx: address (00-6B)") } },
	{ EffectType::RegisterAddress1, { "0Yxx", tr("Register address bank 1, xx: address (00-6B)") } },
	{ EffectType::RegisterValue, { "0Zxx", tr("Register value set, xx: value (00-FF)") } },
	{ EffectType::ARControl, { "Axyy", tr("AR control, x: operator (1-4), yy: attack rate (00-1F)") } },
	{ EffectType::DRControl, { "Dxyy", tr("DR control, x: operator (1-4), yy: decay rate (00-1F)") } },
	{ EffectType::FBControl, { "FBxx", tr("FB control, xx: feedback value (00-07)") } },
	{ EffectType::MLControl, { "MLxy", tr("ML control, x: operator (1-4), y: multiple (0-F)") } },
	{ EffectType::VolumeDelay, { "Mxyy", tr("Volume delay, x: count (1-F), yy: volume (00-FF)") } },
	{ EffectType::RRControl, { "RRxy", tr("RR control, x: operator (1-4), y: release rate (0-F)") } },
	{ EffectType::TLControl, { "Txyy", tr("TL control, x: operator (1-4), yy: total level (00-7F)") } },
	{ EffectType::NoEffect, { "", tr("Invalid effect") } }
};

QString EffectDescription::getEffectFormat(const EffectType type)
{
	return details_.at(type).format;
}
QString EffectDescription::getEffectDescription(const EffectType type)
{
	return details_.at(type).desc;
}

QString EffectDescription::getEffectFormatAndDetailString(const EffectType type)
{
	if (type == EffectType::NoEffect) return details_.at(EffectType::NoEffect).desc;
	else return details_.at(type).mergedString();
}
