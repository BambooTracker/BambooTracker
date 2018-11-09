#pragma once

#include <memory>
#include <string>
#include <vector>
#include "module.hpp"
#include "instruments_manager.hpp"
#include "binary_container.hpp"
#include "gd3_tag.hpp"

class FileIO
{
public:
	static bool saveModule(std::string path, std::weak_ptr<Module> mod,
						   std::weak_ptr<InstrumentsManager> instMan);
	static bool loadModuel(std::string path, std::weak_ptr<Module> mod,
						   std::weak_ptr<InstrumentsManager> instMan);
	static bool saveInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum);
	static AbstractInstrument* loadInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan,
											  int instNum);
	static bool writeWave(std::string path, std::vector<int16_t> samples, uint32_t rate);

	static bool writeVgm(std::string path, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
						 bool loopFlag, uint32_t loopPoint, uint32_t loopSamples, uint32_t totalSamples,
						 bool gd3TagEnabled, GD3Tag tag);

private:
	FileIO() {}

	static const FMEnvelopeParameter ENV_FM_PARAMS[38];

	static size_t loadInstrumentMemoryOperatorSequence(FMEnvelopeParameter param,
			size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan, BinaryContainer& ctr);
	static size_t loadInstrumentMemoryOperatorSequenceForInstrument(FMEnvelopeParameter param,
			size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan, BinaryContainer& ctr,
																	InstrumentFM* inst, int idx);
};
