/*
 * Copyright (C) 2018-2022 Rerrah
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

#include "binary_container.hpp"
#include <algorithm>
#include <iterator>
#include <utility>
#include <stdexcept>

namespace io
{
BinaryContainer::BinaryContainer()
	: isLE_(true)
{
}

BinaryContainer::BinaryContainer(const std::vector<uint8_t>& buf)
	: isLE_(true)
{
	std::copy(buf.cbegin(), buf.cend(), std::back_inserter(buf_));
}

BinaryContainer::BinaryContainer(std::vector<uint8_t>&& buf)
	: isLE_(true)
{
	std::move(buf.begin(), buf.end(), std::back_inserter(buf_));
}

void BinaryContainer::clear()
{
	buf_.clear();
	buf_.shrink_to_fit();
}

void BinaryContainer::appendInt8(int8_t v)
{
	buf_.push_back(static_cast<uint8_t>(v));
}

void BinaryContainer::appendUint8(uint8_t v)
{
	buf_.push_back(v);
}

void BinaryContainer::appendInt16(int16_t v)
{
	append({
			   static_cast<uint8_t>(0x00ff & v),
			   static_cast<uint8_t>(v >> 8),
		   });
}

void BinaryContainer::appendUint16(uint16_t v)
{
	append({
			   static_cast<uint8_t>(0x00ff & v),
			   static_cast<uint8_t>(v >> 8),
		   });
}

void BinaryContainer::appendInt32(int32_t v)
{
	append({
			   static_cast<uint8_t>(0x000000ff & v),
			   static_cast<uint8_t>(0x0000ff & (v >> 8)),
			   static_cast<uint8_t>(0x00ff & (v >> 16)),
			   static_cast<uint8_t>(v >> 24),
		   });
}

void BinaryContainer::appendUint32(uint32_t v)
{
	append({
			   static_cast<uint8_t>(0x000000ff & v),
			   static_cast<uint8_t>(0x0000ff & (v >> 8)),
			   static_cast<uint8_t>(0x00ff & (v >> 16)),
			   static_cast<uint8_t>(v >> 24),
		   });
}

void BinaryContainer::appendChar(char c)
{
	buf_.push_back(static_cast<uint8_t>(c));
}

void BinaryContainer::appendString(const std::string& str)
{
	std::copy(str.cbegin(), str.cend(), std::back_inserter(buf_));
}

void BinaryContainer::appendArray(const uint8_t* array, size_type size)
{
	std::copy(array, array + size, std::back_inserter(buf_));
}

void BinaryContainer::appendVector(const std::vector<uint8_t>& vec)
{
	std::copy(vec.cbegin(), vec.cend(), std::back_inserter(buf_));
}

void BinaryContainer::appendVector(std::vector<uint8_t>&& vec)
{
	std::move(vec.begin(), vec.end(), std::back_inserter(buf_));
}

void BinaryContainer::appendBinaryContainer(const BinaryContainer& bc)
{
	std::copy(bc.cbegin(), bc.cend(), std::back_inserter(buf_));
}

void BinaryContainer::appendBinaryContainer(BinaryContainer&& bc)
{
	std::move(bc.begin(), bc.end(), std::back_inserter(buf_));
}

void BinaryContainer::writeInt8(size_type offset, int8_t v)
{
	buf_.at(offset) = static_cast<uint8_t>(v);
}

void BinaryContainer::writeUint8(size_type offset, uint8_t v)
{
	buf_.at(offset) = v;
}

void BinaryContainer::writeInt16(size_type offset, int16_t v)
{
	write(offset, {
			  static_cast<uint8_t>(0x00ff & v),
			  static_cast<uint8_t>(v >> 8),
		  });
}

void BinaryContainer::writeUint16(size_type offset, uint16_t v)
{
	write(offset, {
			  static_cast<uint8_t>(0x00ff & v),
			  static_cast<uint8_t>(v >> 8),
		  });
}

void BinaryContainer::writeInt32(size_type offset, int32_t v)
{
	write(offset, {
			  static_cast<uint8_t>(0x000000ff & v),
			  static_cast<uint8_t>(0x0000ff & (v >> 8)),
			  static_cast<uint8_t>(0x00ff & (v >> 16)),
			  static_cast<uint8_t>(v >> 24),
		  });
}

void BinaryContainer::writeUint32(size_type offset, uint32_t v)
{
	write(offset, {
			  static_cast<uint8_t>(0x000000ff & v),
			  static_cast<uint8_t>(0x0000ff & (v >> 8)),
			  static_cast<uint8_t>(0x00ff & (v >> 16)),
			  static_cast<uint8_t>(v >> 24),
		  });
}

void BinaryContainer::writeChar(size_type offset, char c)
{
	buf_.at(offset) = static_cast<uint8_t>(c);
}

void BinaryContainer::writeString(size_type offset, const std::string& str)
{
	if (buf_.size() <= offset || buf_.size() < offset + str.length())
		throw std::out_of_range("Invalid buffer range in binary container");

	std::copy(str.cbegin(), str.cend(), buf_.begin() + static_cast<int>(offset));
}

int8_t BinaryContainer::readInt8(size_type offset) const
{
	return static_cast<int8_t>(buf_.at(offset));
}

uint8_t BinaryContainer::readUint8(size_type offset) const
{
	return static_cast<uint8_t>(buf_.at(offset));
}

int16_t BinaryContainer::readInt16(size_type offset) const
{
	std::vector<uint8_t> data = read(offset, 2);
	return static_cast<int16_t>(data[0] | (data[1] << 8));
}

uint16_t BinaryContainer::readUint16(size_type offset) const
{
	std::vector<uint8_t> data = read(offset, 2);
	return static_cast<uint16_t>(data[0] | (data[1] << 8));
}

int32_t BinaryContainer::readInt32(size_type offset) const
{
	std::vector<uint8_t> data = read(offset, 4);
	return static_cast<int32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

uint32_t BinaryContainer::readUint32(size_type offset) const
{
	std::vector<uint8_t> data = read(offset, 4);
	return static_cast<uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

char BinaryContainer::readChar(size_type offset) const
{
	return buf_.at(offset);
}

std::string BinaryContainer::readString(size_type offset, size_type length) const
{
	if (buf_.size() <= offset || buf_.size() < offset + length)
		throw std::out_of_range("Invalid buffer range in binary container");

	return std::string(buf_.begin() + static_cast<int>(offset), buf_.begin() + static_cast<int>(offset + length));
}

BinaryContainer BinaryContainer::getSubcontainer(size_type offset, size_type length) const
{
	if (buf_.size() <= offset || buf_.size() < offset + length)
		throw std::out_of_range("Invalid buffer range in binary container");

	BinaryContainer sub;
	std::copy_n(buf_.begin() + static_cast<int>(offset), length, std::back_inserter(sub));
	return sub;
}

std::vector<uint8_t> BinaryContainer::toVector() const
{
	std::vector<uint8_t> vec;
	vec.reserve(buf_.size());
	std::copy(buf_.cbegin(), buf_.cend(), std::back_inserter(vec));
	return vec;
}

void BinaryContainer::append(const std::vector<uint8_t>&& a)
{
	if (isLE_)
		std::copy(a.cbegin(), a.cend(), std::back_inserter(buf_));
	else
		std::reverse_copy(a.cbegin(), a.cend(), std::back_inserter(buf_));
}

void BinaryContainer::write(size_t offset, const std::vector<uint8_t>&& a)
{
	if (buf_.size() <= offset || buf_.size() < offset + a.size())
		throw std::out_of_range("Invalid buffer range in binary container");

	if (isLE_)
		std::copy(a.cbegin(), a.cend(), buf_.begin() + static_cast<int>(offset));
	else
		std::reverse_copy(a.cbegin(), a.cend(), buf_.begin() + static_cast<int>(offset));
}

std::vector<uint8_t> BinaryContainer::read(size_type offset, size_type size) const
{
	if (buf_.size() <= offset || buf_.size() < offset + size)
		throw std::out_of_range("Invalid buffer range in binary container");

	std::vector<uint8_t> data;
	if (isLE_)
		std::copy(buf_.cbegin() + static_cast<int>(offset), buf_.cbegin() + static_cast<int>(offset + size), std::back_inserter(data));
	else
		std::reverse_copy(buf_.cbegin() + static_cast<int>(offset), buf_.cbegin() + static_cast<int>(offset + size), std::back_inserter(data));
	return data;
}
}
