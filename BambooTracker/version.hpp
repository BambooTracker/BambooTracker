#pragma once

#include <cstdint>
#include <string>

class Version
{
public:
	static uint32_t ofApplicationInBCD();
	static std::string ofApplicationInString();

	static uint32_t ofModuleFileInBCD();
	static std::string ofModuleFileInString();

	static uint32_t ofInstrumentFileInBCD();
	static std::string ofInstrumentFileInString();

private:
	// Application version
	static constexpr unsigned int appMajor			= 0;
	static constexpr unsigned int appMinor			= 1;
	static constexpr unsigned int appRevision		= 0;

	// Module file version
	static constexpr unsigned int modFileMajor		= 1;
	static constexpr unsigned int modFileMinor		= 0;
	static constexpr unsigned int modFileRevision	= 0;

	// Instrument file version
	static constexpr unsigned int instFileMajor		= 1;
	static constexpr unsigned int instFileMinor		= 0;
	static constexpr unsigned int instFileRevision	= 0;

	static uint32_t toBCD(unsigned int major, unsigned int minor, unsigned int revision);
	static std::string toString(unsigned int major, unsigned int minor, unsigned int revision);

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

inline uint32_t Version::toBCD(unsigned int major, unsigned int minor, unsigned int revision)
{
	return (major << 16) + (minor << 8) + revision;
}

inline std::string Version::toString(unsigned int major, unsigned int minor, unsigned int revision)
{
	return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(revision);
}
