/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <stdexcept>
#include <cstdint>
#include <string>
#include "io_file_type.hpp"

namespace io
{
class FileIOError : public std::runtime_error
{
protected:
	const FileType ftype_;

	FileIOError(const std::string& text, const FileType ftype)
		: std::runtime_error(text), ftype_(ftype) {}

public:
	inline FileType fileType() const { return ftype_; }
};

class FileNotExistError : public FileIOError
{
	FileNotExistError(const FileType type)
		: FileIOError("File not exist error", type) {}
};

class FileUnsupportedError : public FileIOError
{
public:
	FileUnsupportedError(const FileType type)
		: FileIOError("File unsupported error", type) {}
};

class FileVersionError : public FileIOError
{
public:
	FileVersionError(const FileType type)
		: FileIOError("File version error", type) {}
};

class FileCorruptionError : public FileIOError
{
private:
	size_t pos_;

public:
	FileCorruptionError(const FileType type, size_t pos, const std::string& desc = "File corruption error")
		: FileIOError(desc, type), pos_(pos) {}
	inline size_t position() const { return pos_; }
};
}
