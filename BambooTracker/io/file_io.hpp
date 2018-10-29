#pragma once

#include <memory>
#include <string>
#include <vector>
#include "module.hpp"
#include "instruments_manager.hpp"
#include "binary_container.hpp"

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

private:
	FileIO() {}

	static const FMEnvelopeParameter ENV_FM_PARAMS[38];

	static size_t loadInstrumentMemoryOperatorSequence(FMEnvelopeParameter param,
			size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan, BinaryContainer& ctr);
	static size_t loadInstrumentMemoryOperatorSequenceForInstrument(FMEnvelopeParameter param,
			size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan, BinaryContainer& ctr,
																	InstrumentFM* inst, int idx);
};
