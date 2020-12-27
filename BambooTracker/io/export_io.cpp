/*
 * Copyright (C) 2019-2020 Rerrah
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

#include "export_io.hpp"
#include "binary_container.hpp"
#include "io_file_type.hpp"
#include "file_io_error.hpp"

namespace io
{
void writeVgm(BinaryContainer& container, int target, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
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
	case Export_NoneFm:
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

	container.appendArray(header, 0x100);

	// Commands
	container.appendVector(samples);
	container.appendUint8(0x66);	// End

	// GD3 tag
	if (gd3TagEnabled) {
		// "Gd3 " ident
		container.appendString("Gd3 ");
		// Version [v1.00]
		uint32_t gd3Version = 0x100;
		container.appendUint32(gd3Version);
		// Data size
		container.appendUint32(tagDataLen);
		// Track name in english
		container.appendString(tag.trackNameEn);
		// Track name in japanes
		container.appendString(tag.trackNameJp);
		// Game name in english
		container.appendString(tag.gameNameEn);
		// Game name in japanese
		container.appendString(tag.gameNameJp);
		// System name in english
		container.appendString(tag.systemNameEn);
		// System name in japanese
		container.appendString(tag.systemNameJp);
		// Track author in english
		container.appendString(tag.authorEn);
		// Track author in japanese
		container.appendString(tag.authorJp);
		// Release date
		container.appendString(tag.releaseDate);
		// VGM creator
		container.appendString(tag.vgmCreator);
		// Notes
		container.appendString(tag.notes);
	}
}

void writeS98(BinaryContainer& container, int target, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
			  bool loopFlag, uint32_t loopPoint, bool tagEnabled, S98Tag tag)
{
	// Header
	// 0x00: Magic "S98"
	container.appendString("S98");
	// 0x03: Format version 3
	uint8_t version = 0x33;
	container.appendUint8(version);
	// 0x04: Timer info (sync numerator)
	uint32_t timeNum = 1;
	container.appendUint32(timeNum);
	// 0x08: Timer info 2 (sync denominator)
	container.appendUint32(rate);
	// 0x0c: Deprecated
	uint32_t zero = 0;
	container.appendUint32(zero);
	// 0x10: Tag offset
	uint32_t tagOffset = tagEnabled ? (0x80 + samples.size() + 1) : 0;
	container.appendUint32(tagOffset);
	// 0x14: Dump data offset
	uint32_t dumpOffset = 0x80;
	container.appendUint32(dumpOffset);
	// 0x18: Loop offset
	uint32_t loopOffset = loopFlag ? (0x80 + loopPoint) : 0;
	container.appendUint32(loopOffset);
	// 0x1c: Device count
	uint32_t deviceCnt = 0;
	if ((target & Export_FmMask) != Export_NoneFm)
		++deviceCnt;
	if ((target & Export_SsgMask) != Export_InternalSsg)
		++deviceCnt;
	container.appendUint32(deviceCnt);

	if ((target & Export_FmMask) != Export_NoneFm) {
		// 0x20-0x2f: Device info (if NoneFM, skipped)
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
		container.appendUint32(deviceType);
		// 0x24: Clock
		container.appendUint32(deviceClock);
		// 0x28: Pan (Unused)
		container.appendUint32(zero);
		// 0x2c: Reserved
		container.appendUint32(zero);
	}

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
		container.appendUint32(subdeviceType);
		// 0x34: Clock
		container.appendUint32(subdeviceClock);
		// 0x38: Pan (Unused)
		container.appendUint32(zero);
		// 0x3c: Reserved
		container.appendUint32(zero);
	}

	// 0x??-0x7f: Unused
	for (uint32_t i = 0; i < (24 - 4 * deviceCnt); ++i)
		container.appendUint32(zero);

	// Commands
	container.appendVector(samples);
	container.appendUint8(0xfd);	// End

	// GD3 tag
	if (tagEnabled) {
		// Tag ident
		container.appendString("[S98]");
		// BOM
		uint8_t bom[] = { 0xef, 0xbb, 0xbf };
		container.appendArray(bom, 3);

		uint8_t nl = 0x0a;

		// Title
		container.appendString("title=" + tag.title);
		container.appendUint8(nl);
		// Artist
		container.appendString("artist=" + tag.artist);
		container.appendUint8(nl);
		// Game
		container.appendString("game=" + tag.game);
		container.appendUint8(nl);
		// Year
		container.appendString("year=" + tag.year);
		container.appendUint8(nl);
		// Genre
		container.appendString("genre=" + tag.genre);
		container.appendUint8(nl);
		// Comment
		container.appendString("comment=" + tag.comment);
		container.appendUint8(nl);
		// Copyright
		container.appendString("copyright=" + tag.copyright);
		container.appendUint8(nl);
		// S98by
		container.appendString("s98by=" + tag.s98by);
		container.appendUint8(nl);
		// System
		container.appendString("system=" + tag.system);
		container.appendUint8(nl);

		uint8_t end = 0;
		container.appendUint8(end);
	}
}
}
