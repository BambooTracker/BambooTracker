/*
	Encode and decode algorithms for
	Y8950/YM2608/YM2610 ADPCM-B

	2019 by superctr.
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define CD_CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define CD_CLAMP_ZERO(x, high)  (((x) > (high)) ? (high) : (x))

namespace codec
{
inline int16_t ymb_step(uint8_t step, int16_t* history, int16_t* step_size)
{
	static const int step_table[8] = {
		57, 57, 57, 57, 77, 102, 128, 153
	};

	int sign = step & 8;
	int delta = step & 7;
	int diff = ((1+(delta<<1)) * *step_size) >> 3;
	int newval = *history;
	int nstep = (step_table[delta] * *step_size) >> 6;
	if (sign > 0)
		newval -= diff;
	else
		newval += diff;
	//*step_size = CLAMP(nstep, 511, 32767);
	*step_size = CD_CLAMP(nstep, 127, 24576);
	*history = newval = CD_CLAMP(newval, -32768, 32767);
	return newval;
}

inline void ymb_encode(int16_t *buffer,uint8_t *outbuffer,long len)
{
	long i;
	int16_t step_size = 127;
	int16_t history = 0;
	uint8_t buf_sample = 0, nibble = 0;
	unsigned int adpcm_sample;

	for(i=0;i<len;i++)
	{
		// we remove a few bits of accuracy to reduce some noise.
		int step = ((*buffer++) & -8) - history;
		adpcm_sample = (abs(step)<<16) / (step_size<<14);
		adpcm_sample = CD_CLAMP_ZERO(adpcm_sample, 7);
		if(step < 0)
			adpcm_sample |= 8;
		if(nibble)
			*outbuffer++ = buf_sample | (adpcm_sample&15);
		else
			buf_sample = (adpcm_sample&15)<<4;
		nibble^=1;
		ymb_step(adpcm_sample, &history, &step_size);
	}
}

inline void ymb_decode(uint8_t *buffer,int16_t *outbuffer,long len)
{
	long i;

	int16_t step_size = 127;
	int16_t history = 0;
	uint8_t nibble = 0;

	for(i=0;i<len;i++)
	{
		int8_t step = (*(int8_t*)buffer)<<nibble;
		step >>= 4;
		if(nibble)
			buffer++;
		nibble^=4;
		*outbuffer++ = ymb_step(step, &history, &step_size);
	}
}
}

#undef CD_CLAMP
#undef CD_CLAMP_ZERO
