/*
 * Copyright (C) 2020-2021 Rerrah
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

#include "bti_io.hpp"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <utility>
#include "enum_hash.hpp"
#include "version.hpp"
#include "file_io_error.hpp"
#include "note.hpp"
#include "io_utils.hpp"
#include "utils.hpp"

namespace io
{
namespace
{
size_t loadInstrumentPropertyOperatorSequenceForInstrument(
		FMEnvelopeParameter param, size_t instMemCsr,
		std::shared_ptr<InstrumentsManager>& instManLocked,
		const BinaryContainer& ctr, InstrumentFM* inst, int idx, uint32_t version)
{
	inst->setOperatorSequenceEnabled(param, true);
	inst->setOperatorSequenceNumber(param, idx);
	uint16_t ofs = ctr.readUint16(instMemCsr);
	size_t csr = instMemCsr + 2;

	uint16_t seqLen = ctr.readUint16(csr);
	csr += 2;
	for (uint16_t l = 0; l < seqLen; ++l) {
		uint16_t data = ctr.readUint16(csr);
		csr += 2;
		if (version < Version::toBCD(1, 2, 1)) csr += 2;
		if (l == 0)
			instManLocked->setOperatorSequenceFMSequenceData(param, idx, 0, data);
		else
			instManLocked->addOperatorSequenceFMSequenceData(param, idx, data);
	}

	uint16_t loopCnt = ctr.readUint16(csr);
	csr += 2;
	for (uint16_t l = 0; l < loopCnt; ++l) {
		int begin = ctr.readUint16(csr);
		csr += 2;
		int end = ctr.readUint16(csr);
		csr += 2;
		int times = ctr.readUint8(csr++);
		instManLocked->addOperatorSequenceFMLoop(param, idx, InstrumentSequenceLoop(begin, end, times));
	}

	switch (ctr.readUint8(csr++)) {
	case 0x00:	// No release
		instManLocked->setOperatorSequenceFMRelease(param, idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
		break;
	case 0x01:	// Fixed
	{
		uint16_t pos = ctr.readUint16(csr);
		csr += 2;
		// Release point check (prevents a bug)
		// https://github.com/rerrahkr/BambooTracker/issues/11
		if (pos < seqLen) instManLocked->setOperatorSequenceFMRelease(param, idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
		else instManLocked->setOperatorSequenceFMRelease(param, idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
		break;
	}
	default:
		throw FileCorruptionError(FileType::Inst, csr);
	}

	if (version >= Version::toBCD(1, 0, 1)) {
		++csr;	// Skip sequence type
	}

	return ofs;
}
}

BtiIO::BtiIO() : AbstractInstrumentIO("bti", "BambooTracker instrument", true, true) {}

AbstractInstrument* BtiIO::load(const BinaryContainer& ctr, const std::string& fileName,
								std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	(void)fileName;

	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerIst")
		throw FileCorruptionError(FileType::Inst, globCsr);
	globCsr += 16;
	/*size_t eofOfs = */ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofInstrumentFileInBCD())
		throw FileVersionError(FileType::Inst);
	globCsr += 4;


	/***** Instrument section *****/
	if (ctr.readString(globCsr, 8) != "INSTRMNT")
		throw FileCorruptionError(FileType::Inst, globCsr);
	else {
		globCsr += 8;
		size_t instOfs = ctr.readUint32(globCsr);
		size_t instCsr = globCsr + 4;
		size_t nameLen = ctr.readUint32(instCsr);
		instCsr += 4;
		std::string name = u8"";
		if (nameLen > 0) {
			name = ctr.readString(instCsr, nameLen);
			instCsr += nameLen;
		}
		std::unordered_map<FMOperatorType, int> fmArpMap, fmPtMap;
		std::unordered_map<int, std::vector<int>> kitSampFileMap, kitSampMap;
		AbstractInstrument* inst = nullptr;
		switch (ctr.readUint8(instCsr++)) {
		case 0x00:	// FM
		{
			inst = new InstrumentFM(instNum, name, instManLocked.get());
			auto fm = dynamic_cast<InstrumentFM*>(inst);
			uint8_t tmp = ctr.readUint8(instCsr++);
			fm->setEnvelopeResetEnabled(FMOperatorType::All, (tmp & 0x01));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op1, (tmp & 0x02));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op2, (tmp & 0x04));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op3, (tmp & 0x08));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op4, (tmp & 0x10));
			if (fileVersion >= Version::toBCD(1, 1, 0)) {
				fmArpMap.emplace(FMOperatorType::All, ctr.readUint8(instCsr++));
				for (auto& t : FM_OP_TYPES) {
					tmp = ctr.readUint8(instCsr++);
					if (!(tmp & 0x80)) fmArpMap.emplace(t, (tmp & 0x7f));
				}
				fmPtMap.emplace(FMOperatorType::All, ctr.readUint8(instCsr++));
				for (auto& t : FM_OP_TYPES) {
					tmp = ctr.readUint8(instCsr++);
					if (!(tmp & 0x80)) fmPtMap.emplace(t, (tmp & 0x7f));
				}
			}
			break;
		}
		case 0x01:	// SSG
		{
			inst = new InstrumentSSG(instNum, name, instManLocked.get());
			break;
		}
		case 0x02:	// ADPCM
		{
			inst = new InstrumentADPCM(instNum, name, instManLocked.get());
			break;
		}
		case 0x03:	// Drumkit
		{
			inst = new InstrumentDrumkit(instNum, name, instManLocked.get());
			auto kit = dynamic_cast<InstrumentDrumkit*>(inst);
			uint8_t cnt = ctr.readUint8(instCsr++);
			for (uint8_t i = 0; i < cnt; ++i) {
				int key = ctr.readUint8(instCsr++);
				int samp = ctr.readUint8(instCsr++);
				if (kitSampFileMap.count(samp)) kitSampFileMap[samp].push_back(key);
				else kitSampFileMap[samp] = { key };
				kit->setSampleEnabled(key, true);
				kit->setPitch(key, ctr.readInt8(instCsr++));
			}
			break;
		}
		default:
			throw FileCorruptionError(FileType::Inst, instCsr);
		}
		globCsr += instOfs;


		/***** Instrument property section *****/
		if (ctr.readString(globCsr, 8) != "INSTPROP")
			throw FileCorruptionError(FileType::Inst, globCsr);
		else {
			globCsr += 8;
			size_t instPropOfs = ctr.readUint32(globCsr);
			size_t instPropCsr = globCsr + 4;
			size_t instPropCsrTmp = instPropCsr;
			globCsr += instPropOfs;
			int kitSampCnt = 0;
			int adpcmSampIdx = 0;
			std::vector<int> nums;
			// Check memory range
			while (instPropCsr < globCsr) {
				switch (ctr.readUint8(instPropCsr++)) {
				case 0x00:	// FM envelope
				{
					nums.push_back(instManLocked->findFirstAssignableEnvelopeFM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x01:	// FM LFO
				{
					nums.push_back(instManLocked->findFirstAssignableLFOFM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x02:	// FM AL
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AL));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x03:	// FM FB
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::FB));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x04:	// FM AR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x05:	// FM DR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x06:	// FM SR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x07:	// FM RR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x08:	// FM SL1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x09:	// FM TL1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0a:	// FM KS1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0b:	// FM ML1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0c:	// FM DT1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT1));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0d:	// FM AR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0e:	// FM DR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0f:	// FM SR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x10:	// FM RR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x11:	// FM SL2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x12:	// FM TL2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x13:	// FM KS2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x14:	// FM ML2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x15:	// FM DT2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT2));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x16:	// FM AR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x17:	// FM DR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x18:	// FM SR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x19:	// FM RR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1a:	// FM SL3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1b:	// FM TL3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1c:	// FM KS3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1d:	// FM ML3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1e:	// FM DT3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT3));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1f:	// FM AR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x20:	// FM DR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x21:	// FM SR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x22:	// FM RR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x23:	// FM SL4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x24:	// FM TL4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x25:	// FM KS4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x26:	// FM ML4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x27:	// FM DT4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT4));
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x28:	// FM arpeggio
				{
					nums.push_back(instManLocked->findFirstAssignableArpeggioFM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x29:	// FM pitch
				{
					nums.push_back(instManLocked->findFirstAssignablePitchFM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x2a:	// FM pan
				{
					nums.push_back(instManLocked->findFirstAssignablePanFM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x30:	// SSG waveform
				{
					nums.push_back(instManLocked->findFirstAssignableWaveformSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x31:	// SSG tone/noise
				{
					nums.push_back(instManLocked->findFirstAssignableToneNoiseSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x32:	// SSG envelope
				{
					nums.push_back(instManLocked->findFirstAssignableEnvelopeSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x33:	// SSG arpeggio
				{
					nums.push_back(instManLocked->findFirstAssignableArpeggioSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x34:	// SSG pitch
				{
					nums.push_back(instManLocked->findFirstAssignablePitchSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x40:	// ADPCM sample
				{
					adpcmSampIdx = instManLocked->findFirstAssignableSampleADPCM(adpcmSampIdx);
					if (adpcmSampIdx == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					nums.push_back(adpcmSampIdx);
					if (inst->getType() == InstrumentType::Drumkit) {
						kitSampMap[adpcmSampIdx] = kitSampFileMap.at(kitSampCnt++);
					}
					instPropCsr += ctr.readUint16(instPropCsr);
					++adpcmSampIdx; // Increment for search appropriate kit sample
					break;
				}
				case 0x41:	// ADPCM envelope
				{
					nums.push_back(instManLocked->findFirstAssignableEnvelopeADPCM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x42:	// ADPCM arpeggio
				{
					nums.push_back(instManLocked->findFirstAssignableArpeggioADPCM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x43:	// ADPCM pitch
				{
					nums.push_back(instManLocked->findFirstAssignablePitchADPCM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x44:	// ADPCM pan
				{
					nums.push_back(instManLocked->findFirstAssignablePanADPCM());
					if (nums.back() == -1) throw FileCorruptionError(FileType::Inst, instPropCsr);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				default:
					throw FileCorruptionError(FileType::Inst, instPropCsr);
				}
			}
			// Read data
			instPropCsr = instPropCsrTmp;
			auto numIt = nums.begin();
			while (instPropCsr < globCsr) {
				switch (ctr.readUint8(instPropCsr++)) {
				case 0x00:	// FM envelope
				{
					int idx = *numIt++;
					dynamic_cast<InstrumentFM*>(inst)->setEnvelopeNumber(idx);
					uint8_t ofs = ctr.readUint8(instPropCsr);
					size_t csr = instPropCsr + 1;
					uint8_t tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AL, tmp >> 4);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::FB, tmp & 0x0f);
					for (int op = 0; op < 4; ++op) {
						auto& params = FM_OP_PARAMS[op];
						tmp = ctr.readUint8(csr++);
						instManLocked->setEnvelopeFMOperatorEnabled(idx, op, (0x20 & tmp) ? true : false);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::AR), tmp & 0x1f);
						tmp = ctr.readUint8(csr++);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::KS), tmp >> 5);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::DR), tmp & 0x1f);
						tmp = ctr.readUint8(csr++);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::DT), tmp >> 5);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::SR), tmp & 0x1f);
						tmp = ctr.readUint8(csr++);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::SL), tmp >> 4);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::RR), tmp & 0x0f);
						tmp = ctr.readUint8(csr++);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::TL), tmp);
						tmp = ctr.readUint8(csr++);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::ML), tmp & 0x0f);
						instManLocked->setEnvelopeFMParameter(idx, params.at(FMOperatorParameter::SSGEG),
															  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
					}
					instPropCsr += ofs;
					break;
				}
				case 0x01:	// FM LFO
				{
					int idx = *numIt++;
					auto fm = dynamic_cast<InstrumentFM*>(inst);
					fm->setLFOEnabled(true);
					fm->setLFONumber(idx);
					uint8_t ofs = ctr.readUint8(instPropCsr);
					size_t csr = instPropCsr + 1;
					uint8_t tmp = ctr.readUint8(csr++);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::FREQ, tmp >> 4);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::PMS, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::AMS, tmp & 0x0f);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::AM1, (tmp & 0x10) ? true : false);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::AM2, (tmp & 0x20) ? true : false);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::AM3, (tmp & 0x40) ? true : false);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::AM4, (tmp & 0x80) ? true : false);
					tmp = ctr.readUint8(csr++);
					instManLocked->setLFOFMParameter(idx, FMLFOParameter::Count, tmp);
					instPropCsr += ofs;
					break;
				}
				case 0x02:	// FM AL
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AL, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x03:	// FM FB
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::FB, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x04:	// FM AR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x05:	// FM DR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x06:	// FM SR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x07:	// FM RR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x08:	// FM SL1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x09:	// FM TL1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0a:	// FM KS1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0b:	// FM ML1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0c:	// FM DT1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT1, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0d:	// FM AR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0e:	// FM DR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0f:	// FM SR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x10:	// FM RR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x11:	// FM SL2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x12:	// FM TL2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x13:	// FM KS2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x14:	// FM ML2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x15:	// FM DT2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT2, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x16:	// FM AR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x17:	// FM DR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x18:	// FM SR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x19:	// FM RR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1a:	// FM SL3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1b:	// FM TL3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1c:	// FM KS3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1d:	// FM ML3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1e:	// FM DT3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT3, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1f:	// FM AR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x20:	// FM DR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x21:	// FM SR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x22:	// FM RR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x23:	// FM SL4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x24:	// FM TL4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x25:	// FM KS4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x26:	// FM ML4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x27:	// FM DT4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT4, instPropCsr, instManLocked, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x28:	// FM arpeggio
				{
					int idx = *numIt++;
					auto fm = dynamic_cast<InstrumentFM*>(inst);
					if (fileVersion >= Version::toBCD(1, 1, 0)) {
						std::vector<FMOperatorType> del;
						for (auto& pair : fmArpMap) {
							if (pair.second-- == 0) {
								fm->setArpeggioEnabled(pair.first, true);
								fm->setArpeggioNumber(pair.first, idx);
								del.push_back(pair.first);
							}
						}
						for (auto t : del) fmArpMap.erase(t);
					}
					else {
						fm->setArpeggioEnabled(FMOperatorType::All, true);
						fm->setArpeggioNumber(FMOperatorType::All, idx);
					}
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 0)) csr += 2;
						if (l == 0)
							instManLocked->setArpeggioFMSequenceData(idx, 0, data);
						else
							instManLocked->addArpeggioFMSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addArpeggioFMLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setArpeggioFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setArpeggioFMType(idx, SequenceType::AbsoluteSequence);
							break;
						case 0x01:	// Fixed
							instManLocked->setArpeggioFMType(idx, SequenceType::FixedSequence);
							break;
						case 0x02:	// Relative
							instManLocked->setArpeggioFMType(idx, SequenceType::RelativeSequence);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setArpeggioFMType(idx, SequenceType::AbsoluteSequence);
								break;
							}
							else {
								throw FileCorruptionError(FileType::Inst, csr);
							}
						}
					}

					instPropCsr += ofs;
					break;
				}
				case 0x29:	// FM pitch
				{
					int idx = *numIt++;
					auto fm = dynamic_cast<InstrumentFM*>(inst);
					if (fileVersion >= Version::toBCD(1, 1, 0)) {
						std::vector<FMOperatorType> del;
						for (auto& pair : fmPtMap) {
							if (pair.second-- == 0) {
								fm->setPitchEnabled(pair.first, true);
								fm->setPitchNumber(pair.first, idx);
								del.push_back(pair.first);
							}
						}
						for (auto t : del) fmPtMap.erase(t);
					}
					else {
						fm->setPitchEnabled(FMOperatorType::All, true);
						fm->setPitchNumber(FMOperatorType::All, idx);
					}
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 0)) csr += 2;
						if (l == 0)
							instManLocked->setPitchFMSequenceData(idx, 0, data);
						else
							instManLocked->addPitchFMSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addPitchFMLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPitchFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPitchFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setPitchFMType(idx, SequenceType::AbsoluteSequence);
							break;
						case 0x02:	// Relative
							instManLocked->setPitchFMType(idx, SequenceType::RelativeSequence);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setPitchFMType(idx, SequenceType::AbsoluteSequence);
								break;
							}
							else {
								throw FileCorruptionError(FileType::Inst, csr);
							}
						}
					}

					instPropCsr += ofs;
					break;
				}
				case 0x2a:	// FM pan
				{
					int idx = *numIt++;
					auto fm = dynamic_cast<InstrumentFM*>(inst);
					fm->setPanEnabled(true);
					fm->setPanNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (l == 0)
							instManLocked->setPanFMSequenceData(idx, 0, data);
						else
							instManLocked->addPanFMSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addPanFMLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPanFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPanFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPanFMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					instPropCsr += ofs;
					break;
				}
				case 0x30:	// SSG waveform
				{
					int idx = *numIt++;
					auto ssg = dynamic_cast<InstrumentSSG*>(inst);
					ssg->setWaveformEnabled(true);
					ssg->setWaveformNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 0)) {
							if (data == 3) data = SSGWaveformType::SQM_TRIANGLE;
							else if (data == 4) data = SSGWaveformType::SQM_SAW;
						}
						int32_t subdata;
						if (fileVersion >= Version::toBCD(1, 2, 0)) {
							subdata = ctr.readInt32(csr);
							csr += 4;
						}
						else {
							subdata = ctr.readUint16(csr);
							csr += 2;
							if (subdata != -1)
								subdata = note_utils::calculateSSGSquareTP(subdata, 0);
						}
						SSGWaveformUnit unit;
						switch (data) {
						case SSGWaveformType::SQM_TRIANGLE:
						case SSGWaveformType::SQM_SAW:
						case SSGWaveformType::SQM_INVSAW:
							unit = SSGWaveformUnit::makeUnitWithDecode(data, subdata);
							break;
						default:
							unit = SSGWaveformUnit::makeOnlyDataUnit(data);
							break;
						}
						if (l == 0)
							instManLocked->setWaveformSSGSequenceData(idx, 0, unit);
						else
							instManLocked->addWaveformSSGSequenceData(idx, unit);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addWaveformSSGLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setWaveformSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setWaveformSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setWaveformSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						++csr;	// Skip sequence type
					}

					instPropCsr += ofs;
					break;
				}
				case 0x31:	// SSG tone/noise
				{
					int idx = *numIt++;
					auto ssg = dynamic_cast<InstrumentSSG*>(inst);
					ssg->setToneNoiseEnabled(true);
					ssg->setToneNoiseNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 2)) {
							if (data > 0) {
								uint16_t tmp = data - 1;
								data = tmp / 32 * 32 + (31 - tmp % 32) + 1;
							}
						}
						if (fileVersion < Version::toBCD(1, 2, 0)) csr += 2;
						if (l == 0)
							instManLocked->setToneNoiseSSGSequenceData(idx, 0, data);
						else
							instManLocked->addToneNoiseSSGSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addToneNoiseSSGLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setToneNoiseSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setToneNoiseSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setToneNoiseSSGRelease(idx,  InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						++csr;	// Skip sequence type
					}

					instPropCsr += ofs;
					break;
				}
				case 0x32:	// SSG envelope
				{
					int idx = *numIt++;
					auto ssg = dynamic_cast<InstrumentSSG*>(inst);
					ssg->setEnvelopeEnabled(true);
					ssg->setEnvelopeNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						int32_t subdata;
						if (fileVersion >= Version::toBCD(1, 2, 0)) {
							subdata = ctr.readInt32(csr);
							csr += 4;
						}
						else {
							subdata = ctr.readUint16(csr);
							csr += 2;
						}
						SSGEnvelopeUnit unit = (data < 16) ? SSGEnvelopeUnit::makeOnlyDataUnit(data)
														   : SSGEnvelopeUnit::makeUnitWithDecode(data, subdata);
						if (l == 0)
							instManLocked->setEnvelopeSSGSequenceData(idx, 0, unit);
						else
							instManLocked->addEnvelopeSSGSequenceData(idx, unit);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addEnvelopeSSGLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setEnvelopeSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::AbsoluteRelease, pos));
						else instManLocked->setEnvelopeSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::RelativeRelease, pos));
						else instManLocked->setEnvelopeSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						++csr;	// Skip sequence type
					}

					instPropCsr += ofs;
					break;
				}
				case 0x33:	// SSG arpeggio
				{
					int idx = *numIt++;
					auto ssg = dynamic_cast<InstrumentSSG*>(inst);
					ssg->setArpeggioEnabled(true);
					ssg->setArpeggioNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 0)) csr += 2;
						if (l == 0)
							instManLocked->setArpeggioSSGSequenceData(idx, 0, data);
						else
							instManLocked->addArpeggioSSGSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addArpeggioSSGLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setArpeggioSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setArpeggioSSGType(idx, SequenceType::AbsoluteSequence);
							break;
						case 0x01:	// Fixed
							instManLocked->setArpeggioSSGType(idx, SequenceType::FixedSequence);
							break;
						case 0x02:	// Relative
							instManLocked->setArpeggioSSGType(idx, SequenceType::RelativeSequence);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setArpeggioSSGType(idx, SequenceType::AbsoluteSequence);
								break;
							}
							else {
								throw FileCorruptionError(FileType::Inst, csr);
							}
						}
					}

					instPropCsr += ofs;
					break;
				}
				case 0x34:	// SSG pitch
				{
					int idx = *numIt++;
					auto ssg = dynamic_cast<InstrumentSSG*>(inst);
					ssg->setPitchEnabled(true);
					ssg->setPitchNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 0)) csr += 2;
						if (l == 0)
							instManLocked->setPitchSSGSequenceData(idx, 0, data);
						else
							instManLocked->addPitchSSGSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addPitchSSGLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPitchSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPitchSSGRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setPitchSSGType(idx, SequenceType::AbsoluteSequence);
							break;
						case 0x02:	// Relative
							instManLocked->setPitchSSGType(idx, SequenceType::RelativeSequence);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setPitchSSGType(idx, SequenceType::AbsoluteSequence);
								break;
							}
							else {
								throw FileCorruptionError(FileType::Inst, csr);
							}
						}
					}

					instPropCsr += ofs;
					break;
				}
				case 0x40:	// ADPCM sample
				{
					int idx = *numIt++;
					if (inst->getType() == InstrumentType::ADPCM) {
						auto adpcm = dynamic_cast<InstrumentADPCM*>(inst);
						adpcm->setSampleNumber(idx);
					}
					else if (inst->getType() == InstrumentType::Drumkit) {
						auto kit = dynamic_cast<InstrumentDrumkit*>(inst);
						for (const int& key : kitSampMap.at(idx))
							kit->setSampleNumber(key, idx);
					}
					uint32_t ofs = ctr.readUint32(instPropCsr);
					size_t csr = instPropCsr + 4;

					instManLocked->setSampleADPCMRootKeyNumber(idx, ctr.readUint8(csr++));
					instManLocked->setSampleADPCMRootDeltaN(idx, ctr.readUint16(csr));
					csr += 2;
					instManLocked->setSampleADPCMRepeatEnabled(idx, (ctr.readUint8(csr++) & 0x01) != 0);
					uint32_t len = ctr.readUint32(csr);
					csr += 4;
					std::vector<uint8_t> samples = ctr.getSubcontainer(csr, len).toVector();
					/* csr += len; */
					instManLocked->storeSampleADPCMRawSample(idx, std::move(samples));

					instPropCsr += ofs;
					break;
				}
				case 0x41:	// ADPCM envelope
				{
					int idx = *numIt++;
					auto adpcm = dynamic_cast<InstrumentADPCM*>(inst);
					adpcm->setEnvelopeEnabled(true);
					adpcm->setEnvelopeNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (l == 0)
							instManLocked->setEnvelopeADPCMSequenceData(idx, 0, data);
						else
							instManLocked->addEnvelopeADPCMSequenceData(idx, data);
						if (fileVersion < Version::toBCD(1, 5, 0)) csr += 4;
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addEnvelopeADPCMLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setEnvelopeADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::AbsoluteRelease, pos));
						else instManLocked->setEnvelopeADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::RelativeRelease, pos));
						else instManLocked->setEnvelopeADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					++csr;	// Skip sequence type

					instPropCsr += ofs;
					break;
				}
				case 0x42:	// ADPCM arpeggio
				{
					int idx = *numIt++;
					auto adpcm = dynamic_cast<InstrumentADPCM*>(inst);
					adpcm->setArpeggioEnabled(true);
					adpcm->setArpeggioNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 0)) csr += 2;
						if (l == 0)
							instManLocked->setArpeggioADPCMSequenceData(idx, 0, data);
						else
							instManLocked->addArpeggioADPCMSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addArpeggioADPCMLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setArpeggioADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// Absolute
						instManLocked->setArpeggioADPCMType(idx, SequenceType::AbsoluteSequence);
						break;
					case 0x01:	// Fixed
						instManLocked->setArpeggioADPCMType(idx, SequenceType::FixedSequence);
						break;
					case 0x02:	// Relative
						instManLocked->setArpeggioADPCMType(idx, SequenceType::RelativeSequence);
						break;
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					instPropCsr += ofs;
					break;
				}
				case 0x43:	// ADPCM pitch
				{
					int idx = *numIt++;
					auto adpcm = dynamic_cast<InstrumentADPCM*>(inst);
					adpcm->setPitchEnabled(true);
					adpcm->setPitchNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (fileVersion < Version::toBCD(1, 2, 0)) csr += 2;
						if (l == 0)
							instManLocked->setPitchADPCMSequenceData(idx, 0, data);
						else
							instManLocked->addPitchADPCMSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addPitchADPCMLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPitchADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPitchADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// Absolute
						instManLocked->setPitchADPCMType(idx, SequenceType::AbsoluteSequence);
						break;
					case 0x02:	// Relative
						instManLocked->setPitchADPCMType(idx, SequenceType::RelativeSequence);
						break;
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					instPropCsr += ofs;
					break;
				}
				case 0x44:	// ADPCM pan
				{
					int idx = *numIt++;
					auto adpcm = dynamic_cast<InstrumentADPCM*>(inst);
					adpcm->setPanEnabled(true);
					adpcm->setPanNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						if (l == 0)
							instManLocked->setPanADPCMSequenceData(idx, 0, data);
						else
							instManLocked->addPanADPCMSequenceData(idx, data);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = ctr.readUint16(csr);
						csr += 2;
						int end = ctr.readUint16(csr);
						csr += 2;
						int times = ctr.readUint8(csr++);
						instManLocked->addPanADPCMLoop(idx, InstrumentSequenceLoop(begin, end, times));
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPanADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPanADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPanADPCMRelease(idx, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Inst, csr);
					}

					instPropCsr += ofs;
					break;
				}
				default:
					throw FileCorruptionError(FileType::Inst, instPropCsr);
				}
			}
		}


		return inst;
	}
}

