#pragma once

#include <stdexcept>
#include <cstdint>
#include <string>

class FileIOError
{
public:
	enum class FileType
	{
		MOD, INST, WAV, VGM
	};

	static std::string fileTypeToString(const FileType type);
};

class FileInputError : public std::runtime_error
{
public:
	FileInputError(const FileIOError::FileType type);

private:
	std::string makeText(const FileIOError::FileType type);
};

class FileOutputError : public std::runtime_error
{
public:
	FileOutputError(const FileIOError::FileType type);

private:
	std::string makeText(const FileIOError::FileType type);
};

class FileVersionError : public std::runtime_error
{
public:
	FileVersionError(const uint32_t fileVersionBCD,
				 const uint32_t appVersionBCD, const FileIOError::FileType type);

private:
	std::string makeText(const uint32_t fileVersionBCD,
						 const uint32_t appVersionBCD, const FileIOError::FileType type);
};

class FileCorruptionError : public std::runtime_error
{
public:
	FileCorruptionError(const FileIOError::FileType type);

private:
	std::string makeText(const FileIOError::FileType type);
};
