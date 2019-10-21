#pragma once

#include <cstdint>
#include <string>
#include "misc.hpp"

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
	static constexpr unsigned int appMinor			= 2;
	static constexpr unsigned int appRevision		= 4;

	// Module file version
	static constexpr unsigned int modFileMajor		= 1;
	static constexpr unsigned int modFileMinor		= 3;
	static constexpr unsigned int modFileRevision	= 0;

	// Instrument file version
	static constexpr unsigned int instFileMajor		= 1;
	static constexpr unsigned int instFileMinor		= 2;
	static constexpr unsigned int instFileRevision	= 1;

	// Bank file version
	static constexpr unsigned int bankFileMajor		= 1;
	static constexpr unsigned int bankFileMinor		= 0;
	static constexpr unsigned int bankFileRevision	= 0;

	Version() {}
};

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
