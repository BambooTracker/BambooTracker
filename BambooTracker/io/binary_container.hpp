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

#include <vector>
#include <cstdint>
#include <string>

namespace io
{
class BinaryContainer
{
public:
	explicit BinaryContainer(size_t defCapacity = 0);
	explicit BinaryContainer(std::vector<char> buf);
	size_t size() const;
	void clear();
	void reserve(size_t capacity);

	void setEndian(bool isLittleEndian);
	bool isLittleEndian() const;

	void appendInt8(const int8_t v);
	void appendUint8(const uint8_t v);
	void appendInt16(const int16_t v);
	void appendUint16(const uint16_t v);
	void appendInt32(const int32_t v);
	void appendUint32(const uint32_t v);
	void appendChar(const char c);
	void appendString(const std::string str);
	void appendArray(const uint8_t* array, size_t size);
	void appendVector(const std::vector<uint8_t>& vec);
	void appendVector(const std::vector<char>& vec);
	void appendBinaryContainer(const BinaryContainer& bc);

	void writeInt8(size_t offset, const int8_t v);
	void writeUint8(size_t offset, const uint8_t v);
	void writeInt16(size_t offset, const int16_t v);
	void writeUint16(size_t offset, const uint16_t v);
	void writeInt32(size_t offset, const int32_t v);
	void writeUint32(size_t offset, const uint32_t v);
	void writeChar(size_t offset, const char c);
	void writeString(size_t offset, const std::string str);

	int8_t readInt8(size_t offset) const;
	uint8_t readUint8(size_t offset) const;
	int16_t readInt16(size_t offset) const;
	uint16_t readUint16(size_t offset) const;
	int32_t readInt32(size_t offset) const;
	uint32_t readUint32(size_t offset) const;
	char readChar(size_t offset) const;
	std::string readString(size_t offset, size_t length) const;

	BinaryContainer getSubcontainer(size_t offset, size_t length) const;

	const char* getPointer() const;
	std::vector<uint8_t> toVector() const;

private:
	std::vector<char> buf_;
	bool isLE_;

	void append(std::vector<char> a);
	void write(size_t offset, std::vector<char> a);
	std::vector<unsigned char> read(size_t offset, size_t size) const;
};
}
