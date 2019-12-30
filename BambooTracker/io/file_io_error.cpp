#include "file_io_error.hpp"
#include <string>
#include "misc.hpp"

FileIOError::FileIOError(std::string text, const FileIO::FileType type)
	: std::runtime_error(text), type_(type)
{
}

FileIO::FileType FileIOError::getFileType() const
{
	return type_;
}

/******************************/
FileInputError::FileInputError(const FileIO::FileType type)
	: FileIOError("File input error", type)
{
}

/******************************/
FileOutputError::FileOutputError(const FileIO::FileType type)
	: FileIOError("File output error", type)
{
}

/******************************/
FileVersionError::FileVersionError(const FileIO::FileType type)
	: FileIOError("File version error", type)
{
}

/******************************/
FileCorruptionError::FileCorruptionError(const FileIO::FileType type)
	: FileIOError("File corruption error", type)
{
}
