#include "file_io_error.hpp"
#include "misc.hpp"

std::string FileIOError::fileTypeToString(const FileType type)
{
	switch (type) {
	case FileType::MOD:		return "module";
	case FileType::INST:	return "instrument";
	case FileType::WAV:		return "wav";
	case FileType::VGM:		return "vgm";
	}
}

/******************************/
FileInputError::FileInputError(const FileIOError::FileType type)
	: std::runtime_error(makeText(type))
{
}

std::string FileInputError::makeText(const FileIOError::FileType type)
{
	return "Failed to load the " + FileIOError::fileTypeToString(type) + ".";
}

/******************************/
FileOutputError::FileOutputError(const FileIOError::FileType type)
	: std::runtime_error(makeText(type))
{
}

std::string FileOutputError::makeText(const FileIOError::FileType type)
{
	return "Failed to save the " + FileIOError::fileTypeToString(type) + ".";
}

/******************************/
FileVersionError::FileVersionError(const uint32_t fileVersionBCD, const uint32_t appVersionBCD, const FileIOError::FileType type)
	: std::runtime_error(makeText(fileVersionBCD, appVersionBCD, type))
{
}


std::string FileVersionError::makeText(const uint32_t fileVersionBCD, const uint32_t appVersionBCD, const FileIOError::FileType type)
{
	uint8_t fmaj = bcdtoui(fileVersionBCD >> 16);
	uint8_t fmin = bcdtoui((fileVersionBCD & 0x00ff00) >> 8);
	uint8_t frev = bcdtoui(fileVersionBCD & 0x0000ff);
	std::string fver = std::to_string(fmaj) + "." + std::to_string(fmin) + "." + std::to_string(frev);
	uint8_t pmaj = bcdtoui(appVersionBCD >> 16);
	uint8_t pmin = bcdtoui((appVersionBCD & 0x00ff00) >> 8);
	uint8_t prev = bcdtoui(appVersionBCD & 0x0000ff);
	std::string pver = std::to_string(pmaj) + "." + std::to_string(pmin) + "." + std::to_string(prev);
	std::string typeStr = FileIOError::fileTypeToString(type);
	return "BambooTracker v" + pver + " does not support " + typeStr + " that its file version is " + fver + ".";
}

/******************************/
FileCorruptionError::FileCorruptionError(const FileIOError::FileType type)
	: std::runtime_error(makeText(type))
{
}

std::string FileCorruptionError::makeText(const FileIOError::FileType type)
{
	return "Could not load the " + FileIOError::fileTypeToString(type) + ". It may be corrupted.";
}
