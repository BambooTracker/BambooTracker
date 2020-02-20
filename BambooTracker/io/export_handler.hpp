#pragma once

#include <vector>
#include "gd3_tag.hpp"
#include "s98_tag.hpp"
#include "binary_container.hpp"

class ExportHandler
{
public:
	static void writeVgm(BinaryContainer& container, int target, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
						 bool loopFlag, uint32_t loopPoint, uint32_t loopSamples, uint32_t totalSamples,
						 bool gd3TagEnabled, GD3Tag tag);
	static void writeS98(BinaryContainer& container, int target, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
						 bool loopFlag, uint32_t loopPoint, bool tagEnabled, S98Tag tag);

private:
	ExportHandler();
};

enum ExportTargetFlag
{
	/* target bits 0-2 : FM type */
	Export_YM2608 = 0,
	Export_YM2612 = 1,
	Export_YM2203 = 2,
	Export_FmMask = Export_YM2608|Export_YM2612|Export_YM2203,
	/* target bit 3 : SSG type */
	Export_InternalSsg = 0,
	Export_AY8910Psg = 4,
	Export_YM2149Psg = 8,
	Export_SsgMask = Export_InternalSsg|Export_AY8910Psg|Export_YM2149Psg,
};
