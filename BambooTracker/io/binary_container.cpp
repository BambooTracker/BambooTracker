#include "binary_container.hpp"
#include <algorithm>
#include <iterator>
#include <utility>

BinaryContainer::BinaryContainer(size_t defCapacity)
	: isLE_(true)
{
	if (defCapacity) reserve(defCapacity);
}

BinaryContainer::BinaryContainer(std::vector<char> buf)
	: buf_(buf),
	  isLE_(true)
{
}

size_t BinaryContainer::size() const
{
	return buf_.size();
}

void BinaryContainer::clear()
{
	buf_.clear();
	buf_.shrink_to_fit();
}

void BinaryContainer::reserve(size_t capacity)
{
	buf_.reserve(capacity);
}

void BinaryContainer::setEndian(bool isLittleEndian)
{
	isLE_ = isLittleEndian;
}

bool BinaryContainer::isLittleEndian() const
{
	return isLE_;
}

void BinaryContainer::appendInt8(const int8_t v)
{
	buf_.push_back(static_cast<char>(v));
}

void BinaryContainer::appendUint8(const uint8_t v)
{
	buf_.push_back(static_cast<char>(v));
}

void BinaryContainer::appendInt16(const int16_t v)
{
	append({
			   static_cast<char>(0x00ff & v),
			   static_cast<char>(v >> 8),
		   });
}

void BinaryContainer::appendUint16(const uint16_t v)
{
	append({
			   static_cast<char>(0x00ff & v),
			   static_cast<char>(v >> 8),
		   });
}

void BinaryContainer::appendInt32(const int32_t v)
{
	append({
			   static_cast<char>(0x000000ff & v),
			   static_cast<char>(0x0000ff & (v >> 8)),
			   static_cast<char>(0x00ff & (v >> 16)),
			   static_cast<char>(v >> 24),
		   });
}

void BinaryContainer::appendUint32(const uint32_t v)
{
	append({
			   static_cast<char>(0x000000ff & v),
			   static_cast<char>(0x0000ff & (v >> 8)),
			   static_cast<char>(0x00ff & (v >> 16)),
			   static_cast<char>(v >> 24),
		   });
}

void BinaryContainer::appendChar(const char c)
{
	buf_.push_back(c);
}

void BinaryContainer::appendString(const std::string str)
{
	std::copy(str.begin(), str.end(), std::back_inserter(buf_));
}

void BinaryContainer::appendArray(const uint8_t* array, size_t size)
{
	std::copy(array, array + size, std::back_inserter(buf_));
}

void BinaryContainer::appendVector(const std::vector<uint8_t> vec)
{
	std::copy(vec.begin(), vec.end(), std::back_inserter(buf_));
}

void BinaryContainer::appendVector(const std::vector<char> vec)
{
	std::copy(vec.begin(), vec.end(), std::back_inserter(buf_));
}

void BinaryContainer::writeInt8(size_t offset, const int8_t v)
{
	buf_.at(offset) = static_cast<char>(v);
}

void BinaryContainer::writeUint8(size_t offset, const uint8_t v)
{
	buf_.at(offset) = static_cast<char>(v);
}

void BinaryContainer::writeInt16(size_t offset, const int16_t v)
{
	write(offset, {
			  static_cast<char>(0x00ff & v),
			  static_cast<char>(v >> 8),
		  });
}

void BinaryContainer::writeUint16(size_t offset, const uint16_t v)
{
	write(offset, {
			  static_cast<char>(0x00ff & v),
			  static_cast<char>(v >> 8),
		  });
}

void BinaryContainer::writeInt32(size_t offset, const int32_t v)
{
	write(offset, {
			  static_cast<char>(0x000000ff & v),
			  static_cast<char>(0x0000ff & (v >> 8)),
			  static_cast<char>(0x00ff & (v >> 16)),
			  static_cast<char>(v >> 24),
		  });
}

void BinaryContainer::writeUint32(size_t offset, const uint32_t v)
{
	write(offset, {
			  static_cast<char>(0x000000ff & v),
			  static_cast<char>(0x0000ff & (v >> 8)),
			  static_cast<char>(0x00ff & (v >> 16)),
			  static_cast<char>(v >> 24),
		  });
}

void BinaryContainer::writeChar(size_t offset, const char c)
{
	buf_.at(offset) = c;
}

void BinaryContainer::writeString(size_t offset, const std::string str)
{
	std::copy(str.begin(), str.end(), buf_.begin() + static_cast<int>(offset));
}

int8_t BinaryContainer::readInt8(size_t offset) const
{
	return static_cast<int8_t>(buf_.at(offset));
}

uint8_t BinaryContainer::readUint8(size_t offset) const
{
	return static_cast<uint8_t>(buf_.at(offset));
}

int16_t BinaryContainer::readInt16(size_t offset) const
{
	std::vector<unsigned char> data = read(offset, 2);
	return static_cast<int16_t>(data[0] | (data[1] << 8));
}

uint16_t BinaryContainer::readUint16(size_t offset) const
{
	std::vector<unsigned char> data = read(offset, 2);
	return static_cast<uint16_t>(data[0] | (data[1] << 8));
}

int32_t BinaryContainer::readInt32(size_t offset) const
{
	std::vector<unsigned char> data = read(offset, 4);
	return static_cast<int32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

uint32_t BinaryContainer::readUint32(size_t offset) const
{
	std::vector<unsigned char> data = read(offset, 4);
	return static_cast<uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

char BinaryContainer::readChar(size_t offset) const
{
	return buf_.at(offset);
}

std::string BinaryContainer::readString(size_t offset, size_t length) const
{
	return std::string(buf_.begin() + static_cast<int>(offset), buf_.begin() + static_cast<int>(offset + length));
}

BinaryContainer BinaryContainer::getSubcontainer(size_t offset, size_t length) const
{
	std::vector<char> tmp;
	std::copy_n(buf_.begin() + static_cast<int>(offset), length, std::back_inserter(tmp));
	return BinaryContainer(std::move(tmp));
}

const char* BinaryContainer::getPointer() const
{
	return &buf_[0];
}

std::vector<uint8_t> BinaryContainer::toVector() const
{
	std::vector<uint8_t> vec;
	std::transform(buf_.begin(), buf_.end(), std::back_inserter(vec),
				   [](const char v) { return static_cast<uint8_t>(v); });
	return vec;
}

void BinaryContainer::append(std::vector<char> a)
{
	if (isLE_)
		std::copy(a.begin(), a.end(), std::back_inserter(buf_));
	else
		std::reverse_copy(a.begin(), a.end(), std::back_inserter(buf_));
}

void BinaryContainer::write(size_t offset, std::vector<char> a)
{
	if (isLE_)
		std::copy(a.begin(), a.end(), buf_.begin() + static_cast<int>(offset));
	else
		std::reverse_copy(a.begin(), a.end(), buf_.begin() + static_cast<int>(offset));
}

std::vector<unsigned char> BinaryContainer::read(size_t offset, size_t size) const
{
	std::vector<unsigned char> data;
	if (isLE_)
		std::copy(buf_.begin() + static_cast<int>(offset), buf_.begin() + static_cast<int>(offset + size), std::back_inserter(data));
	else
		std::reverse_copy(buf_.begin() + static_cast<int>(offset), buf_.begin() + static_cast<int>(offset + size), std::back_inserter(data));
	return data;
}
