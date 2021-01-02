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

#include "dmp_io.hpp"
#include "file_io_error.hpp"
#include "io_utils.hpp"

namespace io
{
DmpIO::DmpIO() : AbstractInstrumentIO("dmp", "DefleMask preset", true, false) {}

AbstractInstrument* DmpIO::load(const BinaryContainer& ctr, const std::string& fileName,
								std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	size_t csr = 0;

	uint8_t insType = 1; // default to FM
	uint8_t fileVersion = ctr.readUint8(csr++);
	if (fileVersion == 0) { // older, unversioned dmp
		if (ctr.size() != 49) throw FileCorruptionError(FileType::Inst, csr);
	}
	else {
		if (fileVersion < 9) throw FileCorruptionError(FileType::Inst, csr);
		if (fileVersion == 9 && ctr.size() != 51) { // make sure it's not for that discontinued chip
			throw FileCorruptionError(FileType::Inst, csr);
		}
		uint8_t system = 2; // default to genesis
		if (fileVersion >= 11) system = ctr.readUint8(csr++);
		if (system != 2 && system != 3 && system != 8) { // genesis, sms and arcade only
			throw FileCorruptionError(FileType::Inst, csr);
		}
		insType = ctr.readUint8(csr++);
	}
	AbstractInstrument* inst = nullptr;
	switch (insType) {
	case 0x00:	// SSG
	{
		inst = new InstrumentSSG(instNum, fileName, instManLocked.get());
		auto ssg = dynamic_cast<InstrumentSSG*>(inst);
		uint8_t envSize = ctr.readUint8(csr++);
		if (envSize > 0) {
			int idx = instManLocked->findFirstAssignableEnvelopeSSG();
			if (idx < 0) throw FileCorruptionError(FileType::Inst, csr);
			ssg->setEnvelopeEnabled(true);
			ssg->setEnvelopeNumber(idx);
			for (uint8_t l = 0; l < envSize; ++l) {
				int data = ctr.readInt32(csr);
				// compensate SN76489's envelope step of 2dB to SSG's 3dB
				if (data > 0) data = 15 - (15 - data) * 2 / 3;
				csr += 4;
				if (l == 0) instManLocked->setEnvelopeSSGSequenceCommand(idx, 0, data, 0);
				else instManLocked->addEnvelopeSSGSequenceCommand(idx, data, 0);
			}
			int8_t loop = ctr.readInt8(csr++);
			if (loop >= 0) instManLocked->setEnvelopeSSGLoops(idx, {loop}, {envSize - 1}, {1});
		}
		uint8_t arpSize = ctr.readUint8(csr++);
		if (arpSize > 0) {
			int idx = instManLocked->findFirstAssignableArpeggioSSG();
			if (idx < 0) throw FileCorruptionError(FileType::Inst, csr);
			ssg->setArpeggioEnabled(true);
			ssg->setArpeggioNumber(idx);
			uint8_t arpType = ctr.readUint8(csr + arpSize * 4 + 1);
			if (arpType == 1) instManLocked->setArpeggioSSGType(idx, SequenceType::FixedSequence);
			for (uint8_t l = 0; l < arpSize; ++l) {
				int data = ctr.readInt32(csr) + 36;
				csr += 4;
				if (arpType == 1) data -= 24;
				if (l == 0) instManLocked->setArpeggioSSGSequenceData(idx, 0, data);
				else instManLocked->addArpeggioSSGSequenceData(idx, data);
			}
			int8_t loop = ctr.readInt8(csr++);
			if (loop >= 0) instManLocked->addArpeggioSSGLoop(idx, InstrumentSequenceLoop(loop, arpSize - 1));
		}
		break;
	}
	case 0x01:	// FM
	{
		int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
		if (envIdx < 0) throw FileCorruptionError(FileType::Inst, csr);
		inst = new InstrumentFM(instNum, fileName, instManLocked.get());
		auto fm = dynamic_cast<InstrumentFM*>(inst);
		fm->setEnvelopeNumber(envIdx);
		if (fileVersion == 9) csr++; // skip version 9's total operators field
		uint8_t pms = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
		uint8_t ams = ctr.readUint8(csr++);

		uint8_t am[4];
		for (const int op : { 0, 2, 1, 3 }) {
			auto& params = FM_OP_PARAMS[op];
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::ML), ctr.readUint8(csr++));
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::TL), ctr.readUint8(csr++));
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::AR), ctr.readUint8(csr++));
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DR), ctr.readUint8(csr++));
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SL), ctr.readUint8(csr++));
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::RR), ctr.readUint8(csr++));
			am[op] = ctr.readUint8(csr++);
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::KS), ctr.readUint8(csr++));
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DT),
												  convertDtFromDmpTfiVgi(ctr.readUint8(csr++) & 15)); // mask out OPM's DT2
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SR), ctr.readUint8(csr++));
			int ssgeg1 = ctr.readUint8(csr++);
			ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
			instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SSGEG), ssgeg1);
		}

		if (pms || ams) {
			int lfoIdx = instManLocked->findFirstAssignableLFOFM();
			if (lfoIdx < 0) throw FileCorruptionError(FileType::Inst, csr);
			fm->setLFOEnabled(true);
			fm->setLFONumber(lfoIdx);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, pms);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, ams);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, am[0]);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, am[1]);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, am[2]);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, am[3]);
		}
		break;
	}
	}
	return inst;
}
}
