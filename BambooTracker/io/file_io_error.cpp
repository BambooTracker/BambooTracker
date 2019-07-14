#include "file_io_error.hpp"
#include <string>
#include "misc.hpp"

/******************************/
FileInputError::FileInputError(const FileIO::FileType type)
	: std::runtime_error(makeText(type))
{
}

std::string FileInputError::makeText(const FileIO::FileType type)
{
	return "Failed to load the " + FileIO::fileTypeToString(type) + ".";
}

/******************************/
FileOutputError::FileOutputError(const FileIO::FileType type)
	: std::runtime_error(makeText(type))
{
}

std::string FileOutputError::makeText(const FileIO::FileType type)
{
	return "Failed to save the " + FileIO::fileTypeToString(type) + ".";
}

/******************************/
FileVersionError::FileVersionError(const uint32_t fileVersionBCD, const uint32_t appVersionBCD, const FileIO::FileType type)
	: std::runtime_error(makeText(fileVersionBCD, appVersionBCD, type))
{
}


std::string FileVersionError::makeText(const uint32_t fileVersionBCD, const uint32_t appVersionBCD, const FileIO::FileType type)
{
	uint8_t fmaj = bcdtoui(static_cast<uint8_t>(fileVersionBCD >> 16));
	uint8_t fmin = bcdtoui((fileVersionBCD & 0x00ff00) >> 8);
	uint8_t frev = bcdtoui(fileVersionBCD & 0x0000ff);
	std::string fver = std::to_string(fmaj) + "." + std::to_string(fmin) + "." + std::to_string(frev);
	uint8_t pmaj = bcdtoui(static_cast<uint8_t>(appVersionBCD >> 16));
	uint8_t pmin = bcdtoui((appVersionBCD & 0x00ff00) >> 8);
	uint8_t prev = bcdtoui(appVersionBCD & 0x0000ff);
	std::string pver = std::to_string(pmaj) + "." + std::to_string(pmin) + "." + std::to_string(prev);
	std::string typeStr = FileIO::fileTypeToString(type);
	return "BambooTracker v" + pver + " does not support " + typeStr + " that its file version is " + fver + ".";
}

/******************************/
FileCorruptionError::FileCorruptionError(const FileIO::FileType type)
	: std::runtime_error(makeText(type))
{
}

std::string FileCorruptionError::makeText(const FileIO::FileType type)
{
	return "Could not load the " + FileIO::fileTypeToString(type) + ". It may be corrupted.";
}
