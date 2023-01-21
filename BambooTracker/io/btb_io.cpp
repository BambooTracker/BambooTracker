/*
 * Copyright (C) 2020-2023 Rerrah
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

#include "btb_io.hpp"
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include "enum_hash.hpp"
#include "version.hpp"
#include "instrument.hpp"
#include "file_io_error.hpp"
#include "io_utils.hpp"

namespace io
{
namespace
{
bool judgeRequiredProperty(const std::multiset<int>& usedInstIdcs, const std::vector<int>& reqInstIdcs)
{
	std::vector<int> intrsct;
	std::set_intersection(usedInstIdcs.begin(), usedInstIdcs.end(), reqInstIdcs.begin(), reqInstIdcs.end(), std::back_inserter(intrsct));
	return !intrsct.empty();
}
}

BtbIO::BtbIO() : AbstractBankIO("btb", "BambooTracker bank", true, true) {}

AbstractBank* BtbIO::load(const BinaryContainer& ctr) const
{
	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerBnk")
		throw FileCorruptionError(FileType::Bank, globCsr);
	globCsr += 16;
	/*size_t eofOfs = */ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofBankFileInBCD())
		throw FileVersionError(FileType::Bank);
	globCsr += 4;


	/***** Instrument section *****/
	std::vector<int> ids;
	std::vector<std::string> names;
	std::vector<BinaryContainer> instCtrs;
	if (ctr.readString(globCsr, 8) != "INSTRMNT")
		throw FileCorruptionError(FileType::Bank, globCsr);
	globCsr += 8;
	size_t instOfs = ctr.readUint32(globCsr);
	size_t instCsr = globCsr + 4;
	uint8_t instCnt = ctr.readUint8(instCsr);
	instCsr += 1;
	for (uint8_t i = 0; i < instCnt; ++i) {
		size_t pos = instCsr;
		uint8_t idx = ctr.readUint8(instCsr);
		ids.push_back(idx);
		instCsr += 1;
		size_t iOfs = ctr.readUint32(instCsr);
		size_t iCsr = instCsr + 4;
		size_t nameLen = ctr.readUint32(iCsr);
		iCsr += 4;
		std::string name = u8"";
		if (nameLen > 0) {
			name = ctr.readString(iCsr, nameLen);
			/* iCsr += nameLen; */
		}
		names.push_back(name);
		instCsr += iOfs;	// Jump to next
		instCtrs.push_back(ctr.getSubcontainer(pos, 1 + iOfs));
	}
	globCsr += instOfs;


	/***** Instrument property section *****/
	if (ctr.readString(globCsr, 8) != "INSTPROP")
		throw FileCorruptionError(FileType::Inst, globCsr);
	globCsr += 8;
	size_t instPropOfs = ctr.readUint32(globCsr);
	BinaryContainer propCtr = ctr.getSubcontainer(globCsr + 4, instPropOfs - 4);

	return new BtBank(ids, names, instCtrs, propCtr, fileVersion);
}

