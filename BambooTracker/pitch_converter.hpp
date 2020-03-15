#pragma once

#include <cstdint>
#include "misc.hpp"

class PitchConverter
{
public:
	static constexpr int SEMINOTE_PITCH = 32;

	static uint16_t getPitchFM(Note note, int octave, int pitch);
	static uint16_t getPitchSSGSquare(Note note, int octave, int pitch);
	static uint16_t getPitchSSGSquare(int n);
	static uint16_t getPitchSSGTriangle(Note note, int octave, int pitch);
	static uint16_t getPitchSSGSaw(Note note, int octave, int pitch);

	static int calculatePitchIndex(int octave, Note note, int pitch);

private:
	static const uint16_t centTableFM_[3072];
	static const uint16_t centTableSSGSquare_[3072];
	static const uint16_t centTableSSGTriangle_[3072];
	static const uint16_t centTableSSGSaw_[3072];

	PitchConverter();
};
