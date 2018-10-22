#pragma once

#include <vector>
#include <cstdint>
#include <string>

class BinaryContainer
{
public:
	explicit BinaryContainer(size_t defCapacity = 0);
	size_t size() const;
	void clear();
	void reserve(size_t capacity);

	void setEndian(bool isLittleEndian);
	bool isLittleEndian() const;

	bool save(std::string path);

	void appendInt8(const int8_t v);
	void appendUint8(const uint8_t v);
	void appendInt16(const int16_t v);
	void appendUint16(const uint16_t v);
	void appendInt32(const int32_t v);
	void appendUint32(const uint32_t v);
	void appendChar(const char c);
	void appendString(const std::string str);

	void writeInt8(size_t offset, const int8_t v);
	void writeUint8(size_t offset, const uint8_t v);
	void writeInt16(size_t offset, const int16_t v);
	void writeUint16(size_t offset, const uint16_t v);
	void writeInt32(size_t offset, const int32_t v);
	void writeUint32(size_t offset, const uint32_t v);
	void writeChar(size_t offset, const char c);
	void writeString(size_t offset, const std::string str);

private:
	std::vector<char> buf_;
	bool isLE_;

	void append(std::vector<char> a);
	void write(size_t offset, std::vector<char> a);
};