void BtbIO::save(BinaryContainer& ctr, const std::weak_ptr<InstrumentsManager> instMan,
				 const std::vector<int>& instNums) const
{
	ctr.appendString("BambooTrackerBnk");
	size_t eofOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy EOF offset
	uint32_t fileVersion = Version::ofBankFileInBCD();
	ctr.appendUint32(fileVersion);


	/***** Instrument section *****/
	ctr.appendString("INSTRMNT");
	size_t instOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument section offset
	ctr.appendUint8(static_cast<uint8_t>(instNums.size()));
	for (auto& idx : instNums) {
		if (std::shared_ptr<AbstractInstrument> inst = instMan.lock()->getInstrumentSharedPtr(static_cast<int>(idx))) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t iOfs = ctr.size();
			ctr.appendUint32(0);	// Dummy instrument block offset
			std::string name = inst->getName();
			ctr.appendUint32(name.length());
			if (!name.empty()) ctr.appendString(name);
			switch (inst->getType()) {
			case InstrumentType::FM:
			{
				ctr.appendUint8(0x00);
				auto instFM = std::dynamic_pointer_cast<InstrumentFM>(inst);
				ctr.appendUint8(static_cast<uint8_t>(instFM->getEnvelopeNumber()));
				uint8_t tmp = static_cast<uint8_t>(instFM->getLFONumber());
				ctr.appendUint8(instFM->getLFOEnabled() ? tmp : (0x80 | tmp));
				for (auto& param : FM_OPSEQ_PARAMS) {
					tmp = static_cast<uint8_t>(instFM->getOperatorSequenceNumber(param));
					ctr.appendUint8(instFM->getOperatorSequenceEnabled(param) ? tmp : (0x80 | tmp));
				}
				tmp = static_cast<uint8_t>(instFM->getArpeggioNumber(FMOperatorType::All));
				ctr.appendUint8(instFM->getArpeggioEnabled(FMOperatorType::All) ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instFM->getPitchNumber(FMOperatorType::All));
				ctr.appendUint8(instFM->getPitchEnabled(FMOperatorType::All) ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::All))
					  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op1) << 1)
					  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op2) << 2)
					  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op3) << 3)
					  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op4) << 4);
				ctr.appendUint8(tmp);
				for (auto& type : FM_OP_TYPES) {
					tmp = static_cast<uint8_t>(instFM->getArpeggioNumber(type));
					ctr.appendUint8(instFM->getArpeggioEnabled(type) ? tmp : (0x80 | tmp));
				}
				for (auto& type : FM_OP_TYPES) {
					tmp = static_cast<uint8_t>(instFM->getPitchNumber(type));
					ctr.appendUint8(instFM->getPitchEnabled(type) ? tmp : (0x80 | tmp));
				}
				tmp = static_cast<uint8_t>(instFM->getPanNumber());
				ctr.appendUint8(instFM->getPanEnabled() ? tmp : (0x80 | tmp));
				break;
			}
			case InstrumentType::SSG:
			{
				ctr.appendUint8(0x01);
				auto instSSG = std::dynamic_pointer_cast<InstrumentSSG>(inst);
				uint8_t tmp = static_cast<uint8_t>(instSSG->getWaveformNumber());
				ctr.appendUint8(instSSG->getWaveformEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instSSG->getToneNoiseNumber());
				ctr.appendUint8(instSSG->getToneNoiseEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instSSG->getEnvelopeNumber());
				ctr.appendUint8(instSSG->getEnvelopeEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instSSG->getArpeggioNumber());
				ctr.appendUint8(instSSG->getArpeggioEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instSSG->getPitchNumber());
				ctr.appendUint8(instSSG->getPitchEnabled() ? tmp : (0x80 | tmp));
				break;
			}
			case InstrumentType::ADPCM:
			{
				ctr.appendUint8(0x02);
				auto instADPCM = std::dynamic_pointer_cast<InstrumentADPCM>(inst);
				ctr.appendUint8(static_cast<uint8_t>(instADPCM->getSampleNumber()));
				uint8_t tmp = static_cast<uint8_t>(instADPCM->getEnvelopeNumber());
				ctr.appendUint8(instADPCM->getEnvelopeEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instADPCM->getArpeggioNumber());
				ctr.appendUint8(instADPCM->getArpeggioEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instADPCM->getPitchNumber());
				ctr.appendUint8(instADPCM->getPitchEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instADPCM->getPanNumber());
				ctr.appendUint8(instADPCM->getPanEnabled() ? tmp : (0x80 | tmp));
				break;
			}
			case InstrumentType::Drumkit:
			{
				ctr.appendUint8(0x03);
				auto instKit = std::dynamic_pointer_cast<InstrumentDrumkit>(inst);
				std::vector<int> keys = instKit->getAssignedKeys();
				ctr.appendUint8(static_cast<uint8_t>(keys.size()));
				for (const int& key : keys) {
					ctr.appendUint8(static_cast<uint8_t>(key));
					ctr.appendUint8(static_cast<uint8_t>(instKit->getSampleNumber(key)));
					ctr.appendInt8(static_cast<int8_t>(instKit->getPitch(key)));
					ctr.appendUint8(static_cast<uint8_t>(instKit->getPan(key)));
				}
				break;
			}
			}
			ctr.writeUint32(iOfs, ctr.size() - iOfs);
		}
	}
	ctr.writeUint32(instOfs, ctr.size() - instOfs);


	/***** Instrument property section *****/
	ctr.appendString("INSTPROP");
	size_t instPropOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument property section offset

	// FM envelope
	std::vector<int> envFMIdcs;
	for (auto& idx : instMan.lock()->getEnvelopeFMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getEnvelopeFMUsers(idx), instNums))
			envFMIdcs.push_back(idx);
	}
	if (!envFMIdcs.empty()) {
		ctr.appendUint8(0x00);
		ctr.appendUint8(static_cast<uint8_t>(envFMIdcs.size()));
		for (auto& idx : envFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AL) << 4)
						  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::FB));
			ctr.appendUint8(tmp);
			for (int op = 0; op < 4; ++op) {	// Operator
				auto& params = FM_OP_PARAMS[op];
				tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, op);
				tmp = static_cast<uint8_t>((tmp << 5))
					  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::AR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::KS)) << 5)
					  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::DR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::DT)) << 5)
					  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::SR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::SL)) << 4)
					  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::RR)));
				ctr.appendUint8(tmp);
				tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::TL)));
				ctr.appendUint8(tmp);
				int tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::SSGEG));
				tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
					  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, params.at(FMOperatorParameter::ML)));
				ctr.appendUint8(tmp);
			}
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}
	}

	// FM LFO
	std::vector<int> lfoFMIdcs;
	for (auto& idx : instMan.lock()->getLFOFMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getLFOFMUsers(idx), instNums))
			lfoFMIdcs.push_back(idx);
	}
	if (!lfoFMIdcs.empty()) {
		ctr.appendUint8(0x01);
		ctr.appendUint8(static_cast<uint8_t>(lfoFMIdcs.size()));
		for (auto& idx : lfoFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::FREQ) << 4)
						  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::PMS));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM4) << 7)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM3) << 6)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM2) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM1) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AMS));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::Count));
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}
	}

	// FM envelope parameter
	for (size_t i = 0; i < 38; ++i) {
		std::vector<int> idcs;
		for (auto& idx : instMan.lock()->getOperatorSequenceFMEntriedIndices(FM_OPSEQ_PARAMS[i])) {
			if (judgeRequiredProperty(instMan.lock()->getOperatorSequenceFMUsers(FM_OPSEQ_PARAMS[i], idx), instNums))
				idcs.push_back(idx);
		}
		if (!idcs.empty()) {
			ctr.appendUint8(0x02 + static_cast<uint8_t>(i));
			ctr.appendUint8(static_cast<uint8_t>(idcs.size()));
			for (auto& idx : idcs) {
				ctr.appendUint8(static_cast<uint8_t>(idx));
				size_t ofs = ctr.size();
				ctr.appendUint16(0);	// Dummy offset
				auto seq = instMan.lock()->getOperatorSequenceFMSequence(FM_OPSEQ_PARAMS[i], idx);
				ctr.appendUint16(static_cast<uint16_t>(seq.size()));
				for (auto& unit : seq) {
					ctr.appendUint16(static_cast<uint16_t>(unit.data));
				}
				auto loops = instMan.lock()->getOperatorSequenceFMLoopRoot(FM_OPSEQ_PARAMS[i], idx).getAllLoops();
				ctr.appendUint16(static_cast<uint16_t>(loops.size()));
				for (auto& loop : loops) {
					ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
					ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
					ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
				}
				auto release = instMan.lock()->getOperatorSequenceFMRelease(FM_OPSEQ_PARAMS[i], idx);
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
	}

	// FM arpeggio
	std::vector<int> arpFMIdcs;
	for (auto& idx : instMan.lock()->getArpeggioFMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getArpeggioFMUsers(idx), instNums))
			arpFMIdcs.push_back(idx);
	}
	if (!arpFMIdcs.empty()) {
		ctr.appendUint8(0x28);
		ctr.appendUint8(static_cast<uint8_t>(arpFMIdcs.size()));
		for (auto& idx : arpFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioFMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getArpeggioFMLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getArpeggioFMRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getArpeggioFMType(idx)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::FixedSequence:		ctr.appendUint8(0x01);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// FM pitch
	std::vector<int> ptFMIdcs;
	for (auto& idx : instMan.lock()->getPitchFMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getPitchFMUsers(idx), instNums))
			ptFMIdcs.push_back(idx);
	}
	if (!ptFMIdcs.empty()) {
		ctr.appendUint8(0x29);
		ctr.appendUint8(static_cast<uint8_t>(ptFMIdcs.size()));
		for (auto& idx : ptFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchFMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getPitchFMLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getPitchFMRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getPitchFMType(idx)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// FM pan
	std::vector<int> panFMIdcs;
	for (auto& idx : instMan.lock()->getPanFMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getPanFMUsers(idx), instNums))
			panFMIdcs.push_back(idx);
	}
	if (!panFMIdcs.empty()) {
		ctr.appendUint8(0x2a);
		ctr.appendUint8(static_cast<uint8_t>(panFMIdcs.size()));
		for (auto& idx : panFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPanFMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getPanFMLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getPanFMRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
	}

	// SSG waveform
	std::vector<int> wfSSGIdcs;
	for (auto& idx : instMan.lock()->getWaveformSSGEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getWaveformSSGUsers(idx), instNums))
			wfSSGIdcs.push_back(idx);
	}
	if (!wfSSGIdcs.empty()) {
		ctr.appendUint8(0x30);
		ctr.appendUint8(static_cast<uint8_t>(wfSSGIdcs.size()));
		for (auto& idx : wfSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getWaveformSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
				ctr.appendInt32(static_cast<int32_t>(unit.subdata));
			}
			auto loops = instMan.lock()->getWaveformSSGLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getWaveformSSGRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// SSG tone/noise
	std::vector<int> tnSSGIdcs;
	for (auto& idx : instMan.lock()->getToneNoiseSSGEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getToneNoiseSSGUsers(idx), instNums))
			tnSSGIdcs.push_back(idx);
	}
	if (!tnSSGIdcs.empty()) {
		ctr.appendUint8(0x31);
		ctr.appendUint8(static_cast<uint8_t>(tnSSGIdcs.size()));
		for (auto& idx : tnSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getToneNoiseSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getToneNoiseSSGLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getToneNoiseSSGRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// SSG envelope
	std::vector<int> envSSGIdcs;
	for (auto& idx : instMan.lock()->getEnvelopeSSGEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getEnvelopeSSGUsers(idx), instNums))
			envSSGIdcs.push_back(idx);
	}
	if (!envSSGIdcs.empty()) {
		ctr.appendUint8(0x32);
		ctr.appendUint8(static_cast<uint8_t>(envSSGIdcs.size()));
		for (auto& idx : envSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getEnvelopeSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
				ctr.appendInt32(static_cast<int32_t>(unit.subdata));
			}
			auto loops = instMan.lock()->getEnvelopeSSGLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getEnvelopeSSGRelease(idx);

			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// SSG arpeggio
	std::vector<int> arpSSGIdcs;
	for (auto& idx : instMan.lock()->getArpeggioSSGEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getArpeggioSSGUsers(idx), instNums))
			arpSSGIdcs.push_back(idx);
	}
	if (!arpSSGIdcs.empty()) {
		ctr.appendUint8(0x33);
		ctr.appendUint8(static_cast<uint8_t>(arpSSGIdcs.size()));
		for (auto& idx : arpSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getArpeggioSSGLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getArpeggioSSGRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getArpeggioSSGType(idx)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::FixedSequence:		ctr.appendUint8(0x01);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG pitch
	std::vector<int> ptSSGIdcs;
	for (auto& idx : instMan.lock()->getPitchSSGEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getPitchSSGUsers(idx), instNums))
			ptSSGIdcs.push_back(idx);
	}
	if (!ptSSGIdcs.empty()) {
		ctr.appendUint8(0x34);
		ctr.appendUint8(static_cast<uint8_t>(ptSSGIdcs.size()));
		for (auto& idx : ptSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getPitchSSGLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getPitchSSGRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getPitchSSGType(idx)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// ADPCM sample
	std::vector<int> sampADPCMIdcs;
	for (auto& idx : instMan.lock()->getSampleADPCMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getSampleADPCMUsers(idx), instNums))
			sampADPCMIdcs.push_back(idx);
	}
	if (!sampADPCMIdcs.empty()) {
		ctr.appendUint8(0x40);
		ctr.appendUint8(static_cast<uint8_t>(sampADPCMIdcs.size()));
		for (auto& idx : sampADPCMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint32(0);	// Dummy offset
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->getSampleADPCMRootKeyNumber(idx)));
			ctr.appendUint16(static_cast<uint16_t>(instMan.lock()->getSampleADPCMRootDeltaN(idx)));
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->isSampleADPCMRepeatable(idx)));
			std::vector<uint8_t> samples = instMan.lock()->getSampleADPCMRawSample(idx);
			ctr.appendUint32(samples.size());
			ctr.appendVector(samples);
			SampleRepeatRange range = instMan.lock()->getSampleADPCMRepeatRange(idx);
			ctr.appendUint16(range.first());
			ctr.appendUint16(range.last());
			ctr.writeUint32(ofs, ctr.size() - ofs);
		}
	}

	// ADPCM envelope
	std::vector<int> envADPCMIdcs;
	for (auto& idx : instMan.lock()->getEnvelopeADPCMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getEnvelopeADPCMUsers(idx), instNums))
			envADPCMIdcs.push_back(idx);
	}
	if (!envADPCMIdcs.empty()) {
		ctr.appendUint8(0x41);
		ctr.appendUint8(static_cast<uint8_t>(envADPCMIdcs.size()));
		for (auto& idx : envADPCMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getEnvelopeADPCMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getEnvelopeADPCMLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getEnvelopeADPCMRelease(idx);

			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// ADPCM arpeggio
	std::vector<int> arpADPCMIdcs;
	for (auto& idx : instMan.lock()->getArpeggioADPCMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getArpeggioADPCMUsers(idx), instNums))
			arpADPCMIdcs.push_back(idx);
	}
	if (!arpADPCMIdcs.empty()) {
		ctr.appendUint8(0x42);
		ctr.appendUint8(static_cast<uint8_t>(arpADPCMIdcs.size()));
		for (auto& idx : arpADPCMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioADPCMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getArpeggioADPCMLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getArpeggioADPCMRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getArpeggioADPCMType(idx)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::FixedSequence:		ctr.appendUint8(0x01);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// ADPCM pitch
	std::vector<int> ptADPCMIdcs;
	for (auto& idx : instMan.lock()->getPitchADPCMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getPitchADPCMUsers(idx), instNums))
			ptADPCMIdcs.push_back(idx);
	}
	if (!ptADPCMIdcs.empty()) {
		ctr.appendUint8(0x43);
		ctr.appendUint8(static_cast<uint8_t>(ptADPCMIdcs.size()));
		for (auto& idx : ptADPCMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchADPCMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getPitchADPCMLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getPitchADPCMRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getPitchADPCMType(idx)) {
			case SequenceType::AbsoluteSequence:	ctr.appendUint8(0x00);	break;
			case SequenceType::RelativeSequence:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// ADPCM pan
	std::vector<int> panADPCMIdcs;
	for (auto& idx : instMan.lock()->getPanADPCMEntriedIndices()) {
		if (judgeRequiredProperty(instMan.lock()->getPanADPCMUsers(idx), instNums))
			panADPCMIdcs.push_back(idx);
	}
	if (!panADPCMIdcs.empty()) {
		ctr.appendUint8(0x34);
		ctr.appendUint8(static_cast<uint8_t>(panADPCMIdcs.size()));
		for (auto& idx : panADPCMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPanADPCMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& unit : seq) {
				ctr.appendUint16(static_cast<uint16_t>(unit.data));
			}
			auto loops = instMan.lock()->getPanADPCMLoopRoot(idx).getAllLoops();
			ctr.appendUint16(static_cast<uint16_t>(loops.size()));
			for (auto& loop : loops) {
				ctr.appendUint16(static_cast<uint16_t>(loop.getBeginPos()));
				ctr.appendUint16(static_cast<uint16_t>(loop.getEndPos()));
				ctr.appendUint8(static_cast<uint8_t>(loop.getTimes()));
			}
			auto release = instMan.lock()->getPanADPCMRelease(idx);
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);

	ctr.writeUint32(eofOfs, ctr.size() - eofOfs);
}

namespace
{
size_t getPropertyPosition(const BinaryContainer& propCtr, uint8_t subsecType, uint8_t index)
{
	size_t csr = 0;

	while (csr < propCtr.size()) {
		uint8_t type = propCtr.readUint8(csr++);
		bool isSection = (type == subsecType);
		size_t bcnt = propCtr.readUint8(csr++);
		for (size_t i = 0; i < bcnt; ++i) {
			if (isSection) {
				if (propCtr.readUint8(csr++) == index) {
					switch (type) {
					case 0x00:	// FM envelope
					case 0x01:	// FM LFO
						csr += 1;
						break;
					case 0x40:	// ADPCM sample
						csr += 4;
						break;
					default:	// Sequence
						csr += 2;
						break;
					}
					return csr;
				}
				else {
					switch (type) {
					case 0x00:	// FM envelope
					case 0x01:	// FM LFO
						csr += propCtr.readUint8(csr);
						break;
					case 0x40:	// ADPCM sample
						csr += propCtr.readUint32(csr);
						break;
					default:	// Sequence
						csr += propCtr.readUint16(csr);
						break;
					}
				}
			}
			else {
				++csr;	// Skip index
				switch (type) {
				case 0x00:	// FM envelope
				case 0x01:	// FM LFO
					csr += propCtr.readUint8(csr);
					break;
				case 0x40:	// ADPCM sample
					csr += propCtr.readUint32(csr);
					break;
				default:	// Sequence
					csr += propCtr.readUint16(csr);
					break;
				}
			}
		}
	}

	return std::numeric_limits<size_t>::max();
}
}

AbstractInstrument* BtbIO::loadInstrument(const BinaryContainer& instCtr,
										  const BinaryContainer& propCtr,
										  std::weak_ptr<InstrumentsManager> instMan,
										  int instNum, uint32_t bankVersion)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	size_t instCsr = 5;	// Skip instrument id and offset
	size_t nameLen = instCtr.readUint32(instCsr);
	instCsr += 4;
	std::string name = u8"";
	if (nameLen > 0) {
		name = instCtr.readString(instCsr, nameLen);
		instCsr += nameLen;
	}

	switch (instCtr.readUint8(instCsr++)) {
	case 0x00:	// FM
	{
		auto fm = new InstrumentFM(instNum, name, instManLocked.get());

		/* Envelope */
		{
			auto orgEnvNum = instCtr.readUint8(instCsr++);
			int envNum = instManLocked->findFirstAssignableEnvelopeFM();
			if (envNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
			fm->setEnvelopeNumber(envNum);
			size_t envCsr = getPropertyPosition(propCtr, 0x00, orgEnvNum);
			if (envCsr != std::numeric_limits<size_t>::max()) {
				uint8_t tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AL, tmp >> 4);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::FB, tmp & 0x0f);
				for (int op = 0; op < 4; ++op) {
					auto& params = FM_OP_PARAMS[op];
					tmp = propCtr.readUint8(envCsr++);
					instManLocked->setEnvelopeFMOperatorEnabled(envNum, op, (0x20 & tmp) ? true : false);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::AR), tmp & 0x1f);
					tmp = propCtr.readUint8(envCsr++);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::KS), tmp >> 5);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::DR), tmp & 0x1f);
					tmp = propCtr.readUint8(envCsr++);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::DT), tmp >> 5);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::SR), tmp & 0x1f);
					tmp = propCtr.readUint8(envCsr++);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::SL), tmp >> 4);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::RR), tmp & 0x0f);
					tmp = propCtr.readUint8(envCsr++);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::TL), tmp);
					tmp = propCtr.readUint8(envCsr++);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::ML), tmp & 0x0f);
					instManLocked->setEnvelopeFMParameter(envNum, params.at(FMOperatorParameter::SSGEG),
														  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				}
			}
		}

		/* LFO */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				fm->setLFOEnabled(false);
				fm->setLFONumber(0x7f & tmp);
			}
			else {
				fm->setLFOEnabled(true);
				uint8_t orgLFONum = 0x7f & tmp;
				int lfoNum = instManLocked->findFirstAssignableLFOFM();
				if (lfoNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				fm->setLFONumber(lfoNum);
				size_t lfoCsr = getPropertyPosition(propCtr, 0x01, orgLFONum);
				if (lfoCsr != std::numeric_limits<size_t>::max()) {
					tmp = propCtr.readUint8(lfoCsr++);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::FREQ, tmp >> 4);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::PMS, tmp & 0x0f);
					tmp = propCtr.readUint8(lfoCsr++);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::AMS, tmp & 0x0f);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::AM1, (tmp & 0x10) ? true : false);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::AM2, (tmp & 0x20) ? true : false);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::AM3, (tmp & 0x40) ? true : false);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::AM4, (tmp & 0x80) ? true : false);
					tmp = propCtr.readUint8(lfoCsr++);
					instManLocked->setLFOFMParameter(lfoNum, FMLFOParameter::Count, tmp);
				}
			}
		}

		/* Operator sequence */
		uint8_t tmpCnt = 0;
		for (auto& param : FM_OPSEQ_PARAMS) {
			++tmpCnt;
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				fm->setOperatorSequenceEnabled(param, false);
				fm->setOperatorSequenceNumber(param, 0x7f & tmp);
			}
			else {
				fm->setOperatorSequenceEnabled(param, true);
				uint8_t orgOpSeqNum = 0x7f & tmp;
				int opSeqNum = instManLocked->findFirstAssignableOperatorSequenceFM(param);
				if (opSeqNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				fm->setOperatorSequenceNumber(param, opSeqNum);
				size_t opSeqCsr = getPropertyPosition(propCtr, 0x02 + tmpCnt, orgOpSeqNum);

				if (opSeqCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(opSeqCsr);
					opSeqCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(opSeqCsr);
						opSeqCsr += 2;
						if (l == 0)
							instManLocked->setOperatorSequenceFMSequenceData(param, opSeqNum, 0, data);
						else
							instManLocked->addOperatorSequenceFMSequenceData(param, opSeqNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(opSeqCsr);
					opSeqCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(opSeqCsr);
						opSeqCsr += 2;
						int end = propCtr.readUint16(opSeqCsr);
						opSeqCsr += 2;
						int times = propCtr.readUint8(opSeqCsr++);
						instManLocked->addOperatorSequenceFMLoop(param, opSeqNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(opSeqCsr++)) {
					case 0x00:	// No release
						instManLocked->setOperatorSequenceFMRelease(param, opSeqNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(opSeqCsr);
						opSeqCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setOperatorSequenceFMRelease(param, opSeqNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setOperatorSequenceFMRelease(param, opSeqNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, instCsr);
					}
				}
			}
		}

		/* Arpeggio */
		{
			std::unordered_map<FMOperatorType, int> tmpMap;
			std::unordered_map<int, FMOperatorType> orgNumMap;
			tmpMap.emplace(FMOperatorType::All, instCtr.readUint8(instCsr));
			instCsr += 3;
			tmpMap.emplace(FMOperatorType::Op1, instCtr.readUint8(instCsr++));
			tmpMap.emplace(FMOperatorType::Op2, instCtr.readUint8(instCsr++));
			tmpMap.emplace(FMOperatorType::Op3, instCtr.readUint8(instCsr++));
			tmpMap.emplace(FMOperatorType::Op4, instCtr.readUint8(instCsr));
			instCsr -= 5;
			for (auto& pair : tmpMap) {
				if (0x80 & pair.second) {
					fm->setArpeggioEnabled(pair.first, false);
					fm->setArpeggioNumber(pair.first, 0x7f & pair.second);
				}
				else {
					fm->setArpeggioEnabled(pair.first, true);
					uint8_t orgArpNum = 0x7f & pair.second;
					auto it = orgNumMap.find(orgArpNum);
					if (it == orgNumMap.end()) {	// Make new property
						orgNumMap.emplace(orgArpNum, pair.first);
						int arpNum = instManLocked->findFirstAssignableArpeggioFM();
						if (arpNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
						fm->setArpeggioNumber(pair.first, arpNum);
						size_t arpCsr = getPropertyPosition(propCtr, 0x28, orgArpNum);

						if (arpCsr != std::numeric_limits<size_t>::max()) {
							uint16_t seqLen = propCtr.readUint16(arpCsr);
							arpCsr += 2;
							for (uint16_t l = 0; l < seqLen; ++l) {
								uint16_t data = propCtr.readUint16(arpCsr);
								arpCsr += 2;
								if (l == 0)
									instManLocked->setArpeggioFMSequenceData(arpNum, 0, data);
								else
									instManLocked->addArpeggioFMSequenceData(arpNum, data);
							}

							uint16_t loopCnt = propCtr.readUint16(arpCsr);
							arpCsr += 2;
							for (uint16_t l = 0; l < loopCnt; ++l) {
								int begin = propCtr.readUint16(arpCsr);
								arpCsr += 2;
								int end = propCtr.readUint16(arpCsr);
								arpCsr += 2;
								int times = propCtr.readUint8(arpCsr++);
								instManLocked->addArpeggioFMLoop(arpNum, InstrumentSequenceLoop(begin, end, times));
							}

							switch (propCtr.readUint8(arpCsr++)) {
							case 0x00:	// No release
								instManLocked->setArpeggioFMRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
								break;
							case 0x01:	// Fixed
							{
								uint16_t pos = propCtr.readUint16(arpCsr);
								arpCsr += 2;
								// Release point check (prevents a bug)
								// https://github.com/BambooTracker/BambooTracker/issues/11
								if (pos < seqLen) instManLocked->setArpeggioFMRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
								else instManLocked->setArpeggioFMRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
								break;
							}
							default:
								throw FileCorruptionError(FileType::Bank, arpCsr);
							}

							switch (propCtr.readUint8(arpCsr++)) {
							case 0x00:	// Absolute
								instManLocked->setArpeggioFMType(arpNum, SequenceType::AbsoluteSequence);
								break;
							case 0x01:	// Fixed
								instManLocked->setArpeggioFMType(arpNum, SequenceType::FixedSequence);
								if (bankVersion < Version::toBCD(1, 3, 1)) {
									// Add infinity loop to the last data, to keep compatibility
									auto loopRt = instManLocked->getArpeggioFMLoopRoot(arpNum).getAllLoops();
									if (std::none_of(loopRt.begin(), loopRt.end(), [](InstrumentSequenceLoop& loop) { return loop.isInfinite(); })) {
										auto seq = instManLocked->getArpeggioFMSequence(arpNum);
										if (!seq.empty()) {
											size_t pos = seq.size();
											instManLocked->addArpeggioFMSequenceData(arpNum, seq.back().data);
											instManLocked->addArpeggioFMLoop(arpNum, InstrumentSequenceLoop(pos, pos, InstrumentSequenceLoop::INFINITE_LOOP));
										}
									}
								}
								break;
							case 0x02:	// Relative
								instManLocked->setArpeggioFMType(arpNum, SequenceType::RelativeSequence);
								break;
							default:
								if (bankVersion < Version::toBCD(1, 0, 2)) {
									// Recover deep clone bug
									// https://github.com/BambooTracker/BambooTracker/issues/170
									instManLocked->setArpeggioFMType(arpNum, SequenceType::AbsoluteSequence);
									break;
								}
								else {
									throw FileCorruptionError(FileType::Bank, arpCsr);
								}
							}
						}
					}
					else {	// Use registered property
						fm->setArpeggioNumber(pair.first, fm->getArpeggioNumber(it->second));
					}
				}
			}
		}

		/* Pitch */
		{
			std::unordered_map<FMOperatorType, int> tmpMap;
			std::unordered_map<int, FMOperatorType> orgNumMap;
			tmpMap.emplace(FMOperatorType::All, instCtr.readUint8(instCsr));
			instCsr += 6;
			tmpMap.emplace(FMOperatorType::Op1, instCtr.readUint8(instCsr++));
			tmpMap.emplace(FMOperatorType::Op2, instCtr.readUint8(instCsr++));
			tmpMap.emplace(FMOperatorType::Op3, instCtr.readUint8(instCsr++));
			tmpMap.emplace(FMOperatorType::Op4, instCtr.readUint8(instCsr));
			instCsr -= 8;
			for (auto& pair : tmpMap) {
				if (0x80 & pair.second) {
					fm->setPitchEnabled(pair.first, false);
					fm->setPitchNumber(pair.first, 0x7f & pair.second);
				}
				else {
					fm->setPitchEnabled(pair.first, true);
					uint8_t orgPtNum = 0x7f & pair.second;
					auto it = orgNumMap.find(orgPtNum);
					if (it == orgNumMap.end()) {	// Make new property
						orgNumMap.emplace(orgPtNum, pair.first);
						int ptNum = instManLocked->findFirstAssignablePitchFM();
						if (ptNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
						fm->setPitchNumber(pair.first, ptNum);
						size_t ptCsr = getPropertyPosition(propCtr, 0x29, orgPtNum);

						if (ptCsr != std::numeric_limits<size_t>::max()) {
							uint16_t seqLen = propCtr.readUint16(ptCsr);
							ptCsr += 2;
							for (uint16_t l = 0; l < seqLen; ++l) {
								uint16_t data = propCtr.readUint16(ptCsr);
								ptCsr += 2;
								if (l == 0)
									instManLocked->setPitchFMSequenceData(ptNum, 0, data);
								else
									instManLocked->addPitchFMSequenceData(ptNum, data);
							}

							uint16_t loopCnt = propCtr.readUint16(ptCsr);
							ptCsr += 2;
							for (uint16_t l = 0; l < loopCnt; ++l) {
								int begin = propCtr.readUint16(ptCsr);
								ptCsr += 2;
								int end = propCtr.readUint16(ptCsr);
								ptCsr += 2;
								int times = propCtr.readUint8(ptCsr++);
								instManLocked->addPitchFMLoop(ptNum, InstrumentSequenceLoop(begin, end, times));
							}

							switch (propCtr.readUint8(ptCsr++)) {
							case 0x00:	// No release
								instManLocked->setPitchFMRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
								break;
							case 0x01:	// Fixed
							{
								uint16_t pos = propCtr.readUint16(ptCsr);
								ptCsr += 2;
								// Release point check (prevents a bug)
								// https://github.com/BambooTracker/BambooTracker/issues/11
								if (pos < seqLen) instManLocked->setPitchFMRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
								else instManLocked->setPitchFMRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
								break;
							}
							default:
								throw FileCorruptionError(FileType::Bank, ptCsr);
							}

							switch (propCtr.readUint8(ptCsr++)) {
							case 0x00:	// Absolute
								instManLocked->setPitchFMType(ptNum, SequenceType::AbsoluteSequence);
								break;
							case 0x02:	// Relative
								instManLocked->setPitchFMType(ptNum, SequenceType::RelativeSequence);
								break;
							default:
								if (bankVersion < Version::toBCD(1, 0, 2)) {
									// Recover deep clone bug
									// https://github.com/BambooTracker/BambooTracker/issues/170
									instManLocked->setPitchFMType(ptNum, SequenceType::AbsoluteSequence);
									break;
								}
								else {
									throw FileCorruptionError(FileType::Bank, ptCsr);
								}
							}
						}
					}
					else {	// Use registered property
						fm->setPitchNumber(pair.first, fm->getPitchNumber(it->second));
					}
				}
			}
		}

		/* Envelope reset */
		{
			uint8_t tmp = instCtr.readUint8(instCsr);
			fm->setEnvelopeResetEnabled(FMOperatorType::All, (tmp & 0x01));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op1, (tmp & 0x02));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op2, (tmp & 0x04));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op3, (tmp & 0x08));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op4, (tmp & 0x10));
		}

		/* Pan */
		 if (bankVersion >= Version::toBCD(1, 3, 0)) {
			instCsr += 9;
			 uint8_t tmp = instCtr.readUint8(instCsr);
			if (0x80 & tmp) {
				fm->setPanEnabled(false);
				fm->setPanNumber(0x7f & tmp);
			}
			else {
				fm->setPanEnabled(true);
				uint8_t orgPanNum = 0x7f & tmp;
				int panNum = instManLocked->findFirstAssignablePanFM();
				if (panNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				fm->setPanNumber(panNum);
				size_t panCsr = getPropertyPosition(propCtr, 0x34, orgPanNum);

				if (panCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(panCsr);
					panCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(panCsr);
						panCsr += 2;
						if (l == 0)
							instManLocked->setPanFMSequenceData(panNum, 0, data);
						else
							instManLocked->addPanFMSequenceData(panNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(panCsr);
					panCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(panCsr);
						panCsr += 2;
						int end = propCtr.readUint16(panCsr);
						panCsr += 2;
						int times = propCtr.readUint8(panCsr++);
						instManLocked->addPanFMLoop(panNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(panCsr++)) {
					case 0x00:	// No release
						instManLocked->setPanFMRelease(panNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(panCsr);
						panCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPanFMRelease(panNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPanFMRelease(panNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, panCsr);
					}
				}
			}
		}

		return fm;
	}
	case 0x01:	// SSG
	{
		auto ssg = new InstrumentSSG(instNum, name, instManLocked.get());

		/* Waveform */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				ssg->setWaveformEnabled(false);
				ssg->setWaveformNumber(0x7f & tmp);
			}
			else {
				ssg->setWaveformEnabled(true);
				uint8_t orgWfNum = 0x7f & tmp;
				int wfNum = instManLocked->findFirstAssignableWaveformSSG();
				if (wfNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				ssg->setWaveformNumber(wfNum);
				size_t wfCsr = getPropertyPosition(propCtr, 0x30, orgWfNum);

				if (wfCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(wfCsr);
					wfCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(wfCsr);
						wfCsr += 2;
						int32_t subdata;
						subdata = propCtr.readInt32(wfCsr);
						wfCsr += 4;
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
							instManLocked->setWaveformSSGSequenceData(wfNum, 0, unit);
						else
							instManLocked->addWaveformSSGSequenceData(wfNum, unit);
					}

					uint16_t loopCnt = propCtr.readUint16(wfCsr);
					wfCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(wfCsr);
						wfCsr += 2;
						int end = propCtr.readUint16(wfCsr);
						wfCsr += 2;
						int times = propCtr.readUint8(wfCsr++);
						instManLocked->addWaveformSSGLoop(wfNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(wfCsr++)) {
					case 0x00:	// No release
						instManLocked->setWaveformSSGRelease(wfNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(wfCsr);
						wfCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setWaveformSSGRelease(wfNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setWaveformSSGRelease(wfNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, wfCsr);
					}
				}
			}
		}

		/* Tone/Noise */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				ssg->setToneNoiseEnabled(false);
				ssg->setToneNoiseNumber(0x7f & tmp);
			}
			else {
				ssg->setToneNoiseEnabled(true);
				uint8_t orgTnNum = 0x7f & tmp;
				int tnNum = instManLocked->findFirstAssignableToneNoiseSSG();
				if (tnNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				ssg->setToneNoiseNumber(tnNum);
				size_t tnCsr = getPropertyPosition(propCtr, 0x31, orgTnNum);

				if (tnCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(tnCsr);
					tnCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(tnCsr);
						tnCsr += 2;
						if (bankVersion < Version::toBCD(1, 0, 1)) {
							if (data > 0) {
								uint16_t tmp = data - 1;
								data = tmp / 32 * 32 + (31 - tmp % 32) + 1;
							}
						}
						if (l == 0)
							instManLocked->setToneNoiseSSGSequenceData(tnNum, 0, data);
						else
							instManLocked->addToneNoiseSSGSequenceData(tnNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(tnCsr);
					tnCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(tnCsr);
						tnCsr += 2;
						int end = propCtr.readUint16(tnCsr);
						tnCsr += 2;
						int times = propCtr.readUint8(tnCsr++);
						instManLocked->addToneNoiseSSGLoop(tnNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(tnCsr++)) {
					case 0x00:	// No release
						instManLocked->setToneNoiseSSGRelease(tnNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(tnCsr);
						tnCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setToneNoiseSSGRelease(tnNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setToneNoiseSSGRelease(tnNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, tnCsr);
					}
				}
			}
		}

		/* Envelope */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				ssg->setEnvelopeEnabled(false);
				ssg->setEnvelopeNumber(0x7f & tmp);
			}
			else {
				ssg->setEnvelopeEnabled(true);
				uint8_t orgEnvNum = 0x7f & tmp;
				int envNum = instManLocked->findFirstAssignableEnvelopeSSG();
				if (envNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				ssg->setEnvelopeNumber(envNum);
				size_t envCsr = getPropertyPosition(propCtr, 0x32, orgEnvNum);

				if (envCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(envCsr);
					envCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(envCsr);
						envCsr += 2;
						int32_t subdata;
						subdata = propCtr.readInt32(envCsr);
						envCsr += 4;
						SSGEnvelopeUnit unit = (data < 16) ? SSGEnvelopeUnit::makeOnlyDataUnit(data)
														   : SSGEnvelopeUnit::makeUnitWithDecode(data, subdata);
						if (l == 0)
							instManLocked->setEnvelopeSSGSequenceData(envNum, 0, unit);
						else
							instManLocked->addEnvelopeSSGSequenceData(envNum, unit);
					}

					uint16_t loopCnt = propCtr.readUint16(envCsr);
					envCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(envCsr);
						envCsr += 2;
						int end = propCtr.readUint16(envCsr);
						envCsr += 2;
						int times = propCtr.readUint8(envCsr++);
						instManLocked->addEnvelopeSSGLoop(envNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(envCsr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeSSGRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setEnvelopeSSGRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::AbsoluteRelease, pos));
						else instManLocked->setEnvelopeSSGRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::RelativeRelease, pos));
						else instManLocked->setEnvelopeSSGRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, envCsr);
					}
				}
			}
		}

		/* Arpeggio */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				ssg->setArpeggioEnabled(false);
				ssg->setArpeggioNumber(0x7f & tmp);
			}
			else {
				ssg->setArpeggioEnabled(true);
				uint8_t orgArpNum = 0x7f & tmp;
				int arpNum = instManLocked->findFirstAssignableArpeggioSSG();
				if (arpNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				ssg->setArpeggioNumber(arpNum);
				size_t arpCsr = getPropertyPosition(propCtr, 0x33, orgArpNum);

				if (arpCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						if (l == 0)
							instManLocked->setArpeggioSSGSequenceData(arpNum, 0, data);
						else
							instManLocked->addArpeggioSSGSequenceData(arpNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						int end = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						int times = propCtr.readUint8(arpCsr++);
						instManLocked->addArpeggioSSGLoop(arpNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioSSGRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioSSGRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setArpeggioSSGRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, arpCsr);
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setArpeggioSSGType(arpNum, SequenceType::AbsoluteSequence);
						break;
					case 0x01:	// Fixed
						instManLocked->setArpeggioSSGType(arpNum, SequenceType::FixedSequence);
						if (bankVersion < Version::toBCD(1, 3, 1)) {
							// Add infinity loop to the last data, to keep compatibility
							auto loopRt = instManLocked->getArpeggioSSGLoopRoot(arpNum).getAllLoops();
							if (std::none_of(loopRt.begin(), loopRt.end(), [](InstrumentSequenceLoop& loop) { return loop.isInfinite(); })) {
								auto seq = instManLocked->getArpeggioSSGSequence(arpNum);
								if (!seq.empty()) {
									size_t pos = seq.size();
									instManLocked->addArpeggioSSGSequenceData(arpNum, seq.back().data);
									instManLocked->addArpeggioSSGLoop(arpNum, InstrumentSequenceLoop(pos, pos, InstrumentSequenceLoop::INFINITE_LOOP));
								}
							}
						}
						break;
					case 0x02:	// Relative
						instManLocked->setArpeggioSSGType(arpNum, SequenceType::RelativeSequence);
						break;
					default:
						if (bankVersion < Version::toBCD(1, 0, 2)) {
							// Recover deep clone bug
							// https://github.com/BambooTracker/BambooTracker/issues/170
							instManLocked->setArpeggioSSGType(arpNum, SequenceType::AbsoluteSequence);
							break;
						}
						else {
							throw FileCorruptionError(FileType::Bank, arpCsr);
						}
					}
				}
			}
		}

		/* Pitch */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				ssg->setPitchEnabled(false);
				ssg->setPitchNumber(0x7f & tmp);
			}
			else {
				ssg->setPitchEnabled(true);
				uint8_t orgPtNum = 0x7f & tmp;
				int ptNum = instManLocked->findFirstAssignablePitchSSG();
				if (ptNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				ssg->setPitchNumber(ptNum);
				size_t ptCsr = getPropertyPosition(propCtr, 0x34, orgPtNum);

				if (ptCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						if (l == 0)
							instManLocked->setPitchSSGSequenceData(ptNum, 0, data);
						else
							instManLocked->addPitchSSGSequenceData(ptNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						int end = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						int times = propCtr.readUint8(ptCsr++);
						instManLocked->addPitchSSGLoop(ptNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// No release
						instManLocked->setPitchSSGRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchSSGRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPitchSSGRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, ptCsr);
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setPitchSSGType(ptNum, SequenceType::AbsoluteSequence);
						break;
					case 0x02:	// Relative
						instManLocked->setPitchSSGType(ptNum, SequenceType::RelativeSequence);
						break;
					default:
						if (bankVersion < Version::toBCD(1, 0, 2)) {
							// Recover deep clone bug
							// https://github.com/BambooTracker/BambooTracker/issues/170
							instManLocked->setPitchSSGType(ptNum, SequenceType::AbsoluteSequence);
							break;
						}
						else {
							throw FileCorruptionError(FileType::Bank, ptCsr);
						}
					}
				}
			}
		}

		return ssg;
	}
	case 0x02:	// ADPCM
	{
		auto adpcm = new InstrumentADPCM(instNum, name, instManLocked.get());

		/* Sample */
		{
			uint8_t orgSampNum = instCtr.readUint8(instCsr++);
			int sampNum = instManLocked->findFirstAssignableSampleADPCM();
			if (sampNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
			adpcm->setSampleNumber(sampNum);
			size_t sampCsr = getPropertyPosition(propCtr, 0x40, orgSampNum);
			if (sampCsr != std::numeric_limits<size_t>::max()) {
				instManLocked->setSampleADPCMRootKeyNumber(sampNum, propCtr.readUint8(sampCsr++));
				instManLocked->setSampleADPCMRootDeltaN(sampNum, propCtr.readUint16(sampCsr));
				sampCsr += 2;
				instManLocked->setSampleADPCMRepeatEnabled(sampNum, (propCtr.readUint8(sampCsr++) & 0x01) != 0);
				uint32_t len = propCtr.readUint32(sampCsr);
				sampCsr += 4;
				std::vector<uint8_t> samples = propCtr.getSubcontainer(sampCsr, len).toVector();
				sampCsr += len;
				instManLocked->storeSampleADPCMRawSample(sampNum, samples);
				if (bankVersion >= Version::toBCD(1, 3, 1)) {
					uint16_t repeatBegin = propCtr.readUint16(sampCsr);
					sampCsr += 2;
					uint16_t repeatEnd = propCtr.readUint16(sampCsr);
					sampCsr += 2;
					instManLocked->setSampleADPCMRepeatrange(sampNum, SampleRepeatRange(repeatBegin, repeatEnd));
				}
				else {
					instManLocked->setSampleADPCMRepeatrange(sampNum, SampleRepeatRange(0, (samples.size() - 1) >> 5));
				}
			}
		}

		/* Envelope */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				adpcm->setEnvelopeEnabled(false);
				adpcm->setEnvelopeNumber(0x7f & tmp);
			}
			else {
				adpcm->setEnvelopeEnabled(true);
				uint8_t orgEnvNum = 0x7f & tmp;
				int envNum = instManLocked->findFirstAssignableEnvelopeADPCM();
				if (envNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				adpcm->setEnvelopeNumber(envNum);
				size_t envCsr = getPropertyPosition(propCtr, 0x41, orgEnvNum);

				if (envCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(envCsr);
					envCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (l == 0)
							instManLocked->setEnvelopeADPCMSequenceData(envNum, 0, data);
						else
							instManLocked->addEnvelopeADPCMSequenceData(envNum, data);
						if (bankVersion < Version::toBCD(1, 3, 0)) envCsr += 4;
					}

					uint16_t loopCnt = propCtr.readUint16(envCsr);
					envCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(envCsr);
						envCsr += 2;
						int end = propCtr.readUint16(envCsr);
						envCsr += 2;
						int times = propCtr.readUint8(envCsr++);
						instManLocked->addEnvelopeADPCMLoop(envNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(envCsr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeADPCMRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setEnvelopeADPCMRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::AbsoluteRelease, pos));
						else instManLocked->setEnvelopeADPCMRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::RelativeRelease, pos));
						else instManLocked->setEnvelopeADPCMRelease(envNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, envCsr);
					}
				}
			}
		}

		/* Arpeggio */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				adpcm->setArpeggioEnabled(false);
				adpcm->setArpeggioNumber(0x7f & tmp);
			}
			else {
				adpcm->setArpeggioEnabled(true);
				uint8_t orgArpNum = 0x7f & tmp;
				int arpNum = instManLocked->findFirstAssignableArpeggioADPCM();
				if (arpNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				adpcm->setArpeggioNumber(arpNum);
				size_t arpCsr = getPropertyPosition(propCtr, 0x42, orgArpNum);

				if (arpCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						if (l == 0)
							instManLocked->setArpeggioADPCMSequenceData(arpNum, 0, data);
						else
							instManLocked->addArpeggioADPCMSequenceData(arpNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						int end = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						int times = propCtr.readUint8(arpCsr++);
						instManLocked->addArpeggioADPCMLoop(arpNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioADPCMRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioADPCMRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setArpeggioADPCMRelease(arpNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, arpCsr);
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setArpeggioADPCMType(arpNum, SequenceType::AbsoluteSequence);
						break;
					case 0x01:	// Fixed
						instManLocked->setArpeggioADPCMType(arpNum, SequenceType::FixedSequence);
						if (bankVersion < Version::toBCD(1, 3, 1)) {
							// Add infinity loop to the last data, to keep compatibility
							auto loopRt = instManLocked->getArpeggioADPCMLoopRoot(arpNum).getAllLoops();
							if (std::none_of(loopRt.begin(), loopRt.end(), [](InstrumentSequenceLoop& loop) { return loop.isInfinite(); })) {
								auto seq = instManLocked->getArpeggioADPCMSequence(arpNum);
								if (!seq.empty()) {
									size_t pos = seq.size();
									instManLocked->addArpeggioADPCMSequenceData(arpNum, seq.back().data);
									instManLocked->addArpeggioADPCMLoop(arpNum, InstrumentSequenceLoop(pos, pos, InstrumentSequenceLoop::INFINITE_LOOP));
								}
							}
						}
						break;
					case 0x02:	// Relative
						instManLocked->setArpeggioADPCMType(arpNum, SequenceType::RelativeSequence);
						break;
					default:
						throw FileCorruptionError(FileType::Bank, arpCsr);
					}
				}
			}
		}

		/* Pitch */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				adpcm->setPitchEnabled(false);
				adpcm->setPitchNumber(0x7f & tmp);
			}
			else {
				adpcm->setPitchEnabled(true);
				uint8_t orgPtNum = 0x7f & tmp;
				int ptNum = instManLocked->findFirstAssignablePitchADPCM();
				if (ptNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				adpcm->setPitchNumber(ptNum);
				size_t ptCsr = getPropertyPosition(propCtr, 0x43, orgPtNum);

				if (ptCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						if (l == 0)
							instManLocked->setPitchADPCMSequenceData(ptNum, 0, data);
						else
							instManLocked->addPitchADPCMSequenceData(ptNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						int end = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						int times = propCtr.readUint8(ptCsr++);
						instManLocked->addPitchADPCMLoop(ptNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// No release
						instManLocked->setPitchADPCMRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchADPCMRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPitchADPCMRelease(ptNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, ptCsr);
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setPitchADPCMType(ptNum, SequenceType::AbsoluteSequence);
						break;
					case 0x02:	// Relative
						instManLocked->setPitchADPCMType(ptNum, SequenceType::RelativeSequence);
						break;
					default:
						throw FileCorruptionError(FileType::Bank, ptCsr);
					}
				}
			}
		}

		/* Pan */
		if (bankVersion >= Version::toBCD(1, 3, 0)) {
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				adpcm->setPanEnabled(false);
				adpcm->setPanNumber(0x7f & tmp);
			}
			else {
				adpcm->setPanEnabled(true);
				uint8_t orgPanNum = 0x7f & tmp;
				int panNum = instManLocked->findFirstAssignablePanADPCM();
				if (panNum == -1) throw FileCorruptionError(FileType::Bank, instCsr);
				adpcm->setPanNumber(panNum);
				size_t panCsr = getPropertyPosition(propCtr, 0x34, orgPanNum);

				if (panCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(panCsr);
					panCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(panCsr);
						panCsr += 2;
						if (l == 0)
							instManLocked->setPanADPCMSequenceData(panNum, 0, data);
						else
							instManLocked->addPanADPCMSequenceData(panNum, data);
					}

					uint16_t loopCnt = propCtr.readUint16(panCsr);
					panCsr += 2;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						int begin = propCtr.readUint16(panCsr);
						panCsr += 2;
						int end = propCtr.readUint16(panCsr);
						panCsr += 2;
						int times = propCtr.readUint8(panCsr++);
						instManLocked->addPanADPCMLoop(panNum, InstrumentSequenceLoop(begin, end, times));
					}

					switch (propCtr.readUint8(panCsr++)) {
					case 0x00:	// No release
						instManLocked->setPanADPCMRelease(panNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(panCsr);
						panCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/BambooTracker/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPanADPCMRelease(panNum, InstrumentSequenceRelease(InstrumentSequenceRelease::FixedRelease, pos));
						else instManLocked->setPanADPCMRelease(panNum, InstrumentSequenceRelease(InstrumentSequenceRelease::NoRelease));
						break;
					}
					default:
						throw FileCorruptionError(FileType::Bank, panCsr);
					}
				}
			}
		}

		return adpcm;
	}
	case 0x03:	// Drumkit
	{
		auto kit = new InstrumentDrumkit(instNum, name, instManLocked.get());

		uint8_t keyCnt = instCtr.readUint8(instCsr++);
		std::unordered_map<int, int> sampMap;
		int newSamp = 0;
		for (uint8_t i = 0; i < keyCnt; ++i) {
			int key = instCtr.readUint8(instCsr++);
			kit->setSampleEnabled(key, true);

			/* Sample */
			{
				uint8_t orgSamp = instCtr.readUint8(instCsr++);
				if (sampMap.count(orgSamp)) {	// Use registered property
					kit->setSampleNumber(key, sampMap.at(orgSamp));
				}
				else {
					newSamp = instManLocked->findFirstAssignableSampleADPCM(newSamp);
					if (newSamp == -1) throw FileCorruptionError(FileType::Bank, instCsr);
					kit->setSampleNumber(key, newSamp);
					sampMap[orgSamp] = newSamp;
					size_t sampCsr = getPropertyPosition(propCtr, 0x40, orgSamp);
					if (sampCsr != std::numeric_limits<size_t>::max()) {
						instManLocked->setSampleADPCMRootKeyNumber(newSamp, propCtr.readUint8(sampCsr++));
						instManLocked->setSampleADPCMRootDeltaN(newSamp, propCtr.readUint16(sampCsr));
						sampCsr += 2;
						instManLocked->setSampleADPCMRepeatEnabled(newSamp, (propCtr.readUint8(sampCsr++) & 0x01) != 0);
						uint32_t len = propCtr.readUint32(sampCsr);
						sampCsr += 4;
						std::vector<uint8_t> samples = propCtr.getSubcontainer(sampCsr, len).toVector();
						sampCsr += len;
						instManLocked->storeSampleADPCMRawSample(newSamp, samples);
						if (bankVersion >= Version::toBCD(1, 3, 1)) {
							uint16_t repeatBegin = propCtr.readUint16(sampCsr);
							sampCsr += 2;
							uint16_t repeatEnd = propCtr.readUint16(sampCsr);
							sampCsr += 2;
							instManLocked->setSampleADPCMRepeatrange(newSamp, SampleRepeatRange(repeatBegin, repeatEnd));
						}
						else {
							instManLocked->setSampleADPCMRepeatrange(newSamp, SampleRepeatRange(0, (samples.size() - 1) >> 5));
						}
						++newSamp;	// Increment for search
					}
				}
			}

			/* Pitch */
			kit->setPitch(key, instCtr.readInt8(instCsr++));

			/* Pan */
			if (bankVersion >= Version::toBCD(1, 3, 0)) {
				kit->setPan(key, instCtr.readUint8(instCsr++));
			}
		}

		return kit;
	}
	default:
		throw FileCorruptionError(FileType::Bank, instCsr);
	}
}
}
