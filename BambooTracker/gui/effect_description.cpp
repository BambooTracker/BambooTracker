#include "effect_description.hpp"

EffectDescription::EffectDescription() {}

const std::unordered_map<EffectType, EffectDescription::EffectDetail> EffectDescription::details_ = {
	{ EffectType::Arpeggio, { "00xy", QT_TR_NOOP("Arpeggio, x: 2nd note (0-F), y: 3rd note (0-F)") } },
	{ EffectType::PortamentoUp, { "01xx", QT_TR_NOOP("Portamento up, xx: depth (00-FF)") } },
	{ EffectType::PortamentoDown, { "02xx", QT_TR_NOOP("Portamento down, xx: depth (00-FF)") } },
	{ EffectType::TonePortamento, { "03xx", QT_TR_NOOP("Tone portamento, xx: depth (00-FF)") } },
	{ EffectType::Vibrato, { "04xy", QT_TR_NOOP("Vibrato, x: period (0-F), y: depth (0-F)") } },
	{ EffectType::Tremolo, { "07xx", QT_TR_NOOP("Tremolo, x: period (0-F), y: depth (0-F)") } },
	{ EffectType::Pan, { "08xx", QT_TR_NOOP("Pan, xx: 00 = no sound, 01 = right, 02 = left, 03 = center") } },
	{ EffectType::VolumeSlide, { "0Axy", QT_TR_NOOP("Volume slide, x: up (0-F), y: down (0-F)") } },
	{ EffectType::PositionJump, { "0Bxx", QT_TR_NOOP("Jump to beginning of order xx") } },
	{ EffectType::SongEnd, { "0Cxx", QT_TR_NOOP("End of song") } },
	{ EffectType::PatternBreak, { "0Dxx", QT_TR_NOOP("Jump to step xx of next order") } },
	{ EffectType::SpeedTempoChange, { "0Fxx", QT_TR_NOOP("Change speed (xx: 00-1F), change tempo (xx: 20-FF)") } },
	{ EffectType::NoteDelay, { "0Gxx", QT_TR_NOOP("Note delay, xx: count (00-FF)") } },
	{ EffectType::AutoEnvelope, { "0Hxy", QT_TR_NOOP("Auto envelope, x: shift amount (0-F), y: shape (0-F)") } },
	{ EffectType::HardEnvHighPeriod, { "0Ixx", QT_TR_NOOP("Hardware envelope period 1, xx: high byte (00-FF)") } },
	{ EffectType::HardEnvLowPeriod, { "0Jxx", QT_TR_NOOP("Hardware envelope period 2, xx: low byte (00-FF)") } },
	{ EffectType::Groove, { "0Oxx", QT_TR_NOOP("Set groove xx") } },
	{ EffectType::Detune, { "0Pxx", QT_TR_NOOP("Detune, xx: pitch (00-FF)") } },
	{ EffectType::NoteSlideUp, { "0Qxy", QT_TR_NOOP("Note slide up, x: count (0-F), y: seminote (0-F)") } },
	{ EffectType::NoteSlideDown, { "0Rxy", QT_TR_NOOP("Note slide down, x: count (0-F), y: seminote (0-F)") } },
	{ EffectType::NoteCut, { "0Sxx", QT_TR_NOOP("Note cut, xx: count (01-FF)") } },
	{ EffectType::TransposeDelay, { "0Txy", QT_TR_NOOP("Transpose delay, x: count (1-7: up, 9-F: down), y: seminote (0-F)") } },
	{ EffectType::ToneNoiseMix, { "0Vxx", QT_TR_NOOP("Tone/Noise mix, xx: 00 = no sound, 01 = tone, 02 = noise, 03 = tone & noise") } },
	{ EffectType::MasterVolume, { "0Vxx", QT_TR_NOOP("Master volume, xx: volume (00-3F)") } },
	{ EffectType::NoisePitch, { "0Wxx", QT_TR_NOOP("Noise pitch, xx: pitch (00-1F)") } },
	{ EffectType::RegisterAddress0, { "0Xxx", QT_TR_NOOP("Register address bank 0, xx: address (00-6B)") } },
	{ EffectType::RegisterAddress1, { "0Yxx", QT_TR_NOOP("Register address bank 1, xx: address (00-6B)") } },
	{ EffectType::RegisterValue, { "0Zxx", QT_TR_NOOP("Register value set, xx: value (00-FF)") } },
	{ EffectType::ARControl, { "Axyy", QT_TR_NOOP("AR control, x: operator (1-4), yy: attack rate (00-1F)") } },
	{ EffectType::Brightness, { "B0xx", QT_TR_NOOP("Brightness, xx: relative value (01-FF)") } },
	{ EffectType::DRControl, { "Dxyy", QT_TR_NOOP("DR control, x: operator (1-4), yy: decay rate (00-1F)") } },
	{ EffectType::FBControl, { "FBxx", QT_TR_NOOP("FB control, xx: feedback value (00-07)") } },
	{ EffectType::MLControl, { "MLxy", QT_TR_NOOP("ML control, x: operator (1-4), y: multiple (0-F)") } },
	{ EffectType::VolumeDelay, { "Mxyy", QT_TR_NOOP("Volume delay, x: count (1-F), yy: volume (00-FF)") } },
	{ EffectType::RRControl, { "RRxy", QT_TR_NOOP("RR control, x: operator (1-4), y: release rate (0-F)") } },
	{ EffectType::TLControl, { "Txyy", QT_TR_NOOP("TL control, x: operator (1-4), yy: total level (00-7F)") } },
	{ EffectType::NoEffect, { "", QT_TR_NOOP("Invalid effect") } }
};

QString EffectDescription::getEffectFormat(const EffectType type)
{
	return details_.at(type).format;
}
QString EffectDescription::getEffectDescription(const EffectType type)
{
	return tr(details_.at(type).desc);
}

QString EffectDescription::getEffectFormatAndDetailString(const EffectType type)
{
	if (type == EffectType::NoEffect) return tr(details_.at(EffectType::NoEffect).desc);
	else return details_.at(type).mergedString();
}
