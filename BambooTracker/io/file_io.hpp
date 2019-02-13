#pragma once

#include <string>
#include "instruments_manager.hpp"

class FileIO
{
public:
	static const FMEnvelopeParameter ENV_FM_PARAMS[38];

	enum class FileType
	{
		MOD, INST, WAV, VGM, BANK, S98
	};

	static std::string fileTypeToString(const FileType type);

private:
	FileIO() {}
};
