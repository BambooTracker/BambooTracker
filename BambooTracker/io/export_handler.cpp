#include "export_handler.hpp"
#include <fstream>
#include "file_io.hpp"
#include "file_io_error.hpp"

ExportHandler::ExportHandler()
{
}

void ExportHandler::writeWave(std::string path, std::vector<int16_t> samples, uint32_t sampRate)
{
	try {
		std::ofstream ofs(path, std::ios::binary);

		// RIFF header
		ofs.write("RIFF", 4);
		uint32_t offset = samples.size() * sizeof(short) + 36;
		ofs.write(reinterpret_cast<char*>(&offset), 4);
		ofs.write("WAVE", 4);

		// fmt chunk
		ofs.write("fmt ", 4);
		uint32_t chunkOfs = 16;
		ofs.write(reinterpret_cast<char*>(&chunkOfs), 4);
		uint16_t fmtId = 1;
		ofs.write(reinterpret_cast<char*>(&fmtId), 2);
		uint16_t chCnt = 2;
		ofs.write(reinterpret_cast<char*>(&chCnt), 2);
		ofs.write(reinterpret_cast<char*>(&sampRate), 4);
		uint16_t bitSize = sizeof(int16_t) * 8;
		uint16_t blockSize = bitSize / 8 * chCnt;
		uint32_t byteRate = blockSize * sampRate;
		ofs.write(reinterpret_cast<char*>(&byteRate), 4);
		ofs.write(reinterpret_cast<char*>(&blockSize), 2);
		ofs.write(reinterpret_cast<char*>(&bitSize), 2);

		// Data chunk
		ofs.write("data", 4);
		uint32_t dataSize = samples.size() * bitSize / 8;
		ofs.write(reinterpret_cast<char*>(&dataSize), 4);
		ofs.write(reinterpret_cast<char*>(&samples[0]), static_cast<std::streamsize>(dataSize));
	}
	catch (...) {
		throw FileOutputError(FileIO::FileType::WAV);
	}
}

