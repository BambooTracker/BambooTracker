#pragma once

#include <stdexcept>
#include <cstdint>
#include "file_io.hpp"

class FileIOError : public std::runtime_error
{
public:
	FileIO::FileType getFileType() const;

protected:
	FileIOError(std::string text, const FileIO::FileType type);
	const FileIO::FileType type_;
};

class FileInputError : public FileIOError
{
public:
	FileInputError(const FileIO::FileType type);
};

class FileOutputError : public FileIOError
{
public:
	FileOutputError(const FileIO::FileType type);
};

class FileVersionError : public FileIOError
{
public:
	FileVersionError(const FileIO::FileType type);
};

class FileCorruptionError : public FileIOError
{
public:
	FileCorruptionError(const FileIO::FileType type);
};
