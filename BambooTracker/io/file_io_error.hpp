#pragma once

#include <stdexcept>
#include <cstdint>
#include "file_io.hpp"

class FileInputError : public std::runtime_error
{
public:
	FileInputError(const FileIO::FileType type);

private:
	std::string makeText(const FileIO::FileType type);
};

class FileOutputError : public std::runtime_error
{
public:
	FileOutputError(const FileIO::FileType type);

private:
	std::string makeText(const FileIO::FileType type);
};

class FileVersionError : public std::runtime_error
{
public:
	FileVersionError(const uint32_t fileVersionBCD,
				 const uint32_t appVersionBCD, const FileIO::FileType type);

private:
	std::string makeText(const uint32_t fileVersionBCD,
						 const uint32_t appVersionBCD, const FileIO::FileType type);
};

class FileCorruptionError : public std::runtime_error
{
public:
	FileCorruptionError(const FileIO::FileType type);

private:
	std::string makeText(const FileIO::FileType type);
};