void BtiIO::save(BinaryContainer& ctr,
				 const std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	const std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();

	ctr.appendString("BambooTrackerIst");
	size_t eofOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy EOF offset
	uint32_t fileVersion = Version::ofInstrumentFileInBCD();
	ctr.appendUint32(fileVersion);


	/***** Instrument section *****/
	ctr.appendString("INSTRMNT");
	size_t instOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument section offset

	std::vector<int> fmArpNums, fmPtNums;
	std::shared_ptr<AbstractInstrument> inst = instManLocked->getInstrumentSharedPtr(instNum);
	if (inst) {
		std::string name = inst->getName();
		ctr.appendUint32(name.length());
		if (!name.empty()) ctr.appendString(name);
		switch (inst->getType()) {
		case InstrumentType::FM:
		{
			ctr.appendUint8(0x00);
			auto instFM = std::dynamic_pointer_cast<InstrumentFM>(inst);
			uint8_t tmp = static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::All))
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op1) << 1)
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op2) << 2)
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op3) << 3)
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op4) << 4);
			ctr.appendUint8(tmp);
			if (instFM->getArpeggioEnabled(FMOperatorType::All))
				fmArpNums.push_back(instFM->getArpeggioNumber(FMOperatorType::All));
			for (auto& t : FM_OP_TYPES) {
				if (instFM->getArpeggioEnabled(t)) {
					int n = instFM->getArpeggioNumber(t);
					if (utils::find(fmArpNums, n) == fmArpNums.end())
						fmArpNums.push_back(n);
				}
			}
			if (instFM->getArpeggioEnabled(FMOperatorType::All)) {
				int n = instFM->getArpeggioNumber(FMOperatorType::All);
				for (size_t i = 0; i < fmArpNums.size(); ++i) {
					if (n == fmArpNums[i]) {
						ctr.appendUint8(static_cast<uint8_t>(i));
						break;
					}
				}
			}
			else {
				ctr.appendUint8(0x80);
			}
			for (auto& t : FM_OP_TYPES) {
				if (instFM->getArpeggioEnabled(t)) {
					int n = instFM->getArpeggioNumber(t);
					for (size_t i = 0; i < fmArpNums.size(); ++i) {
						if (n == fmArpNums[i]) {
							ctr.appendUint8(static_cast<uint8_t>(i));
							break;
						}
					}
				}
				else {
					ctr.appendUint8(0x80);
				}
			}
			if (instFM->getPitchEnabled(FMOperatorType::All))
				fmPtNums.push_back(instFM->getPitchNumber(FMOperatorType::All));
			for (auto& t : FM_OP_TYPES) {
				if (instFM->getPitchEnabled(t)) {
					int n = instFM->getPitchNumber(t);
					if (utils::find(fmPtNums, n) == fmPtNums.end())
						fmPtNums.push_back(n);
				}
			}
			if (instFM->getPitchEnabled(FMOperatorType::All)) {
				int n = instFM->getPitchNumber(FMOperatorType::All);
				for (size_t i = 0; i < fmPtNums.size(); ++i) {
					if (n == fmPtNums[i]) {
						ctr.appendUint8(static_cast<uint8_t>(i));
						break;
					}
				}
			}
			else {
				ctr.appendUint8(0x80);
			}
			for (auto& t : FM_OP_TYPES) {
				if (instFM->getPitchEnabled(t)) {
					int n = instFM->getPitchNumber(t);
					for (size_t i = 0; i < fmPtNums.size(); ++i) {
						if (n == fmPtNums[i]) {
							ctr.appendUint8(static_cast<uint8_t>(i));
							break;
						}
					}
				}
				else {
					ctr.appendUint8(0x80);
				}
			}
			break;
		}
		case InstrumentType::SSG:
		{
			ctr.appendUint8(0x01);
			break;
		}
		case InstrumentType::ADPCM:
		{
			ctr.appendUint8(0x02);
			break;
		}
		case InstrumentType::Drumkit:
		{
			ctr.appendUint8(0x03);
			auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(inst);
			std::vector<int> keys = kit->getAssignedKeys();
			ctr.appendUint8(static_cast<uint8_t>(keys.size()));
			int sampCnt = 0;
			std::unordered_map<int, int> sampMap;
			for (const int& key : keys) {
				ctr.appendUint8(static_cast<uint8_t>(key));
				int samp = kit->getSampleNumber(key);
				if (!sampMap.count(samp)) sampMap[samp] = sampCnt++;
				ctr.appendUint8(static_cast<uint8_t>(sampMap[samp]));
				ctr.appendInt8(static_cast<int8_t>(kit->getPitch(key)));
			}
			break;
		}
		}
	}
	ctr.writeUint32(instOfs, ctr.size() - instOfs);


	/***** Instrument property section *****/
	ctr.appendString("INSTPROP");
	size_t instPropOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument property section offset

	switch (inst->getType()) {
	case InstrumentType::FM:
	{
		auto instFM = std::dynamic_pointer_cast<InstrumentFM>(inst);

		// FM envelope
		int envNum = instFM->getEnvelopeNumber();
		{
			ctr.appendUint8(0x00);
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AL) << 4)
						  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::FB));
			ctr.appendUint8(tmp);
			for (int op = 0; op < 4; ++op) {
				auto& params = FM_OP_PARAMS[op];
				tmp = instManLocked->getEnvelopeFMOperatorEnabled(envNum, op);
				tmp = static_cast<uint8_t>(tmp << 5)
					  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::AR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::KS)) << 5)
					  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::DR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::DT)) << 5)
					  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::SR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::SL)) << 4)
					  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::RR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::TL)));
				ctr.appendUint8(tmp);
				int tmp2 = instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::SSGEG));
				tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
					  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::ML)));
				ctr.appendUint8(tmp);
			}
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}

		// FM LFO
		if (instFM->getLFOEnabled()) {
			int lfoNum = instFM->getLFONumber();
			ctr.appendUint8(0x01);
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::FREQ) << 4)
						  | static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::PMS));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::AM4) << 7)
				  | static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::AM3) << 6)
				  | static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::AM2) << 5)
				  | static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::AM1) << 4)
				  | static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::AMS));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getLFOFMparameter(lfoNum, FMLFOParameter::Count));
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}

		// FM envelope parameter
		for (size_t i = 0; i < 38; ++i) {
			if (instFM->getOperatorSequenceEnabled(FM_OPSEQ_PARAMS[i])) {
				int seqNum = instFM->getOperatorSequenceNumber(FM_OPSEQ_PARAMS[i]);
				ctr.appendUint8(0x02 + static_cast<uint8_t>(i));
				size_t ofs = ctr.size();
				ctr.appendUint16(0);	// Dummy offset
				auto seq = instManLocked->getOperatorSequenceFMSequence(FM_OPSEQ_PARAMS[i], seqNum);
				ctr.appendUint16(static_cast<uint16_t>(seq.size()));
				for (auto& unit : seq) {
					ctr.appendUint16(static_cast<uint16_t>(unit.data));
				}
				auto loops = instManLocked->getOperatorSequenceFMLoopRoot(FM_OPSEQ_PARAMS[i], seqNum).getAllLoops();
				ctr.appendUint16(static_cast<uint16_t>(loops.size()));
				for (auto& loop : loops) {
					ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
					ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
					ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
				}
				auto release = instManLocked->getOperatorSequenceFMRelease(FM_OPSEQ_PARAMS[i], seqNum);
				switch (release.getType()) {
				case InstrumentSequenceRelease::NoRelease:
					ctr.appendUint8(0x00);
					break;
				case InstrumentSequenceRelease::FixedRelease:
					ctr.appendUint8(0x01);
					ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
					break;
				case InstrumentSequenceRelease::AbsoluteRelease:
					ctr.appendUint8(0x02);
					ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
					break;
				case InstrumentSequenceRelease::RelativeRelease:
					ctr.appendUint8(0x03);
					ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
					break;
				}
				ctr.appendUint8(0);	// Skip sequence type
				ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
			}
		}

		// FM arpeggio
		for (int& arpNum : fmArpNums) {
			ctr.appendUint8(0x28);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getArpeggioFMSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getArpeggioFMLoopRoot(arpNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getArpeggioFMRelease(arpNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			switch (instManLocked->getArpeggioFMType(arpNum)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::FixedSequence:		ctr.appendUint8(0x01);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// FM pitch
		for (int& ptNum : fmPtNums) {
			ctr.appendUint8(0x29);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getPitchFMSequence(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getPitchFMLoopRoot(ptNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getPitchFMRelease(ptNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			switch (instManLocked->getPitchFMType(ptNum)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// FM pan
		if (instFM->getPanEnabled()) {
			int panNum = instFM->getPanNumber();
			ctr.appendUint8(0x2a);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getPanFMSequence(panNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getPanFMLoopRoot(panNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getPanFMRelease(panNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
		break;
	}
	case InstrumentType::SSG:
	{
		auto instSSG = std::dynamic_pointer_cast<InstrumentSSG>(inst);

		// SSG waveform
		if (instSSG->getWaveformEnabled()) {
			int wfNum = instSSG->getWaveformNumber();
			ctr.appendUint8(0x30);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getWaveformSSGSequence(wfNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
				ctr.appendInt32(static_cast<int32_t>(unit.subdata));
			}
			auto loops = instManLocked->getWaveformSSGLoopRoot(wfNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getWaveformSSGRelease(wfNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			ctr.appendUint8(0);	// Skip sequence type
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// SSG tone/noise
		if (instSSG->getToneNoiseEnabled()) {
			int tnNum = instSSG->getToneNoiseNumber();
			ctr.appendUint8(0x31);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getToneNoiseSSGSequence(tnNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getToneNoiseSSGLoopRoot(tnNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getToneNoiseSSGRelease(tnNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			ctr.appendUint8(0);	// Skip sequence type
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// SSG envelope
		if (instSSG->getEnvelopeEnabled()) {
			int envNum = instSSG->getEnvelopeNumber();
			ctr.appendUint8(0x32);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getEnvelopeSSGSequence(envNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
				ctr.appendInt32(static_cast<int32_t>(unit.subdata));
			}
			auto loops = instManLocked->getEnvelopeSSGLoopRoot(envNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getEnvelopeSSGRelease(envNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			ctr.appendUint8(0);	// Skip sequence type
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// SSG arpeggio
		if (instSSG->getArpeggioEnabled()) {
			int arpNum = instSSG->getArpeggioNumber();
			ctr.appendUint8(0x33);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getArpeggioSSGSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getArpeggioSSGLoopRoot(arpNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getArpeggioSSGRelease(arpNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			switch (instManLocked->getArpeggioSSGType(arpNum)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::FixedSequence:		ctr.appendUint8(0x01);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// SSG pitch
		if (instSSG->getPitchEnabled()) {
			int ptNum = instSSG->getPitchNumber();
			ctr.appendUint8(0x34);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getPitchSSGSequence(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getPitchSSGLoopRoot(ptNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getPitchSSGRelease(ptNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			switch (instManLocked->getPitchSSGType(ptNum)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
		break;
	}
	case InstrumentType::ADPCM:
	{
		auto instADPCM = std::dynamic_pointer_cast<InstrumentADPCM>(inst);

		// ADPCM sample
		int sampNum = instADPCM->getSampleNumber();
		{
			ctr.appendUint8(0x40);
			size_t ofs = ctr.size();
			ctr.appendUint32(0);	// Dummy offset
			ctr.appendUint8(static_cast<uint8_t>(instManLocked->getSampleADPCMRootKeyNumber(sampNum)));
			ctr.appendUint16(static_cast<uint16_t>(instManLocked->getSampleADPCMRootDeltaN(sampNum)));
			ctr.appendUint8(static_cast<uint8_t>(instManLocked->isSampleADPCMRepeatable(sampNum)));
			std::vector<uint8_t> samples = instManLocked->getSampleADPCMRawSample(sampNum);
			ctr.appendUint32(samples.size());
			ctr.appendVector(std::move(samples));
			ctr.writeUint32(ofs, ctr.size() - ofs);
		}

		// ADPCM envelope
		if (instADPCM->getEnvelopeEnabled()) {
			int envNum = instADPCM->getEnvelopeNumber();
			ctr.appendUint8(0x41);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getEnvelopeADPCMSequence(envNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getEnvelopeADPCMLoopRoot(envNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getEnvelopeADPCMRelease(envNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			ctr.appendUint8(0);	// Skip sequence type
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// ADPCM arpeggio
		if (instADPCM->getArpeggioEnabled()) {
			int arpNum = instADPCM->getArpeggioNumber();
			ctr.appendUint8(0x42);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getArpeggioADPCMSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getArpeggioADPCMLoopRoot(arpNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getArpeggioADPCMRelease(arpNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			switch (instManLocked->getArpeggioADPCMType(arpNum)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::FixedSequence:		ctr.appendUint8(0x01);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// ADPCM pitch
		if (instADPCM->getPitchEnabled()) {
			int ptNum = instADPCM->getPitchNumber();
			ctr.appendUint8(0x43);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getPitchADPCMSequence(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getPitchADPCMLoopRoot(ptNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getPitchADPCMRelease(ptNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			switch (instManLocked->getPitchADPCMType(ptNum)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// ADPCM pan
		if (instADPCM->getPanEnabled()) {
			int panNum = instADPCM->getPanNumber();
			ctr.appendUint8(0x44);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getPanADPCMSequence(panNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instManLocked->getPanADPCMLoopRoot(panNum).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instManLocked->getPanADPCMRelease(panNum);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case InstrumentSequenceRelease::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			case InstrumentSequenceRelease::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.getBeginPos()));
				break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(inst);

		// ADPCM sample
		{
			std::vector<int> sampList;
			for (const int& key : kit->getAssignedKeys()) {
				int samp = kit->getSampleNumber(key);
				if (std::none_of(sampList.begin(), sampList.end(), [samp](const int& v) { return v == samp; })) {
					sampList.push_back(samp);

					ctr.appendUint8(0x40);
					size_t ofs = ctr.size();
					ctr.appendUint32(0);	// Dummy offset
					ctr.appendUint8(static_cast<uint8_t>(instManLocked->getSampleADPCMRootKeyNumber(samp)));
					ctr.appendUint16(static_cast<uint16_t>(instManLocked->getSampleADPCMRootDeltaN(samp)));
					ctr.appendUint8(static_cast<uint8_t>(instManLocked->isSampleADPCMRepeatable(samp)));
					std::vector<uint8_t> samples = instManLocked->getSampleADPCMRawSample(samp);
					ctr.appendUint32(samples.size());
					ctr.appendVector(std::move(samples));
					ctr.writeUint32(ofs, ctr.size() - ofs);
				}
			}
		}
		break;
	}
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);

	ctr.writeUint32(eofOfs, ctr.size() - eofOfs);
}
}
