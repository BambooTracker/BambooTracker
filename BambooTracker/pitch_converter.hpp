#pragma once

#include <cstdint>
#include "misc.hpp"

class PitchConverter
{
public:
	static uint16_t getPitchFM(Note note, int octave, int fine);
	static uint16_t getPitchSSGSquare(Note note, int octave, int fine);
	static uint16_t getPitchSSGSquare(int n);
	static uint16_t getPitchSSGTriangle(Note note, int octave, int fine);
	static uint16_t getPitchSSGSaw(Note note, int octave, int fine);

private:
	static const uint16_t centTableFM_[384];
	static const uint16_t centTableSSGSquare_[384];
	static const uint16_t centTableSSGTriangle_[384];
	static const uint16_t centTableSSGSaw_[384];

	PitchConverter();
	static void calc(int& index, int& octave);
};
