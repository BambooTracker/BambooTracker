#pragma once

#include <string>
#include "misc.hpp"

enum class EffectType
{
	NoEffect,
	Arpeggio, PortamentoUp, PortamentoDown, TonePortamento, Vibrato, Tremolo, Pan, VolumeSlide, PositionJump,
	SongEnd, PatternBreak, SpeedTempoChange, NoteDelay, Groove, Detune, NoteSlideUp, NoteSlideDown,
	NoteCut, TransposeDelay, MasterVolume, VolumeDelay, ToneNoiseMix, NoisePitch, HardEnvHighPeriod,
	HardEnvLowPeriod, AutoEnvelope, FBControl, TLControl
};

struct Effect
{
	EffectType type;
	int value;

	static EffectType toEffectType(SoundSource src, std::string id);
	static Effect makeEffectData(SoundSource src, std::string id, int value);
};
