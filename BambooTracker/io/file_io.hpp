#pragma once

#include <memory>
#include <string>
#include <vector>
#include "module.hpp"
#include "instruments_manager.hpp"

class FileIO
{
public:
	static bool saveModule(std::string path, std::weak_ptr<Module> mod,
						   std::weak_ptr<InstrumentsManager> instMan);

private:
	FileIO() {}

	static const FMEnvelopeParameter ENV_FM_PARAMS[38];
};
