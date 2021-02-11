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

#include <cstdint>
#include <string>
#include <stdexcept>

class Version
{
public:
	static uint32_t ofApplicationInBCD();
	static std::string ofApplicationInString();

	static uint32_t ofModuleFileInBCD();
	static std::string ofModuleFileInString();

	static uint32_t ofInstrumentFileInBCD();
	static std::string ofInstrumentFileInString();

	static uint32_t ofBankFileInBCD();
	static std::string ofBankFileInString();

	static uint32_t toBCD(unsigned int major, unsigned int minor, unsigned int revision);
	static std::string toString(unsigned int major, unsigned int minor, unsigned int revision);

private:
	// Application version
	static constexpr unsigned int appMajor			= 0;
	static constexpr unsigned int appMinor			= 4;
	static constexpr unsigned int appRevision		= 6;

	// Module file version
	static constexpr unsigned int modFileMajor		= 1;
	static constexpr unsigned int modFileMinor		= 5;
	static constexpr unsigned int modFileRevision	= 0;

	// Instrument file version
	static constexpr unsigned int instFileMajor		= 1;
	static constexpr unsigned int instFileMinor		= 4;
	static constexpr unsigned int instFileRevision	= 0;

	// Bank file version
	static constexpr unsigned int bankFileMajor		= 1;
	static constexpr unsigned int bankFileMinor		= 2;
	static constexpr unsigned int bankFileRevision	= 0;

	Version() = default;

	static uint8_t uitobcd(const uint8_t v);
};

//===============================================
inline uint32_t Version::ofApplicationInBCD()
{
	return toBCD(appMajor, appMinor, appRevision);
}

inline std::string Version::ofApplicationInString()
{
	return toString(appMajor, appMinor, appRevision);
}

inline uint32_t Version::ofModuleFileInBCD()
{
	return toBCD(modFileMajor, modFileMinor, modFileRevision);
}

inline std::string Version::ofModuleFileInString()
{
	return toString(modFileMajor, modFileMinor, modFileRevision);
}

inline uint32_t Version::ofInstrumentFileInBCD()
{
	return toBCD(instFileMajor, instFileMinor, instFileRevision);
}

inline std::string Version::ofInstrumentFileInString()
{
	return toString(instFileMajor, instFileMinor, instFileRevision);
}

inline uint32_t Version::ofBankFileInBCD()
{
	return toBCD(bankFileMajor, bankFileMinor, bankFileRevision);
}

inline std::string Version::ofBankFileInString()
{
	return toString(bankFileMajor, bankFileMinor, bankFileRevision);
}

inline uint32_t Version::toBCD(unsigned int major, unsigned int minor, unsigned int revision)
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

inline uint8_t Version::uitobcd(const uint8_t v)
{
	if (v > 99) throw std::out_of_range("Out of range.");

	uint8_t high = v / 10;
	uint8_t low = v % 10;
	return static_cast<uint8_t>(high << 4) + low;
}
