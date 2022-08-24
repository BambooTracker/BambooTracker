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

#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>

// ***********
// The use of constexpr function was avoided because old `g++ -std=c++1y` does not fully support C++14.
// ***********

class Version
{
public:
	static /* constexpr */ uint32_t ofApplicationInBCD();
	static std::string ofApplicationInString();

	static /* constexpr */ uint32_t ofModuleFileInBCD();
	static std::string ofModuleFileInString();

	static /* constexpr */ uint32_t ofInstrumentFileInBCD();
	static std::string ofInstrumentFileInString();

	static /* constexpr */ uint32_t ofBankFileInBCD();
	static std::string ofBankFileInString();

	static /* constexpr */ uint32_t toBCD(unsigned int major, unsigned int minor, unsigned int revision);
	static std::string toString(unsigned int major, unsigned int minor, unsigned int revision);

private:
	// Application version
	static constexpr unsigned int appMajor			= 0;
	static constexpr unsigned int appMinor			= 5;
	static constexpr unsigned int appRevision		= 2;

	// Module file version
	static constexpr unsigned int modFileMajor		= 1;
	static constexpr unsigned int modFileMinor		= 6;
	static constexpr unsigned int modFileRevision	= 0;

	// Instrument file version
	static constexpr unsigned int instFileMajor		= 1;
	static constexpr unsigned int instFileMinor		= 5;
	static constexpr unsigned int instFileRevision	= 0;

	// Bank file version
	static constexpr unsigned int bankFileMajor		= 1;
	static constexpr unsigned int bankFileMinor		= 3;
	static constexpr unsigned int bankFileRevision	= 0;

	constexpr Version() = default;

	static /* constexpr */ uint8_t uitobcd(uint8_t v);
};

//===============================================
inline /* constexpr */ uint32_t Version::ofApplicationInBCD()
{
	return toBCD(appMajor, appMinor, appRevision);
}

inline std::string Version::ofApplicationInString()
{
	return toString(appMajor, appMinor, appRevision);
}

inline /* constexpr */ uint32_t Version::ofModuleFileInBCD()
{
	return toBCD(modFileMajor, modFileMinor, modFileRevision);
}

inline std::string Version::ofModuleFileInString()
{
	return toString(modFileMajor, modFileMinor, modFileRevision);
}

inline /* constexpr */ uint32_t Version::ofInstrumentFileInBCD()
{
	return toBCD(instFileMajor, instFileMinor, instFileRevision);
}

inline std::string Version::ofInstrumentFileInString()
{
	return toString(instFileMajor, instFileMinor, instFileRevision);
}

inline /* constexpr */ uint32_t Version::ofBankFileInBCD()
{
	return toBCD(bankFileMajor, bankFileMinor, bankFileRevision);
}

inline std::string Version::ofBankFileInString()
{
	return toString(bankFileMajor, bankFileMinor, bankFileRevision);
}

inline /* constexpr */ uint32_t Version::toBCD(unsigned int major, unsigned int minor, unsigned int revision)
{
	uint32_t maj = uitobcd(static_cast<uint8_t>(major));
	uint32_t min = uitobcd(static_cast<uint8_t>(minor));
	uint32_t rev = uitobcd(static_cast<uint8_t>(revision));
	return (maj << 16) + (min << 8) + rev;
}

inline std::string Version::toString(unsigned int major, unsigned int minor, unsigned int revision)
{
	return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(revision);
}

inline /* constexpr */ uint8_t Version::uitobcd(uint8_t v)
{
	 if (v > 99) throw std::out_of_range("[Version::uitobcd] Argument error: out of range");

	uint8_t high = v / 10u;
	uint8_t low = v % 10u;
	return static_cast<uint8_t>(high << 4) + low;
}
