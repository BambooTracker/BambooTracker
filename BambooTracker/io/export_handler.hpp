#pragma once

#include <string>
#include <vector>
#include "gd3_tag.hpp"
#include "s98_tag.hpp"

class ExportHandler
{
public:
	static void writeWave(std::string path, std::vector<int16_t> samples, uint32_t rate);
	static void writeVgm(std::string path, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
						 bool loopFlag, uint32_t loopPoint, uint32_t loopSamples, uint32_t totalSamples,
						 bool gd3TagEnabled, GD3Tag tag);
	static void writeS98(std::string path, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
						 bool loopFlag, uint32_t loopPoint, bool tagEnabled, S98Tag tag);

private:
	ExportHandler();
};