void ExportHandler::writeVgm(std::string path, int target, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
							 bool loopFlag, uint32_t loopPoint, uint32_t loopSamples, uint32_t totalSamples,
							 bool gd3TagEnabled, GD3Tag tag)
{
	uint32_t tagLen = 0;
	uint32_t tagDataLen = 0;
	if (gd3TagEnabled) {
		tagDataLen = tag.trackNameEn.length() + tag.trackNameJp.length()
					 + tag.gameNameEn.length() + tag.gameNameJp.length()
					 + tag.systemNameEn.length() + tag.systemNameJp.length()
					 + tag.authorEn.length() + tag.authorJp.length()
					 + tag.releaseDate.length() + tag.vgmCreator.length() + tag.notes.length();
		tagLen = 12 + tagDataLen;
	}

	try {
		std::ofstream ofs(path, std::ios::binary);

		// Header
		// 0x00: "Vgm " ident
		uint8_t header[0x100] = {'V', 'g', 'm', ' '};
		// 0x04: EOF offset
		uint32_t offset = 0x100 + samples.size() + 1 + tagLen - 4;
		*reinterpret_cast<uint32_t *>(header + 0x04) = offset;
		// 0x08: Version [v1.71]
		uint32_t version = 0x171;
		*reinterpret_cast<uint32_t *>(header + 0x08) = version;
		// 0x14: GD3 offset
		uint32_t gd3Offset = gd3TagEnabled ? (0x100 + samples.size() + 1 - 0x14) : 0;
		*reinterpret_cast<uint32_t *>(header + 0x14) = gd3Offset;
		// 0x18: Total # samples
		*reinterpret_cast<uint32_t *>(header + 0x18) = totalSamples;
		// 0x1c: Loop offset
		uint32_t loopOffset = loopFlag ? (loopPoint + 0x100 - 0x1c) : 0;
		*reinterpret_cast<uint32_t *>(header + 0x1c) = loopOffset;
		// 0x20: Loop # samples
		uint32_t loopSamps = loopFlag ? loopSamples : 0;
		*reinterpret_cast<uint32_t *>(header + 0x20) = loopSamps;
		// 0x24: Rate
		*reinterpret_cast<uint32_t *>(header + 0x24) = rate;
		// 0x34: VGM data offset
		uint32_t dataOffset = 0xcc;
		*reinterpret_cast<uint32_t *>(header + 0x34) = dataOffset;

		switch (target & Export_FmMask) {
		default:
		case Export_YM2608:
			// 0x48: YM2608 clock
			*reinterpret_cast<uint32_t *>(header + 0x48) = clock;
			break;
		case Export_YM2612:
			// 0x2c: YM2612 clock
			*reinterpret_cast<uint32_t *>(header + 0x2c) = clock;
			break;
		case Export_YM2203:
			// 0x44: YM2203 clock
			*reinterpret_cast<uint32_t *>(header + 0x44) = clock / 2;
			break;
		}

		switch (target & Export_SsgMask) {
		case Export_InternalSsg:
			break;
		default:
			// 0x74: AY8910 clock
			*reinterpret_cast<uint32_t *>(header + 0x74) = clock / 4;
			// 0x78: AY8910 chip type
			if ((target & Export_SsgMask) == Export_YM2149Psg)
				*reinterpret_cast<uint32_t *>(header + 0x78) = 0x10;
			// 0x79: AY8910 flags
			*reinterpret_cast<uint32_t *>(header + 0x79) = 0x01;
			break;
		}

		ofs.write(reinterpret_cast<char *>(header), 0x100);

		// Commands
		ofs.write(reinterpret_cast<char*>(&samples[0]), static_cast<std::streamsize>(samples.size()));
		uint8_t end = 0x66;
		ofs.write(reinterpret_cast<char*>(&end), 1);

		// GD3 tag
		if (gd3TagEnabled) {
			// "Gd3 " ident
			ofs.write("Gd3 ", 4);
			// Version [v1.00]
			uint32_t gd3Version = 0x100;
			ofs.write(reinterpret_cast<char*>(&gd3Version), 4);
			// Data size
			ofs.write(reinterpret_cast<char*>(&tagDataLen), 4);
			// Track name in english
			ofs.write(reinterpret_cast<char*>(&tag.trackNameEn[0]),
					static_cast<std::streamsize>(tag.trackNameEn.length()));
			// Track name in japanes
			ofs.write(reinterpret_cast<char*>(&tag.trackNameJp[0]),
					static_cast<std::streamsize>(tag.trackNameJp.length()));
			// Game name in english
			ofs.write(reinterpret_cast<char*>(&tag.gameNameEn[0]),
					static_cast<std::streamsize>(tag.gameNameEn.length()));
			// Game name in japanese
			ofs.write(reinterpret_cast<char*>(&tag.gameNameJp[0]),
					static_cast<std::streamsize>(tag.gameNameJp.length()));
			// System name in english
			ofs.write(reinterpret_cast<char*>(&tag.systemNameEn[0]),
					static_cast<std::streamsize>(tag.systemNameEn.length()));
			// System name in japanese
			ofs.write(reinterpret_cast<char*>(&tag.systemNameJp[0]),
					static_cast<std::streamsize>(tag.systemNameJp.length()));
			// Track author in english
			ofs.write(reinterpret_cast<char*>(&tag.authorEn[0]),
					static_cast<std::streamsize>(tag.authorEn.length()));
			// Track author in japanese
			ofs.write(reinterpret_cast<char*>(&tag.authorJp[0]),
					static_cast<std::streamsize>(tag.authorJp.length()));
			// Release date
			ofs.write(reinterpret_cast<char*>(&tag.releaseDate[0]),
					static_cast<std::streamsize>(tag.releaseDate.length()));
			// VGM creator
			ofs.write(reinterpret_cast<char*>(&tag.vgmCreator[0]),
					static_cast<std::streamsize>(tag.vgmCreator.length()));
			// Notes
			ofs.write(reinterpret_cast<char*>(&tag.notes[0]),
					static_cast<std::streamsize>(tag.notes.length()));
		}
	} catch (...) {
		throw FileOutputError(FileIO::FileType::VGM);
	}
}

