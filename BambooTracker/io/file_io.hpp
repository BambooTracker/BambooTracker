#pragma once

#include <string>
#include "instruments_manager.hpp"

class FileIO
{
public:
	static const FMEnvelopeParameter ENV_FM_PARAMS[38];
	static const FMOperatorType OP_FM_TYPES[4];

	enum class FileType
	{
		Mod, Inst, WAV, VGM, Bank, S98, Unknown
	};

	static std::string getExtension(const std::string path);
	static FileType judgeFileTypeFromExtension(const std::string ext);

private:
	FileIO() {}
};
