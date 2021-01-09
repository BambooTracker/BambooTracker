/*
 * Copyright (C) 2018-2021 Rerrah
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

#include <deque>
#include <vector>
#include <cstdint>
#include <string>

namespace io
{
class BinaryContainer
{
public:
	using container_type = std::deque<uint8_t>;
	using value_type = container_type::value_type;
	using size_type = container_type::size_type;
	using iterator = container_type::iterator;
	using const_iterator = container_type::const_iterator;
	using reverse_iterator = container_type::reverse_iterator;
	using const_reverse_iterator = container_type::const_reverse_iterator;

	explicit BinaryContainer();
	explicit BinaryContainer(const std::vector<uint8_t>& buf);
	explicit BinaryContainer(std::vector<uint8_t>&& buf);

	inline iterator begin() noexcept { return buf_.begin(); }
	inline const_iterator begin() const noexcept { return buf_.begin(); }

	inline iterator end() noexcept { return buf_.end(); }
	inline const_iterator end() const noexcept { return buf_.end(); }

	inline const_iterator cbegin() const noexcept { return buf_.cbegin(); }
	inline const_iterator cend() const noexcept { return buf_.cend(); }

	inline reverse_iterator rbegin() noexcept { return buf_.rbegin(); }
	inline const_reverse_iterator rbegin() const noexcept { return buf_.rbegin(); }

	inline reverse_iterator rend() noexcept { return buf_.rend(); }
	inline const_reverse_iterator rend() const noexcept { return buf_.rend(); }

	inline const_reverse_iterator crbegin() const noexcept { return buf_.crbegin(); }
	inline const_reverse_iterator crend() const noexcept { return buf_.crend(); }

	inline void push_back(uint8_t v) { appendUint8(v); }

	inline size_type size() const noexcept { return buf_.size(); }
	void clear();
	inline void resize(size_type size) { buf_.resize(size); }

	inline void setEndian(bool isLittleEndian) noexcept { isLE_ = isLittleEndian; }
	inline bool isLittleEndian() const noexcept { return isLE_; }

	void appendInt8(int8_t v);
	void appendUint8(uint8_t v);
	void appendInt16(int16_t v);
	void appendUint16(uint16_t v);
	void appendInt32(int32_t v);
	void appendUint32(uint32_t v);
	void appendChar(char c);
	void appendString(const std::string& str);
	void appendArray(const uint8_t* array, size_type size);
	void appendVector(const std::vector<uint8_t>& vec);
	void appendVector(std::vector<uint8_t>&& vec);
	void appendBinaryContainer(const BinaryContainer& bc);
	void appendBinaryContainer(BinaryContainer&& bc);

	void writeInt8(size_type offset, int8_t v);
	void writeUint8(size_type offset, uint8_t v);
	void writeInt16(size_type offset, int16_t v);
	void writeUint16(size_type offset, uint16_t v);
	void writeInt32(size_type offset, int32_t v);
	void writeUint32(size_type offset, uint32_t v);
	void writeChar(size_type offset, char c);
	void writeString(size_type offset, const std::string& str);

	int8_t readInt8(size_type offset) const;
	uint8_t readUint8(size_type offset) const;
	int16_t readInt16(size_type offset) const;
	uint16_t readUint16(size_type offset) const;
	int32_t readInt32(size_type offset) const;
	uint32_t readUint32(size_type offset) const;
	char readChar(size_type offset) const;
	std::string readString(size_type offset, size_type length) const;

	BinaryContainer getSubcontainer(size_type offset, size_type length) const;

	std::vector<uint8_t> toVector() const;

private:
	container_type buf_;
	bool isLE_;

	void append(const std::vector<uint8_t>&& a);
	void write(size_t offset, const std::vector<uint8_t>&& a);
	std::vector<uint8_t> read(size_type offset, size_type size) const;
};
}