void ExportHandler::writeS98(std::string path, int target, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
							 bool loopFlag, uint32_t loopPoint, bool tagEnabled, S98Tag tag)
{
	try {
		std::ofstream ofs(path, std::ios::binary);

		// Header
		// 0x00: Magic "S98"
		ofs.write("S98", 3);
		// 0x03: Format version 3
		uint8_t version = 0x33;
		ofs.write(reinterpret_cast<char*>(&version), 1);
		// 0x04: Timer info (sync numerator)
		uint32_t timeNum = 1;
		ofs.write(reinterpret_cast<char*>(&timeNum), 4);
		// 0x08: Timer info 2 (sync denominator)
		ofs.write(reinterpret_cast<char*>(&rate), 4);
		// 0x0c: Deprecated
		uint32_t zero = 0;
		ofs.write(reinterpret_cast<char*>(&zero), 4);
		// 0x10: Tag offset
		uint32_t tagOffset = tagEnabled ? (0x80 + samples.size() + 1) : 0;
		ofs.write(reinterpret_cast<char*>(&tagOffset), 4);
		// 0x14: Dump data offset
		uint32_t dumpOffset = 0x80;
		ofs.write(reinterpret_cast<char*>(&dumpOffset), 4);
		// 0x18: Loop offset
		uint32_t loopOffset = loopFlag ? (0x80 + loopPoint) : 0;
		ofs.write(reinterpret_cast<char*>(&loopOffset), 4);
		// 0x1c: Device count
		uint32_t deviceCnt = 1;
		if ((target & Export_SsgMask) != Export_InternalSsg)
			deviceCnt = 2;
		ofs.write(reinterpret_cast<char*>(&deviceCnt), 4);

		// 0x20-0x2f: Device info
		// 0x20: Device type
		uint32_t deviceType;
		uint32_t deviceClock;
		switch (target & Export_FmMask) {
		default:
		case Export_YM2608:
			deviceType = 4;	// OPNA
			deviceClock = clock;
			break;
		case Export_YM2612:
			deviceType = 3;	// OPN2
			deviceClock = clock;
			break;
		case Export_YM2203:
			deviceType = 2;	// OPN
			deviceClock = clock / 2;
			break;
		}
		ofs.write(reinterpret_cast<char*>(&deviceType), 4);
		// 0x24: Clock
		ofs.write(reinterpret_cast<char*>(&deviceClock), 4);
		// 0x28: Pan (Unused)
		ofs.write(reinterpret_cast<char*>(&zero), 4);
		// 0x2c: Reserved
		ofs.write(reinterpret_cast<char*>(&zero), 4);

		if ((target & Export_SsgMask) != Export_InternalSsg) {
			// 0x30-0x3f: Device info
			// 0x30: Device type
			uint32_t subdeviceType;
			uint32_t subdeviceClock;
			switch (target & Export_SsgMask) {
			default:
			case Export_AY8910Psg:
				subdeviceType = 15;	// PSG (AY-3-8910)
				subdeviceClock = clock / 4;
				break;
			case Export_YM2149Psg:
				subdeviceType = 1;		// PSG (YM 2149)
				subdeviceClock = clock / 2;
				break;
			}
			ofs.write(reinterpret_cast<char*>(&subdeviceType), 4);
			// 0x34: Clock
			ofs.write(reinterpret_cast<char*>(&subdeviceClock), 4);
			// 0x38: Pan (Unused)
			ofs.write(reinterpret_cast<char*>(&zero), 4);
			// 0x3c: Reserved
			ofs.write(reinterpret_cast<char*>(&zero), 4);
		}

		// 0x??-0x7f: Unused
		for (uint32_t i = 0; i < (24 - 4 * deviceCnt); ++i)
			ofs.write(reinterpret_cast<char*>(&zero), 4);

		// Commands
		ofs.write(reinterpret_cast<char*>(&samples[0]), static_cast<std::streamsize>(samples.size()));
		uint8_t end = 0xfd;
		ofs.write(reinterpret_cast<char*>(&end), 1);

		// GD3 tag
		if (tagEnabled) {
			// Tag ident
			ofs.write("[S98]", 5);
			// BOM
			uint8_t bom[] = { 0xef, 0xbb, 0xbf };
			ofs.write(reinterpret_cast<char*>(bom), 3);

			uint8_t nl = 0x0a;

			// Title
			ofs.write("title=", 6);
			ofs.write(reinterpret_cast<char*>(&tag.title[0]),
					static_cast<std::streamsize>(tag.title.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// Artist
			ofs.write("artist=", 7);
			ofs.write(reinterpret_cast<char*>(&tag.artist[0]),
					static_cast<std::streamsize>(tag.artist.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// Game
			ofs.write("game=", 5);
			ofs.write(reinterpret_cast<char*>(&tag.game[0]),
					static_cast<std::streamsize>(tag.game.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// Year
			ofs.write("year=", 5);
			ofs.write(reinterpret_cast<char*>(&tag.year[0]),
					static_cast<std::streamsize>(tag.year.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// Genre
			ofs.write("genre=", 6);
			ofs.write(reinterpret_cast<char*>(&tag.genre[0]),
					static_cast<std::streamsize>(tag.genre.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// Comment
			ofs.write("comment=", 8);
			ofs.write(reinterpret_cast<char*>(&tag.comment[0]),
					static_cast<std::streamsize>(tag.comment.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// Copyright
			ofs.write("copyright=", 10);
			ofs.write(reinterpret_cast<char*>(&tag.copyright[0]),
					static_cast<std::streamsize>(tag.copyright.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// S98by
			ofs.write("s98by=", 6);
			ofs.write(reinterpret_cast<char*>(&tag.s98by[0]),
					static_cast<std::streamsize>(tag.s98by.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);
			// System
			ofs.write("system=", 7);
			ofs.write(reinterpret_cast<char*>(&tag.system[0]),
					static_cast<std::streamsize>(tag.system.length()));
			ofs.write(reinterpret_cast<char*>(&nl), 1);

			uint8_t end = 0;
			ofs.write(reinterpret_cast<char*>(&end), 1);
		}
	} catch (...) {
		throw FileOutputError(FileIO::FileType::S98);
	}
}
