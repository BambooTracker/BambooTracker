#include "file_io.hpp"
#include <fstream>
#include <locale>
#include "version.hpp"
#include "file_io_error.hpp"
#include "misc.hpp"
#include "bank.hpp"
#include "format/wopn_file.h"

const FMEnvelopeParameter FileIO::ENV_FM_PARAMS[38] = {
	FMEnvelopeParameter::AL,
	FMEnvelopeParameter::FB,
	FMEnvelopeParameter::AR1,
	FMEnvelopeParameter::DR1,
	FMEnvelopeParameter::SR1,
	FMEnvelopeParameter::RR1,
	FMEnvelopeParameter::SL1,
	FMEnvelopeParameter::TL1,
	FMEnvelopeParameter::KS1,
	FMEnvelopeParameter::ML1,
	FMEnvelopeParameter::DT1,
	FMEnvelopeParameter::AR2,
	FMEnvelopeParameter::DR2,
	FMEnvelopeParameter::SR2,
	FMEnvelopeParameter::RR2,
	FMEnvelopeParameter::SL2,
	FMEnvelopeParameter::TL2,
	FMEnvelopeParameter::KS2,
	FMEnvelopeParameter::ML2,
	FMEnvelopeParameter::DT2,
	FMEnvelopeParameter::AR3,
	FMEnvelopeParameter::DR3,
	FMEnvelopeParameter::SR3,
	FMEnvelopeParameter::RR3,
	FMEnvelopeParameter::SL3,
	FMEnvelopeParameter::TL3,
	FMEnvelopeParameter::KS3,
	FMEnvelopeParameter::ML3,
	FMEnvelopeParameter::DT3,
	FMEnvelopeParameter::AR4,
	FMEnvelopeParameter::DR4,
	FMEnvelopeParameter::SR4,
	FMEnvelopeParameter::RR4,
	FMEnvelopeParameter::SL4,
	FMEnvelopeParameter::TL4,
	FMEnvelopeParameter::KS4,
	FMEnvelopeParameter::ML4,
	FMEnvelopeParameter::DT4
};

void FileIO::saveModule(std::string path, std::weak_ptr<Module> mod,
						std::weak_ptr<InstrumentsManager> instMan)
{
	BinaryContainer ctr;

	ctr.appendString("BambooTrackerMod");
	size_t eofOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy EOF offset
	uint32_t fileVersion = Version::ofModuleFileInBCD();
	ctr.appendUint32(fileVersion);


	/***** Module section *****/
	ctr.appendString("MODULE  ");
	size_t modOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy module section offset
	std::string modTitle = mod.lock()->getTitle();
	ctr.appendUint32(modTitle.length());
	if (!modTitle.empty()) ctr.appendString(modTitle);
	std::string author = mod.lock()->getAuthor();
	ctr.appendUint32(author.length());
	if (!author.empty()) ctr.appendString(author);
	std::string copyright = mod.lock()->getCopyright();
	ctr.appendUint32(copyright.length());
	if (!copyright.empty()) ctr.appendString(copyright);
	std::string comment = mod.lock()->getComment();
	ctr.appendUint32(comment.length());
	if (!comment.empty()) ctr.appendString(comment);
	ctr.appendUint32(mod.lock()->getTickFrequency());
	ctr.appendUint32(mod.lock()->getStepHighlightDistance());
	ctr.writeUint32(modOfs, ctr.size() - modOfs);


	/***** Instrument section *****/
	ctr.appendString("INSTRMNT");
	size_t instOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument section offset
	std::vector<int> instIdcs = instMan.lock()->getEntriedInstrumentIndices();
	ctr.appendUint8(instIdcs.size());
	for (auto& idx : instIdcs) {
		if (std::shared_ptr<AbstractInstrument> inst = instMan.lock()->getInstrumentSharedPtr(idx)) {
			ctr.appendUint8(inst->getNumber());
			size_t iOfs = ctr.size();
			ctr.appendUint32(0);	// Dummy instrument block offset
			std::string name = inst->getName();
			ctr.appendUint32(name.length());
			if (!name.empty()) ctr.appendString(name);
			switch (inst->getSoundSource()) {
			case SoundSource::FM:
			{
				ctr.appendUint8(0x00);
				auto instFM = std::dynamic_pointer_cast<InstrumentFM>(inst);
				ctr.appendUint8(instFM->getEnvelopeNumber());
				uint8_t tmp = instFM->getLFONumber();
				ctr.appendUint8(instFM->getLFOEnabled() ? tmp : (0x80 | tmp));
				for (auto& param : ENV_FM_PARAMS) {
					tmp = instFM->getOperatorSequenceNumber(param);
					ctr.appendUint8(instFM->getOperatorSequenceEnabled(param) ? tmp : (0x80 | tmp));
				}
				tmp = instFM->getArpeggioNumber();
				ctr.appendUint8(instFM->getArpeggioEnabled() ? tmp : (0x80 | tmp));
				tmp = instFM->getPitchNumber();
				ctr.appendUint8(instFM->getPitchEnabled() ? tmp : (0x80 | tmp));
				ctr.appendUint8(instFM->getEnvelopeResetEnabled());
				break;
			}
			case SoundSource::SSG:
			{
				ctr.appendUint8(0x01);
				auto instSSG = std::dynamic_pointer_cast<InstrumentSSG>(inst);
				uint8_t tmp = instSSG->getWaveFormNumber();
				ctr.appendUint8(instSSG->getWaveFormEnabled() ? tmp : (0x80 | tmp));
				tmp = instSSG->getToneNoiseNumber();
				ctr.appendUint8(instSSG->getToneNoiseEnabled() ? tmp : (0x80 | tmp));
				tmp = instSSG->getEnvelopeNumber();
				ctr.appendUint8(instSSG->getEnvelopeEnabled() ? tmp : (0x80 | tmp));
				tmp = instSSG->getArpeggioNumber();
				ctr.appendUint8(instSSG->getArpeggioEnabled() ? tmp : (0x80 | tmp));
				tmp = instSSG->getPitchNumber();
				ctr.appendUint8(instSSG->getPitchEnabled() ? tmp : (0x80 | tmp));
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
	std::vector<int> envFMIdcs = instMan.lock()->getEnvelopeFMEntriedIndices();
	if (!envFMIdcs.empty()) {
		ctr.appendUint8(0x00);
		ctr.appendUint8(envFMIdcs.size());
		for (auto& idx : envFMIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AL) << 4)
						  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::FB);
			ctr.appendUint8(tmp);
			// Operator 1
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 0);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR1);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS1) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR1);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT1) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR1);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL1) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR1);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL1);
			ctr.appendUint8(tmp);
			int tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG1);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML1);
			ctr.appendUint8(tmp);
			// Operator 2
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 1);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR2);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS2) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR2);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT2) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR2);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL2) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR2);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL2);
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG2);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML2);
			ctr.appendUint8(tmp);
			// Operator 3
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 2);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR3);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS3) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR3);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT3) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR3);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL3) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR3);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL3);
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG3);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML3);
			ctr.appendUint8(tmp);
			// Operator 4
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 3);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR4);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS4) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR4);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT4) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR4);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL4) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR4);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL4);
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG4);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML4);
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, ctr.size() - ofs);
		}
	}

	// FM LFO
	std::vector<int> lfoFMIdcs = instMan.lock()->getLFOFMEntriedIndices();
	if (!lfoFMIdcs.empty()) {
		ctr.appendUint8(0x01);
		ctr.appendUint8(lfoFMIdcs.size());
		for (auto& idx : lfoFMIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = (instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::FREQ) << 4)
						  | instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::PMS);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM4) << 7)
				  | (instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM3) << 6)
				  | (instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM2) << 5)
				  | (instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AM1) << 4)
				  | instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::AMS);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::COUNT);
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, ctr.size() - ofs);
		}
	}

	// FM envelope parameter
	for (size_t i = 0; i < 38; ++i) {
		std::vector<int> idcs = instMan.lock()->getOperatorSequenceFMEntriedIndices(ENV_FM_PARAMS[i]);
		if (!idcs.empty()) {
			ctr.appendUint8(0x02 + i);
			ctr.appendUint8(idcs.size());
			for (auto& idx : idcs) {
				ctr.appendUint8(idx);
				size_t ofs = ctr.size();
				ctr.appendUint16(0);	// Dummy offset
				auto seq = instMan.lock()->getOperatorSequenceFMSequence(ENV_FM_PARAMS[i], idx);
				ctr.appendUint16(seq.size());
				for (auto& com : seq) {
					ctr.appendUint16(com.type);
					ctr.appendInt16(com.data);
				}
				auto loop = instMan.lock()->getOperatorSequenceFMLoops(ENV_FM_PARAMS[i], idx);
				ctr.appendUint16(loop.size());
				for (auto& l : loop) {
					ctr.appendUint16(l.begin);
					ctr.appendUint16(l.end);
					ctr.appendUint8(l.times);
				}
				auto release = instMan.lock()->getOperatorSequenceFMRelease(ENV_FM_PARAMS[i], idx);
				switch (release.type) {
				case ReleaseType::NO_RELEASE:
					ctr.appendUint8(0x00);
					break;
				case ReleaseType::FIX:
					ctr.appendUint8(0x01);
					ctr.appendUint16(release.begin);
					break;
				case ReleaseType::ABSOLUTE:
					ctr.appendUint8(0x02);
					ctr.appendUint16(release.begin);
					break;
				case ReleaseType::RELATIVE:
					ctr.appendUint8(0x03);
					ctr.appendUint16(release.begin);
					break;
				}
				if (fileVersion >= Version::toBCD(1, 0, 1)) {
					ctr.appendUint8(0);	// Skip sequence type
				}
				ctr.writeUint16(ofs, ctr.size() - ofs);
			}
		}
	}

	// FM arpeggio
	std::vector<int> arpFMIdcs = instMan.lock()->getArpeggioFMEntriedIndices();
	if (!arpFMIdcs.empty()) {
		ctr.appendUint8(0x28);
		ctr.appendUint8(arpFMIdcs.size());
		for (auto& idx : arpFMIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioFMSequence(idx);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getArpeggioFMLoops(idx);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getArpeggioFMRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getArpeggioFMType(idx));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	// FM pitch
	std::vector<int> ptFMIdcs = instMan.lock()->getPitchFMEntriedIndices();
	if (!ptFMIdcs.empty()) {
		ctr.appendUint8(0x29);
		ctr.appendUint8(ptFMIdcs.size());
		for (auto& idx : ptFMIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchFMSequence(idx);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getPitchFMLoops(idx);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getPitchFMRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getPitchFMType(idx));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	// SSG wave form
	std::vector<int> wfSSGIdcs = instMan.lock()->getWaveFormSSGEntriedIndices();
	if (!wfSSGIdcs.empty()) {
		ctr.appendUint8(0x30);
		ctr.appendUint8(wfSSGIdcs.size());
		for (auto& idx : wfSSGIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getWaveFormSSGSequence(idx);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getWaveFormSSGLoops(idx);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getWaveFormSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(0);	// Skip sequence type
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	// SSG tone/noise
	std::vector<int> tnSSGIdcs = instMan.lock()->getToneNoiseSSGEntriedIndices();
	if (!tnSSGIdcs.empty()) {
		ctr.appendUint8(0x31);
		ctr.appendUint8(tnSSGIdcs.size());
		for (auto& idx : tnSSGIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getToneNoiseSSGSequence(idx);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getToneNoiseSSGLoops(idx);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getToneNoiseSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(0);	// Skip sequence type
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	// SSG envelope
	std::vector<int> envSSGIdcs = instMan.lock()->getEnvelopeSSGEntriedIndices();
	if (!envSSGIdcs.empty()) {
		ctr.appendUint8(0x32);
		ctr.appendUint8(envSSGIdcs.size());
		for (auto& idx : envSSGIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getEnvelopeSSGSequence(idx);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getEnvelopeSSGLoops(idx);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getEnvelopeSSGRelease(idx);

			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(0);	// Skip sequence type
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	// SSG arpeggio
	std::vector<int> arpSSGIdcs = instMan.lock()->getArpeggioSSGEntriedIndices();
	if (!arpSSGIdcs.empty()) {
		ctr.appendUint8(0x33);
		ctr.appendUint8(arpSSGIdcs.size());
		for (auto& idx : arpSSGIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioSSGSequence(idx);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getArpeggioSSGLoops(idx);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getArpeggioSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getArpeggioSSGType(idx));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	// SSG pitch
	std::vector<int> ptSSGIdcs = instMan.lock()->getPitchSSGEntriedIndices();
	if (!ptSSGIdcs.empty()) {
		ctr.appendUint8(0x34);
		ctr.appendUint8(ptSSGIdcs.size());
		for (auto& idx : ptSSGIdcs) {
			ctr.appendUint8(idx);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchSSGSequence(idx);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getPitchSSGLoops(idx);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getPitchSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getPitchSSGType(idx));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);


	/***** Groove section *****/
	ctr.appendString("GROOVE  ");
	size_t grooveOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy groove section offset
	size_t grooveCnt = mod.lock()->getGrooveCount();
	ctr.appendUint8(grooveCnt - 1);
	for (size_t i = 0; i < grooveCnt; ++i) {
		ctr.appendUint8(i);
		auto seq = mod.lock()->getGroove(i).getSequence();
		ctr.appendUint8(seq.size());
		for (auto& g : seq) {
			ctr.appendUint8(g);
		}
	}
	ctr.writeUint32(grooveOfs, ctr.size() - grooveOfs);


	/***** Song section *****/
	ctr.appendString("SONG    ");
	size_t songSecOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy song section offset
	size_t songCnt = mod.lock()->getSongCount();
	ctr.appendUint8(songCnt);

	// Song
	for (size_t i = 0; i < songCnt; ++i) {
		ctr.appendUint8(i);
		size_t songOfs = ctr.size();
		ctr.appendUint32(0);	// Dummy song block offset
		auto& sng = mod.lock()->getSong(i);
		std::string title = sng.getTitle();
		ctr.appendUint32(title.length());
		if (!title.empty()) ctr.appendString(title);
		ctr.appendUint32(sng.getTempo());
		uint8_t tmp = sng.getGroove();
		ctr.appendUint8(sng.isUsedTempo() ? (0x80 | tmp) : tmp);
		ctr.appendUint32(sng.getSpeed());
		ctr.appendUint8(sng.getDefaultPatternSize() - 1);
		auto style = sng.getStyle();
		switch (style.type) {
		case SongType::STD:	ctr.appendUint8(0x00);	break;
		default:									break;
		}

		// Track
		for (auto& attrib : style.trackAttribs) {
			ctr.appendUint8(attrib.number);
			size_t trackOfs = ctr.size();
			ctr.appendUint32(0);	// Dummy track subblock offset
			auto& track = sng.getTrack(attrib.number);

			// Order
			size_t odrSize = track.getOrderSize();
			ctr.appendUint8(odrSize - 1);
			for (size_t o = 0; o < odrSize; ++o)
				ctr.appendUint8(track.getOrderData(o).patten);

			// Pattern
			for (auto& idx : track.getEditedPatternIndices()) {
				ctr.appendUint8(idx);
				size_t ptnOfs = ctr.size();
				ctr.appendUint32(0);	// Dummy pattern subblock offset
				auto& pattern = track.getPattern(idx);

				// Step
				std::vector<int> stepIdcs = pattern.getEditedStepIndices();
				for (auto& sidx : stepIdcs) {
					ctr.appendUint8(sidx);
					size_t evFlagOfs = ctr.size();
					ctr.appendUint16(0);	// Dummy set event flag
					auto& step = pattern.getStep(sidx);
					uint16_t eventFlag = 0;
					int tmp = step.getNoteNumber();
					if (tmp != -1) {
						eventFlag |= 0x0001;
						ctr.appendInt8(tmp);
					}
					tmp = step.getInstrumentNumber();
					if (tmp != -1) {
						eventFlag |= 0x0002;
						ctr.appendUint8(tmp);
					}
					tmp = step.getVolume();
					if (tmp != -1) {
						eventFlag |= 0x0004;
						ctr.appendUint8(tmp);
					}
					for (int i = 0; i < 4; ++i) {
						std::string tmpstr = step.getEffectID(i);
						if (tmpstr != "--") {
							eventFlag |= (0x0008 << (i << 1));
							ctr.appendString(tmpstr);
						}
						tmp = step.getEffectValue(i);
						if (tmp != -1) {
							eventFlag |= (0x0010 << (i << 1));
							ctr.appendUint8(tmp);
						}
					}
					ctr.writeUint16(evFlagOfs, eventFlag);
				}

				ctr.writeUint32(ptnOfs, ctr.size() - ptnOfs);
			}

			ctr.writeUint32(trackOfs, ctr.size() - trackOfs);
		}
		ctr.writeUint32(songOfs, ctr.size() - songOfs);

	}
	ctr.writeUint32(songSecOfs, ctr.size() - songSecOfs);

	ctr.writeUint32(eofOfs, ctr.size() - eofOfs);

	mod.lock()->setFilePath(path);

	if (!ctr.save(path)) throw FileOutputError(FileIOError::FileType::MOD);
}

void FileIO::loadModule(std::string path, std::weak_ptr<Module> mod,
						std::weak_ptr<InstrumentsManager> instMan)
{
	BinaryContainer ctr;

	if (!ctr.load(path)) throw FileInputError(FileIOError::FileType::MOD);

	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerMod")
		throw FileCorruptionError(FileIOError::FileType::MOD);
	globCsr += 16;
	size_t eofOfs = ctr.readUint32(globCsr);
	size_t eof = globCsr + eofOfs;
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofModuleFileInBCD())
		throw FileVersionError(fileVersion, Version::ofApplicationInBCD(), FileIOError::FileType::MOD);
	globCsr += 4;

	while (globCsr < eof) {
		if (ctr.readString(globCsr, 8) == "MODULE  ")
			globCsr = loadModuleSectionInModule(mod, ctr, globCsr + 8, fileVersion);
		else if (ctr.readString(globCsr, 8) == "INSTRMNT")
			globCsr = loadInstrumentSectionInModule(instMan, ctr, globCsr + 8, fileVersion);
		else if (ctr.readString(globCsr, 8) == "INSTPROP")
			globCsr = loadInstrumentPropertySectionInModule(instMan, ctr, globCsr + 8, fileVersion);
		else if (ctr.readString(globCsr, 8) == "GROOVE  ")
			globCsr = loadGrooveSectionInModule(mod, ctr, globCsr + 8, fileVersion);
		else if (ctr.readString(globCsr, 8) == "SONG    ")
			globCsr = loadSongSectionInModule(mod, ctr, globCsr + 8, fileVersion);
		else
			throw FileCorruptionError(FileIOError::FileType::MOD);
	}

	mod.lock()->setFilePath(path);
}

size_t FileIO::loadModuleSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr,
										 size_t globCsr, uint32_t version)
{
	size_t modOfs = ctr.readUint32(globCsr);
	size_t modCsr = globCsr + 4;
	size_t modTitleLen = ctr.readUint32(modCsr);
	modCsr += 4;
	if (modTitleLen > 0) {
		mod.lock()->setTitle(ctr.readString(modCsr, modTitleLen));
		modCsr += modTitleLen;
	}
	size_t modAuthorLen = ctr.readUint32(modCsr);
	modCsr += 4;
	if (modAuthorLen > 0) {
		mod.lock()->setAuthor(ctr.readString(modCsr, modAuthorLen));
		modCsr += modAuthorLen;
	}
	size_t modCopyrightLen = ctr.readUint32(modCsr);
	modCsr += 4;
	if (modCopyrightLen > 0) {
		mod.lock()->setCopyright(ctr.readString(modCsr, modCopyrightLen));
		modCsr += modCopyrightLen;
	}
	size_t modCommentLen = ctr.readUint32(modCsr);
	modCsr += 4;
	if (modCommentLen > 0) {
		mod.lock()->setComment(ctr.readString(modCsr, modCommentLen));
		modCsr += modCommentLen;
	}
	mod.lock()->setTickFrequency(ctr.readUint32(modCsr));
	modCsr += 4;
	mod.lock()->setStepHighlightDistance(ctr.readUint32(modCsr));
	modCsr += 4;

	return globCsr + modOfs;
}

size_t FileIO::loadInstrumentSectionInModule(std::weak_ptr<InstrumentsManager> instMan,
											 BinaryContainer& ctr, size_t globCsr, uint32_t version)
{
	size_t instOfs = ctr.readUint32(globCsr);
	size_t instCsr = globCsr + 4;
	uint8_t instCnt = ctr.readUint8(instCsr);
	instCsr += 1;
	for (uint8_t i = 0; i < instCnt; ++i) {
		uint8_t idx = ctr.readUint8(instCsr);
		instCsr += 1;
		size_t iOfs = ctr.readUint32(instCsr);
		size_t iCsr = instCsr + 4;
		size_t nameLen = ctr.readUint32(iCsr);
		iCsr += 4;
		std::string name = u8"";
		if (nameLen > 0) {
			name = ctr.readString(iCsr, nameLen);
			iCsr += nameLen;
		}
		switch (ctr.readUint8(iCsr++)) {
		case 0x00:	// FM
		{
			InstrumentFM* instFM = new InstrumentFM(idx, name, instMan.lock().get());
			instFM->setEnvelopeNumber(ctr.readUint8(iCsr));
			iCsr += 1;
			uint8_t tmp = ctr.readUint8(iCsr);
			instFM->setLFOEnabled((0x80 & tmp) ? false : true);
			instFM->setLFONumber(0x7f & tmp);
			iCsr += 1;
			for (auto& param : ENV_FM_PARAMS) {
				tmp = ctr.readUint8(iCsr);
				instFM->setOperatorSequenceEnabled(param, (0x80 & tmp) ? false : true);
				instFM->setOperatorSequenceNumber(param, 0x7f & tmp);
				iCsr += 1;
			}
			tmp = ctr.readUint8(iCsr);
			instFM->setArpeggioEnabled((0x80 & tmp) ? false : true);
			instFM->setArpeggioNumber(0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instFM->setPitchEnabled((0x80 & tmp) ? false : true);
			instFM->setPitchNumber(0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instFM->setEnvelopeResetEnabled(tmp ? true : false);
			iCsr += 1;
			instMan.lock()->addInstrument(std::unique_ptr<AbstractInstrument>(instFM));
			break;
		}
		case 0x01:	// SSG
		{
			InstrumentSSG* instSSG = new InstrumentSSG(idx, name, instMan.lock().get());
			uint8_t tmp = ctr.readUint8(iCsr);
			instSSG->setWaveFormEnabled((0x80 & tmp) ? false : true);
			instSSG->setWaveFormNumber(0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instSSG->setToneNoiseEnabled((0x80 & tmp) ? false : true);
			instSSG->setToneNoiseNumber(0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instSSG->setEnvelopeEnabled((0x80 & tmp) ? false : true);
			instSSG->setEnvelopeNumber(0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instSSG->setArpeggioEnabled((0x80 & tmp) ? false : true);
			instSSG->setArpeggioNumber(0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instSSG->setPitchEnabled((0x80 & tmp) ? false : true);
			instSSG->setPitchNumber(0x7f & tmp);
			iCsr += 1;
			instMan.lock()->addInstrument(std::unique_ptr<AbstractInstrument>(instSSG));
			break;
		}
		}
		instCsr += iOfs;
	}

	return globCsr + instOfs;
}

size_t FileIO::loadInstrumentPropertySectionInModule(std::weak_ptr<InstrumentsManager> instMan,
													 BinaryContainer& ctr, size_t globCsr,
													 uint32_t version)
{
	size_t instPropOfs = ctr.readUint32(globCsr);
	size_t instPropCsr = globCsr + 4;
	globCsr += instPropOfs;
	while (instPropCsr < globCsr) {
		switch (ctr.readUint8(instPropCsr++)) {
		case 0x00:	// FM envelope
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint8_t ofs = ctr.readUint8(instPropCsr);
				size_t csr = instPropCsr + 1;
				uint8_t tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AL, tmp >> 4);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::FB, tmp & 0x0f);
				// Operator 1
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 0, (0x20 & tmp) ? true : false);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR1, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS1, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR1, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT1, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR1, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL1, tmp >> 4);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR1, tmp & 0x0f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL1, tmp);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML1, tmp & 0x0f);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG1,
													   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				// Operator 2
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 1, (0x20 & tmp) ? true : false);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR2, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS2, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR2, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT2, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR2, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL2, tmp >> 4);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR2, tmp & 0x0f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL2, tmp);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML2, tmp & 0x0f);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG2,
													   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				// Operator 3
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 2, (0x20 & tmp) ? true : false);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR3, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS3, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR3, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT3, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR3, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL3, tmp >> 4);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR3, tmp & 0x0f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL3, tmp);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML3, tmp & 0x0f);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG3,
													   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				// Operator 4
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 3, (0x20 & tmp) ? true : false);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR4, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS4, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR4, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT4, tmp >> 5);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR4, tmp & 0x1f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL4, tmp >> 4);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR4, tmp & 0x0f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL4, tmp);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML4, tmp & 0x0f);
				instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG4,
													   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				instPropCsr += ofs;
			}
			break;
		}
		case 0x01:	// FM LFO
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint8_t ofs = ctr.readUint8(instPropCsr);
				size_t csr = instPropCsr + 1;
				uint8_t tmp = ctr.readUint8(csr++);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::FREQ, tmp >> 4);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::PMS, tmp & 0x0f);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AMS, tmp & 0x0f);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM1, (tmp & 0x10) ? true : false);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM2, (tmp & 0x20) ? true : false);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM3, (tmp & 0x40) ? true : false);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM4, (tmp & 0x80) ? true : false);
				tmp = ctr.readUint8(csr++);
				instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::COUNT, tmp);
				instPropCsr += ofs;
			}
			break;
		}
		case 0x02:	// FM AL
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::AL, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x03:	// FM FB
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::FB, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x04:	// FM AR1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::AR1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x05:	// FM DR1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DR1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x06:	// FM SR1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SR1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x07:	// FM RR1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::RR1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x08:	// FM SL1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SL1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x09:	// FM TL1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::TL1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x0a:	// FM KS1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::KS1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x0b:	// FM ML1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::ML1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x0c:	// FM DT1
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DT1, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x0d:	// FM AR2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::AR2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x0e:	// FM DR2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DR2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x0f:	// FM SR2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SR2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x10:	// FM RR2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::RR2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x11:	// FM SL2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SL2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x12:	// FM TL2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::TL2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x13:	// FM KS2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::KS2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x14:	// FM ML2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::ML2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x15:	// FM DT2
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DT2, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x16:	// FM AR3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::AR3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x17:	// FM DR3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DR3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x18:	// FM SR3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SR3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x19:	// FM RR3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::RR3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x1a:	// FM SL3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SL3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x1b:	// FM TL3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::TL3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x1c:	// FM KS3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::KS3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x1d:	// FM ML3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::ML3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x1e:	// FM DT3
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DT3, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x1f:	// FM AR4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::AR4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x20:	// FM DR4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DR4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x21:	// FM SR4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SR4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x22:	// FM RR4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::RR4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x23:	// FM SL4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::SL4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x24:	// FM TL4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::TL4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x25:	// FM KS4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::KS4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x26:	// FM ML4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::ML4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x27:	// FM DT4
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i)
				instPropCsr += loadInstrumentPropertyOperatorSequence(
								  FMEnvelopeParameter::DT4, instPropCsr, instMan, ctr, version);
			break;
		}
		case 0x28:	// FM arpeggio
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint16_t ofs = ctr.readUint16(instPropCsr);
				size_t csr = instPropCsr + 2;

				uint16_t seqLen = ctr.readUint16(csr);
				csr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = ctr.readUint16(csr);
					csr += 2;
					int16_t subdata = ctr.readInt16(csr);
					csr += 2;
					if (l == 0)
						instMan.lock()->setArpeggioFMSequenceCommand(idx, 0, data, subdata);
					else
						instMan.lock()->addArpeggioFMSequenceCommand(idx, data, subdata);
				}

				uint16_t loopCnt = ctr.readUint16(csr);
				csr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(ctr.readUint16(csr));
						csr += 2;
						ends.push_back(ctr.readUint16(csr));
						csr += 2;
						times.push_back(ctr.readUint8(csr++));
					}
					instMan.lock()->setArpeggioFMLoops(idx, begins, ends, times);
				}
				switch (ctr.readUint8(csr++)) {
				case 0x00:	// No release
					instMan.lock()->setArpeggioFMRelease(
								idx, ReleaseType::NO_RELEASE, -1);
					break;
				case 0x01:	// Fix
					instMan.lock()->setArpeggioFMRelease(
								idx, ReleaseType::FIX, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x02:	// Absolute
					instMan.lock()->setArpeggioFMRelease(
								idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x03:	// Relative
					instMan.lock()->setArpeggioFMRelease(
								idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
					csr += 2;
					break;
				}
				if (version >= Version::toBCD(1, 0, 1)) {
					instMan.lock()->setArpeggioFMType(idx, ctr.readUint8(csr++));
				}

				instPropCsr += ofs;
			}
			break;
		}
		case 0x29:	// FM pitch
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint16_t ofs = ctr.readUint16(instPropCsr);
				size_t csr = instPropCsr + 2;

				uint16_t seqLen = ctr.readUint16(csr);
				csr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = ctr.readUint16(csr);
					csr += 2;
					int16_t subdata = ctr.readInt16(csr);
					csr += 2;
					if (l == 0)
						instMan.lock()->setPitchFMSequenceCommand(idx, 0, data, subdata);
					else
						instMan.lock()->addPitchFMSequenceCommand(idx, data, subdata);
				}

				uint16_t loopCnt = ctr.readUint16(csr);
				csr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(ctr.readUint16(csr));
						csr += 2;
						ends.push_back(ctr.readUint16(csr));
						csr += 2;
						times.push_back(ctr.readUint8(csr++));
					}
					instMan.lock()->setPitchFMLoops(idx, begins, ends, times);
				}

				switch (ctr.readUint8(csr++)) {
				case 0x00:	// No release
					instMan.lock()->setPitchFMRelease(
								idx, ReleaseType::NO_RELEASE, -1);
					break;
				case 0x01:	// Fix
					instMan.lock()->setPitchFMRelease(
								idx, ReleaseType::FIX, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x02:	// Absolute
					instMan.lock()->setPitchFMRelease(
								idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x03:	// Relative
					instMan.lock()->setPitchFMRelease(
								idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
					csr += 2;
					break;
				}
				if (version >= Version::toBCD(1, 0, 1)) {
					instMan.lock()->setPitchFMType(idx, ctr.readUint8(csr++));
				}

				instPropCsr += ofs;
			}
			break;
		}
		case 0x30:	// SSG wave form
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint16_t ofs = ctr.readUint16(instPropCsr);
				size_t csr = instPropCsr + 2;

				uint16_t seqLen = ctr.readUint16(csr);
				csr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = ctr.readUint16(csr);
					csr += 2;
					int16_t subdata = ctr.readInt16(csr);
					csr += 2;
					if (l == 0)
						instMan.lock()->setWaveFormSSGSequenceCommand(idx, 0, data, subdata);
					else
						instMan.lock()->addWaveFormSSGSequenceCommand(idx, data, subdata);
				}

				uint16_t loopCnt = ctr.readUint16(csr);
				csr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(ctr.readUint16(csr));
						csr += 2;
						ends.push_back(ctr.readUint16(csr));
						csr += 2;
						times.push_back(ctr.readUint8(csr++));
					}
					instMan.lock()->setWaveFormSSGLoops(idx, begins, ends, times);
				}

				switch (ctr.readUint8(csr++)) {
				case 0x00:	// No release
					instMan.lock()->setWaveFormSSGRelease(
								idx, ReleaseType::NO_RELEASE, -1);
					break;
				case 0x01:	// Fix
					instMan.lock()->setWaveFormSSGRelease(
								idx, ReleaseType::FIX, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x02:	// Absolute
					instMan.lock()->setWaveFormSSGRelease(
								idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x03:	// Relative
					instMan.lock()->setWaveFormSSGRelease(
								idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
					csr += 2;
					break;
				}
				if (version >= Version::toBCD(1, 0, 1)) {
					++csr;	// Skip sequence type
				}

				instPropCsr += ofs;
			}
			break;
		}
		case 0x31:	// SSG tone/noise
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint16_t ofs = ctr.readUint16(instPropCsr);
				size_t csr = instPropCsr + 2;

				uint16_t seqLen = ctr.readUint16(csr);
				csr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = ctr.readUint16(csr);
					csr += 2;
					int16_t subdata = ctr.readInt16(csr);
					csr += 2;
					if (l == 0)
						instMan.lock()->setToneNoiseSSGSequenceCommand(idx, 0, data, subdata);
					else
						instMan.lock()->addToneNoiseSSGSequenceCommand(idx, data, subdata);
				}

				uint16_t loopCnt = ctr.readUint16(csr);
				csr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(ctr.readUint16(csr));
						csr += 2;
						ends.push_back(ctr.readUint16(csr));
						csr += 2;
						times.push_back(ctr.readUint8(csr++));
					}
					instMan.lock()->setToneNoiseSSGLoops(idx, begins, ends, times);
				}

				switch (ctr.readUint8(csr++)) {
				case 0x00:	// No release
					instMan.lock()->setToneNoiseSSGRelease(
								idx, ReleaseType::NO_RELEASE, -1);
					break;
				case 0x01:	// Fix
					instMan.lock()->setToneNoiseSSGRelease(
								idx, ReleaseType::FIX, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x02:	// Absolute
					instMan.lock()->setToneNoiseSSGRelease(
								idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x03:	// Relative
					instMan.lock()->setToneNoiseSSGRelease(
								idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
					csr += 2;
					break;
				}
				if (version >= Version::toBCD(1, 0, 1)) {
					++csr;	// Skip sequence type
				}

				instPropCsr += ofs;
			}
			break;
		}
		case 0x32:	// SSG envelope
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint16_t ofs = ctr.readUint16(instPropCsr);
				size_t csr = instPropCsr + 2;

				uint16_t seqLen = ctr.readUint16(csr);
				csr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = ctr.readUint16(csr);
					csr += 2;
					int16_t subdata = ctr.readInt16(csr);
					csr += 2;
					if (l == 0)
						instMan.lock()->setEnvelopeSSGSequenceCommand(idx, 0, data, subdata);
					else
						instMan.lock()->addEnvelopeSSGSequenceCommand(idx, data, subdata);
				}

				uint16_t loopCnt = ctr.readUint16(csr);
				csr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(ctr.readUint16(csr));
						csr += 2;
						ends.push_back(ctr.readUint16(csr));
						csr += 2;
						times.push_back(ctr.readUint8(csr++));
					}
					instMan.lock()->setEnvelopeSSGLoops(idx, begins, ends, times);
				}

				switch (ctr.readUint8(csr++)) {
				case 0x00:	// No release
					instMan.lock()->setEnvelopeSSGRelease(
								idx, ReleaseType::NO_RELEASE, -1);
					break;
				case 0x01:	// Fix
					instMan.lock()->setEnvelopeSSGRelease(
								idx, ReleaseType::FIX, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x02:	// Absolute
					instMan.lock()->setEnvelopeSSGRelease(
								idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x03:	// Relative
					instMan.lock()->setEnvelopeSSGRelease(
								idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
					csr += 2;
					break;
				}
				if (version >= Version::toBCD(1, 0, 1)) {
					++csr;	// Skip sequence type
				}

				instPropCsr += ofs;
			}
			break;
		}
		case 0x33:	// SSG arpeggio
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint16_t ofs = ctr.readUint16(instPropCsr);
				size_t csr = instPropCsr + 2;

				uint16_t seqLen = ctr.readUint16(csr);
				csr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = ctr.readUint16(csr);
					csr += 2;
					int16_t subdata = ctr.readInt16(csr);
					csr += 2;
					if (l == 0)
						instMan.lock()->setArpeggioSSGSequenceCommand(idx, 0, data, subdata);
					else
						instMan.lock()->addArpeggioSSGSequenceCommand(idx, data, subdata);
				}

				uint16_t loopCnt = ctr.readUint16(csr);
				csr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(ctr.readUint16(csr));
						csr += 2;
						ends.push_back(ctr.readUint16(csr));
						csr += 2;
						times.push_back(ctr.readUint8(csr++));
					}
					instMan.lock()->setArpeggioSSGLoops(idx, begins, ends, times);
				}

				switch (ctr.readUint8(csr++)) {
				case 0x00:	// No release
					instMan.lock()->setArpeggioSSGRelease(
								idx, ReleaseType::NO_RELEASE, -1);
					break;
				case 0x01:	// Fix
					instMan.lock()->setArpeggioSSGRelease(
								idx, ReleaseType::FIX, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x02:	// Absolute
					instMan.lock()->setArpeggioSSGRelease(
								idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x03:	// Relative
					instMan.lock()->setArpeggioSSGRelease(
								idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
					csr += 2;
					break;
				}
				if (version >= Version::toBCD(1, 0, 1)) {
					instMan.lock()->setArpeggioSSGType(idx, ctr.readUint8(csr++));
				}

				instPropCsr += ofs;
			}
			break;
		}
		case 0x34:	// SSG pitch
		{
			uint8_t cnt = ctr.readUint8(instPropCsr++);
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t idx = ctr.readUint8(instPropCsr++);
				uint16_t ofs = ctr.readUint16(instPropCsr);
				size_t csr = instPropCsr + 2;

				uint16_t seqLen = ctr.readUint16(csr);
				csr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = ctr.readUint16(csr);
					csr += 2;
					int16_t subdata = ctr.readInt16(csr);
					csr += 2;
					if (l == 0)
						instMan.lock()->setPitchSSGSequenceCommand(idx, 0, data, subdata);
					else
						instMan.lock()->addPitchSSGSequenceCommand(idx, data, subdata);
				}

				uint16_t loopCnt = ctr.readUint16(csr);
				csr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(ctr.readUint16(csr));
						csr += 2;
						ends.push_back(ctr.readUint16(csr));
						csr += 2;
						times.push_back(ctr.readUint8(csr++));
					}
					instMan.lock()->setPitchSSGLoops(idx, begins, ends, times);
				}

				switch (ctr.readUint8(csr++)) {
				case 0x00:	// No release
					instMan.lock()->setPitchSSGRelease(
								idx, ReleaseType::NO_RELEASE, -1);
					break;
				case 0x01:	// Fix
					instMan.lock()->setPitchSSGRelease(
								idx, ReleaseType::FIX, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x02:	// Absolute
					instMan.lock()->setPitchSSGRelease(
								idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
					csr += 2;
					break;
				case 0x03:	// Relative
					instMan.lock()->setPitchSSGRelease(
								idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
					csr += 2;
					break;
				}
				if (version >= Version::toBCD(1, 0, 1)) {
					instMan.lock()->setPitchSSGType(idx, ctr.readUint8(csr++));
				}

				instPropCsr += ofs;
			}
			break;
		}
		}
	}

	return globCsr;
}

size_t FileIO::loadInstrumentPropertyOperatorSequence(FMEnvelopeParameter param,
													  size_t instMemCsr,
													  std::weak_ptr<InstrumentsManager> instMan,
													  BinaryContainer& ctr, uint32_t version)
{
	uint8_t idx = ctr.readUint8(instMemCsr++);
	uint16_t ofs = ctr.readUint16(instMemCsr);
	size_t csr = instMemCsr + 2;

	uint16_t seqLen = ctr.readUint16(csr);
	csr += 2;
	for (uint16_t l = 0; l < seqLen; ++l) {
		uint16_t data = ctr.readUint16(csr);
		csr += 2;
		int16_t subdata = ctr.readInt16(csr);
		csr += 2;
		if (l == 0)
			instMan.lock()->setOperatorSequenceFMSequenceCommand(param, idx, 0, data, subdata);
		else
			instMan.lock()->addOperatorSequenceFMSequenceCommand(param, idx, data, subdata);
	}

	uint16_t loopCnt = ctr.readUint16(csr);
	csr += 2;
	if (loopCnt > 0) {
		std::vector<int> begins, ends, times;
		for (uint16_t l = 0; l < loopCnt; ++l) {
			begins.push_back(ctr.readUint16(csr));
			csr += 2;
			ends.push_back(ctr.readUint16(csr));
			csr += 2;
			times.push_back(ctr.readUint8(csr++));
		}
		instMan.lock()->setOperatorSequenceFMLoops(param, idx, begins, ends, times);
	}

	switch (ctr.readUint8(csr++)) {
	case 0x00:	// No release
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::NO_RELEASE, -1);
		break;
	case 0x01:	// Fix
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::FIX, ctr.readUint16(csr));
		csr += 2;
		break;
	case 0x02:	// Absolute
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
		csr += 2;
		break;
	case 0x03:	// Relative
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
		csr += 2;
		break;
	}

	if (version >= Version::toBCD(1, 0, 1)) {
		++csr;	// Skip sequence type
	}

	return ofs + 1;
}

size_t FileIO::loadGrooveSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr,
										 size_t globCsr, uint32_t version)
{
	size_t grvOfs = ctr.readUint32(globCsr);
	size_t grvCsr = globCsr + 4;
	uint8_t cnt = ctr.readUint8(grvCsr++) + 1;
	for (uint8_t i = 0; i < cnt; ++i) {
		uint8_t idx = ctr.readUint8(grvCsr++);
		uint8_t seqLen = ctr.readUint8(grvCsr++);
		std::vector<int> seq;
		for (uint8_t l = 0; l < seqLen; ++l) {
			seq.push_back(ctr.readUint8(grvCsr++));
		}

		if (idx > 0) mod.lock()->addGroove();
		mod.lock()->setGroove(idx, seq);
	}

	return globCsr + grvOfs;
}

size_t FileIO::loadSongSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr, size_t globCsr, uint32_t version)
{
	size_t songOfs = ctr.readUint32(globCsr);
	size_t songCsr = globCsr + 4;
	uint8_t cnt = ctr.readUint8(songCsr++);
	for (uint8_t i = 0; i < cnt; ++i) {
		uint8_t idx = ctr.readUint8(songCsr++);
		size_t sOfs = ctr.readUint32(songCsr);
		size_t scsr = songCsr + 4;
		songCsr += sOfs;
		size_t titleLen = ctr.readUint32(scsr);
		scsr += 4;
		std::string title = u8"";
		if (titleLen > 0) title = ctr.readString(scsr, titleLen);
		scsr += titleLen;
		uint32_t tempo = ctr.readUint32(scsr);
		scsr += 4;
		uint8_t groove = ctr.readUint8(scsr);
		scsr += 1;
		bool isTempo = (groove & 0x80) ? true : false;
		groove &= 0x7f;
		uint32_t speed = ctr.readUint32(scsr);
		scsr += 4;
		size_t ptnSize = ctr.readUint8(scsr) + 1;
		scsr += 1;
		switch (ctr.readUint8(scsr++)) {
		case 0x00:	// Standard
		{
			mod.lock()->addSong(idx, SongType::STD, title, isTempo, tempo, groove, speed, ptnSize);
			break;
		}
		}
		auto& song = mod.lock()->getSong(idx);
		while (scsr < songCsr) {
			// Song
			uint8_t trackIdx = ctr.readUint8(scsr++);
			auto& track = song.getTrack(trackIdx);
			size_t trackOfs = ctr.readUint32(scsr);
			size_t trackEnd = scsr + trackOfs;
			size_t tcsr = scsr + 4;
			uint8_t odrLen = ctr.readUint8(tcsr++) + 1;
			for (uint8_t oi = 0; oi < odrLen; ++oi) {
				if (!oi)
					track.registerPatternToOrder(oi, ctr.readUint8(tcsr++));
				else {
					track.insertOrderBelow(oi - 1);
					track.registerPatternToOrder(oi, ctr.readUint8(tcsr++));
				}
			}

			// Pattern
			while (tcsr < trackEnd) {
				uint8_t ptnIdx = ctr.readUint8(tcsr++);
				auto& pattern = track.getPattern(ptnIdx);
				size_t ptnOfs = ctr.readUint32(tcsr);
				size_t pcsr = tcsr + 4;
				tcsr += ptnOfs;

				// Step
				while (pcsr < tcsr) {
					uint32_t stepIdx = ctr.readUint8(pcsr++);
					auto& step = pattern.getStep(stepIdx);
					uint16_t eventFlag = ctr.readUint16(pcsr);
					pcsr += 2;
					if (eventFlag & 0x0001)	step.setNoteNumber(ctr.readInt8(pcsr++));
					if (eventFlag & 0x0002)	step.setInstrumentNumber(ctr.readUint8(pcsr++));
					if (eventFlag & 0x0004)	step.setVolume(ctr.readUint8(pcsr++));
					if (eventFlag & 0x0008)	{
						step.setEffectID(0, ctr.readString(pcsr, 2));
						pcsr += 2;
					}
					if (eventFlag & 0x0010)	step.setEffectValue(0, ctr.readUint8(pcsr++));
					if (eventFlag & 0x0020)	{
						step.setEffectID(1, ctr.readString(pcsr, 2));
						pcsr += 2;
					}
					if (eventFlag & 0x0040)	step.setEffectValue(1, ctr.readUint8(pcsr++));
					if (eventFlag & 0x0080)	{
						step.setEffectID(2, ctr.readString(pcsr, 2));
						pcsr += 2;
					}
					if (eventFlag & 0x0100)	step.setEffectValue(2, ctr.readUint8(pcsr++));
					if (eventFlag & 0x0200)	{
						step.setEffectID(3, ctr.readString(pcsr, 2));
						pcsr += 2;
					}
					if (eventFlag & 0x0400)	step.setEffectValue(3, ctr.readUint8(pcsr++));
				}
			}

			scsr += trackOfs;
		}
	}

	return globCsr + songOfs;
}

void FileIO::saveInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	BinaryContainer ctr;

	ctr.appendString("BambooTrackerIst");
	size_t eofOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy EOF offset
	uint32_t fileVersion = Version::ofInstrumentFileInBCD();
	ctr.appendUint32(fileVersion);


	/***** Instrument section *****/
	ctr.appendString("INSTRMNT");
	size_t instOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument section offset
	std::shared_ptr<AbstractInstrument> inst = instMan.lock()->getInstrumentSharedPtr(instNum);
	if (inst) {
		std::string name = inst->getName();
		ctr.appendUint32(name.length());
		if (!name.empty()) ctr.appendString(name);
		switch (inst->getSoundSource()) {
		case SoundSource::FM:
		{
			ctr.appendUint8(0x00);
			auto instFM = std::dynamic_pointer_cast<InstrumentFM>(inst);
			ctr.appendUint8(instFM->getEnvelopeResetEnabled());
			break;
		}
		case SoundSource::SSG:
		{
			ctr.appendUint8(0x01);
			break;
		}
		}
	}
	ctr.writeUint32(instOfs, ctr.size() - instOfs);


	/***** Instrument property section *****/
	ctr.appendString("INSTPROP");
	size_t instPropOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument property section offset

	switch (inst->getSoundSource()) {
	case SoundSource::FM:
	{
		auto instFM = std::dynamic_pointer_cast<InstrumentFM>(inst);

		// FM envelope
		int envNum = instFM->getEnvelopeNumber();
		{
			ctr.appendUint8(0x00);
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AL) << 4)
						  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::FB);
			ctr.appendUint8(tmp);
			// Operator 1
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 0);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR1);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS1) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR1);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT1) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR1);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL1) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR1);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL1);
			ctr.appendUint8(tmp);
			int tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG1);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML1);
			ctr.appendUint8(tmp);
			// Operator 2
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 1);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR2);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS2) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR2);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT2) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR2);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL2) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR2);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL2);
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG2);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML2);
			ctr.appendUint8(tmp);
			// Operator 3
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 2);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR3);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS3) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR3);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT3) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR3);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL3) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR3);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL3);
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG3);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML3);
			ctr.appendUint8(tmp);
			// Operator 4
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 3);
			tmp = (tmp << 5) | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR4);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS4) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR4);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT4) << 5)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR4);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL4) << 4)
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR4);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL4);
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG4);
			tmp = ((tmp2 == -1) ? 0x80 : (tmp2 << 4))
				  | instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML4);
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, ctr.size() - ofs);
		}

		// FM LFO
		if (instFM->getLFOEnabled()) {
			int lfoNum = instFM->getLFONumber();
			ctr.appendUint8(0x01);
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = (instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::FREQ) << 4)
						  | instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::PMS);
			ctr.appendUint8(tmp);
			tmp = (instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM4) << 7)
				  | (instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM3) << 6)
				  | (instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM2) << 5)
				  | (instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM1) << 4)
				  | instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AMS);
			ctr.appendUint8(tmp);
			tmp = instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::COUNT);
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, ctr.size() - ofs);
		}

		// FM envelope parameter
		for (size_t i = 0; i < 38; ++i) {
			if (instFM->getOperatorSequenceEnabled(ENV_FM_PARAMS[i])) {
				int seqNum = instFM->getOperatorSequenceNumber(ENV_FM_PARAMS[i]);
				ctr.appendUint8(0x02 + i);
				size_t ofs = ctr.size();
				ctr.appendUint16(0);	// Dummy offset
				auto seq = instMan.lock()->getOperatorSequenceFMSequence(ENV_FM_PARAMS[i], seqNum);
				ctr.appendUint16(seq.size());
				for (auto& com : seq) {
					ctr.appendUint16(com.type);
					ctr.appendInt16(com.data);
				}
				auto loop = instMan.lock()->getOperatorSequenceFMLoops(ENV_FM_PARAMS[i], seqNum);
				ctr.appendUint16(loop.size());
				for (auto& l : loop) {
					ctr.appendUint16(l.begin);
					ctr.appendUint16(l.end);
					ctr.appendUint8(l.times);
				}
				auto release = instMan.lock()->getOperatorSequenceFMRelease(ENV_FM_PARAMS[i], seqNum);
				switch (release.type) {
				case ReleaseType::NO_RELEASE:
					ctr.appendUint8(0x00);
					break;
				case ReleaseType::FIX:
					ctr.appendUint8(0x01);
					ctr.appendUint16(release.begin);
					break;
				case ReleaseType::ABSOLUTE:
					ctr.appendUint8(0x02);
					ctr.appendUint16(release.begin);
					break;
				case ReleaseType::RELATIVE:
					ctr.appendUint8(0x03);
					ctr.appendUint16(release.begin);
					break;
				}
				if (fileVersion >= Version::toBCD(1, 0, 1)) {
					ctr.appendUint8(0);	// Skip sequence type
				}
				ctr.writeUint16(ofs, ctr.size() - ofs);
			}
		}

		// FM arpeggio
		if (instFM->getArpeggioEnabled()) {
			int arpNum = instFM->getArpeggioNumber();
			ctr.appendUint8(0x28);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioFMSequence(arpNum);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getArpeggioFMLoops(arpNum);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getArpeggioFMRelease(arpNum);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getArpeggioFMType(arpNum));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}

		// FM pitch
		if (instFM->getPitchEnabled()) {
			int ptNum = instFM->getPitchNumber();
			ctr.appendUint8(0x29);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchFMSequence(ptNum);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getPitchFMLoops(ptNum);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getPitchFMRelease(ptNum);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getPitchFMType(ptNum));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
		break;
	}
	case SoundSource::SSG:
	{
		auto instSSG = std::dynamic_pointer_cast<InstrumentSSG>(inst);

		// SSG wave form
		if (instSSG->getWaveFormEnabled()) {
			int wfNum = instSSG->getWaveFormNumber();
			ctr.appendUint8(0x30);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getWaveFormSSGSequence(wfNum);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getWaveFormSSGLoops(wfNum);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getWaveFormSSGRelease(wfNum);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(0);	// Skip sequence type
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}

		// SSG tone/noise
		if (instSSG->getToneNoiseEnabled()) {
			int tnNum = instSSG->getToneNoiseNumber();
			ctr.appendUint8(0x31);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getToneNoiseSSGSequence(tnNum);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getToneNoiseSSGLoops(tnNum);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getToneNoiseSSGRelease(tnNum);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(0);	// Skip sequence type
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}

		// SSG envelope
		if (instSSG->getEnvelopeEnabled()) {
			int envNum = instSSG->getEnvelopeNumber();
			ctr.appendUint8(0x32);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getEnvelopeSSGSequence(envNum);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getEnvelopeSSGLoops(envNum);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getEnvelopeSSGRelease(envNum);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(0);	// Skip sequence type
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}

		// SSG arpeggio
		if (instSSG->getArpeggioEnabled()) {
			int arpNum = instSSG->getArpeggioNumber();
			ctr.appendUint8(0x33);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioSSGSequence(arpNum);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getArpeggioSSGLoops(arpNum);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getArpeggioSSGRelease(arpNum);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getArpeggioSSGType(arpNum));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}

		// SSG pitch
		if (instSSG->getPitchEnabled()) {
			int ptNum = instSSG->getPitchNumber();
			ctr.appendUint8(0x34);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchSSGSequence(ptNum);
			ctr.appendUint16(seq.size());
			for (auto& com : seq) {
				ctr.appendUint16(com.type);
				ctr.appendInt16(com.data);
			}
			auto loop = instMan.lock()->getPitchSSGLoops(ptNum);
			ctr.appendUint16(loop.size());
			for (auto& l : loop) {
				ctr.appendUint16(l.begin);
				ctr.appendUint16(l.end);
				ctr.appendUint8(l.times);
			}
			auto release = instMan.lock()->getPitchSSGRelease(ptNum);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(release.begin);
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(release.begin);
				break;
			}
			if (fileVersion >= Version::toBCD(1, 0, 1)) {
				ctr.appendUint8(instMan.lock()->getPitchSSGType(ptNum));
			}
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
		break;
	}
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);

	ctr.writeUint32(eofOfs, ctr.size() - eofOfs);

	if (!ctr.save(path)) throw FileOutputError(FileIOError::FileType::INST);
}

AbstractInstrument* FileIO::loadInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	std::string ext = path.substr(path.find_last_of(".")+1);
	for (auto& i : ext) i = std::tolower(i, std::locale());
	if (ext.compare("dmp") == 0) return FileIO::loadDMPFile(path, instMan, instNum);
	if (ext.compare("tfi") == 0) return FileIO::loadTFIFile(path, instMan, instNum);
	if (ext.compare("vgi") == 0) return FileIO::loadVGIFile(path, instMan, instNum);
	if (ext.compare("opni") == 0) return FileIO::loadOPNIFile(path, instMan, instNum);

	BinaryContainer ctr;

	if (!ctr.load(path)) throw FileInputError(FileIOError::FileType::INST);

	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerIst")
		throw FileCorruptionError(FileIOError::FileType::INST);
	globCsr += 16;
	size_t eofOfs = ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofInstrumentFileInBCD())
		throw FileVersionError(fileVersion, Version::ofApplicationInBCD(), FileIOError::FileType::INST);
	globCsr += 4;


	/***** Instrument section *****/
	if (ctr.readString(globCsr, 8) != "INSTRMNT")
		throw FileCorruptionError(FileIOError::FileType::INST);
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
		AbstractInstrument* inst = nullptr;
		switch (ctr.readUint8(instCsr++)) {
		case 0x00:	// FM
		{
			inst = new InstrumentFM(instNum, name, instMan.lock().get());
			dynamic_cast<InstrumentFM*>(inst)->setEnvelopeResetEnabled(
						ctr.readUint8(instCsr++) ? true : false);
			break;
		}
		case 0x01:	// SSG
		{
			inst = new InstrumentSSG(instNum, name, instMan.lock().get());
			break;
		}
		}
		globCsr += instOfs;


		/***** Instrument property section *****/
		if (ctr.readString(globCsr, 8) != "INSTPROP")
			throw FileCorruptionError(FileIOError::FileType::INST);
		else {
			globCsr += 8;
			size_t instPropOfs = ctr.readUint32(globCsr);
			size_t instPropCsr = globCsr + 4;
			size_t instPropCsrTmp = instPropCsr;
			globCsr += instPropOfs;
			std::vector<int> nums;
			// Check memory range
			while (instPropCsr < globCsr) {
				switch (ctr.readUint8(instPropCsr++)) {
				case 0x00:	// FM envelope
				{
					nums.push_back(instMan.lock()->findFirstFreeEnvelopeFM());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x01:	// FM LFO
				{
					nums.push_back(instMan.lock()->findFirstFreeLFOFM());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x02:	// FM AL
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::AL));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x03:	// FM FB
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::FB));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x04:	// FM AR1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::AR1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x05:	// FM DR1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DR1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x06:	// FM SR1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SR1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x07:	// FM RR1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::RR1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x08:	// FM SL1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SL1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x09:	// FM TL1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::TL1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0a:	// FM KS1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::KS1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0b:	// FM ML1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::ML1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0c:	// FM DT1
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DT1));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0d:	// FM AR2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::AR2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0e:	// FM DR2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DR2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0f:	// FM SR2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SR2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x10:	// FM RR2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::RR2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x11:	// FM SL2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SL2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x12:	// FM TL2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::TL2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x13:	// FM KS2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::KS2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x14:	// FM ML2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::ML2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x15:	// FM DT2
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DT2));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x16:	// FM AR3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::AR3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x17:	// FM DR3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DR3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x18:	// FM SR3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SR3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x19:	// FM RR3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::RR3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1a:	// FM SL3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SL3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1b:	// FM TL3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::TL3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1c:	// FM KS3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::KS3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1d:	// FM ML3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::ML3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1e:	// FM DT3
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DT3));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1f:	// FM AR4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::AR4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x20:	// FM DR4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DR4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x21:	// FM SR4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SR4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x22:	// FM RR4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::RR4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x23:	// FM SL4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::SL4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x24:	// FM TL4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::TL4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x25:	// FM KS4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::KS4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x26:	// FM ML4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::ML4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x27:	// FM DT4
				{
					nums.push_back(instMan.lock()->findFirstFreeOperatorSequenceFM(FMEnvelopeParameter::DT4));
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x28:	// FM arpeggio
				{
					nums.push_back(instMan.lock()->findFirstFreeArpeggioFM());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x29:	// FM pitch
				{
					nums.push_back(instMan.lock()->findFirstFreePitchFM());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x30:	// SSG wave form
				{
					nums.push_back(instMan.lock()->findFirstFreeWaveFormSSG());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x31:	// SSG tone/noise
				{
					nums.push_back(instMan.lock()->findFirstFreeToneNoiseSSG());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x32:	// SSG envelope
				{
					nums.push_back(instMan.lock()->findFirstFreeEnvelopeSSG());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x33:	// SSG arpeggio
				{
					nums.push_back(instMan.lock()->findFirstFreeArpeggioSSG());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x34:	// SSG pitch
				{
					nums.push_back(instMan.lock()->findFirstFreePitchSSG());
					if (nums.back() == -1) return nullptr;
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
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
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AL, tmp >> 4);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::FB, tmp & 0x0f);
					// Operator 1
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 0, (0x20 & tmp) ? true : false);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR1, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS1, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR1, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT1, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR1, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL1, tmp >> 4);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR1, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL1, tmp);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML1, tmp & 0x0f);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG1,
														   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
					// Operator 2
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 1, (0x20 & tmp) ? true : false);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR2, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS2, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR2, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT2, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR2, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL2, tmp >> 4);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR2, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL2, tmp);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML2, tmp & 0x0f);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG2,
														   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
					// Operator 3
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 2, (0x20 & tmp) ? true : false);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR3, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS3, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR3, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT3, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR3, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL3, tmp >> 4);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR3, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL3, tmp);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML3, tmp & 0x0f);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG3,
														   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
					// Operator 4
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMOperatorEnabled(idx, 3, (0x20 & tmp) ? true : false);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR4, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS4, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR4, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT4, tmp >> 5);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR4, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL4, tmp >> 4);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR4, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL4, tmp);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML4, tmp & 0x0f);
					instMan.lock()->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG4,
														   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
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
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::FREQ, tmp >> 4);
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::PMS, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AMS, tmp & 0x0f);
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM1, (tmp & 0x10) ? true : false);
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM2, (tmp & 0x20) ? true : false);
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM3, (tmp & 0x40) ? true : false);
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::AM4, (tmp & 0x80) ? true : false);
					tmp = ctr.readUint8(csr++);
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::COUNT, tmp);
					instPropCsr += ofs;
					break;
				}
				case 0x02:	// FM AL
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AL, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x03:	// FM FB
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::FB, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x04:	// FM AR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x05:	// FM DR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x06:	// FM SR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x07:	// FM RR1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x08:	// FM SL1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x09:	// FM TL1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0a:	// FM KS1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0b:	// FM ML1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0c:	// FM DT1
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT1, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0d:	// FM AR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0e:	// FM DR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x0f:	// FM SR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x10:	// FM RR2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x11:	// FM SL2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x12:	// FM TL2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x13:	// FM KS2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x14:	// FM ML2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x15:	// FM DT2
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT2, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x16:	// FM AR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x17:	// FM DR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x18:	// FM SR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x19:	// FM RR3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1a:	// FM SL3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1b:	// FM TL3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1c:	// FM KS3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1d:	// FM ML3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1e:	// FM DT3
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT3, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x1f:	// FM AR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::AR4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x20:	// FM DR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DR4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x21:	// FM SR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SR4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x22:	// FM RR4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::RR4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x23:	// FM SL4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::SL4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x24:	// FM TL4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::TL4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x25:	// FM KS4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::KS4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x26:	// FM ML4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::ML4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x27:	// FM DT4
				{
					instPropCsr += loadInstrumentPropertyOperatorSequenceForInstrument(
									   FMEnvelopeParameter::DT4, instPropCsr, instMan, ctr,
									   dynamic_cast<InstrumentFM*>(inst), *numIt++, fileVersion);
					break;
				}
				case 0x28:	// FM arpeggio
				{
					int idx = *numIt++;
					auto fm = dynamic_cast<InstrumentFM*>(inst);
					fm->setArpeggioEnabled(true);
					fm->setArpeggioNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						int16_t subdata = ctr.readInt16(csr);
						csr += 2;
						if (l == 0)
							instMan.lock()->setArpeggioFMSequenceCommand(idx, 0, data, subdata);
						else
							instMan.lock()->addArpeggioFMSequenceCommand(idx, data, subdata);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					if (loopCnt > 0) {
						std::vector<int> begins, ends, times;
						for (uint16_t l = 0; l < loopCnt; ++l) {
							begins.push_back(ctr.readUint16(csr));
							csr += 2;
							ends.push_back(ctr.readUint16(csr));
							csr += 2;
							times.push_back(ctr.readUint8(csr++));
						}
						instMan.lock()->setArpeggioFMLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::NO_RELEASE, -1);
						break;
					case 0x01:	// Fix
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::FIX, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x02:	// Absolute
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x03:	// Relative
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
						csr += 2;
						break;
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						instMan.lock()->setArpeggioFMType(idx, ctr.readUint8(csr++));
					}

					instPropCsr += ofs;
					break;
				}
				case 0x29:	// FM pitch
				{
					int idx = *numIt++;
					auto fm = dynamic_cast<InstrumentFM*>(inst);
					fm->setPitchEnabled(true);
					fm->setPitchNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						int16_t subdata = ctr.readInt16(csr);
						csr += 2;
						if (l == 0)
							instMan.lock()->setPitchFMSequenceCommand(idx, 0, data, subdata);
						else
							instMan.lock()->addPitchFMSequenceCommand(idx, data, subdata);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					if (loopCnt > 0) {
						std::vector<int> begins, ends, times;
						for (uint16_t l = 0; l < loopCnt; ++l) {
							begins.push_back(ctr.readUint16(csr));
							csr += 2;
							ends.push_back(ctr.readUint16(csr));
							csr += 2;
							times.push_back(ctr.readUint8(csr++));
						}
						instMan.lock()->setPitchFMLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::NO_RELEASE, -1);
						break;
					case 0x01:	// Fix
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::FIX, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x02:	// Absolute
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x03:	// Relative
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
						csr += 2;
						break;
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						instMan.lock()->setPitchFMType(idx, ctr.readUint8(csr++));
					}

					instPropCsr += ofs;
					break;
				}
				case 0x30:	// SSG wave form
				{
					int idx = *numIt++;
					auto ssg = dynamic_cast<InstrumentSSG*>(inst);
					ssg->setWaveFormEnabled(true);
					ssg->setWaveFormNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					size_t csr = instPropCsr + 2;

					uint16_t seqLen = ctr.readUint16(csr);
					csr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = ctr.readUint16(csr);
						csr += 2;
						int16_t subdata = ctr.readInt16(csr);
						csr += 2;
						if (l == 0)
							instMan.lock()->setWaveFormSSGSequenceCommand(idx, 0, data, subdata);
						else
							instMan.lock()->addWaveFormSSGSequenceCommand(idx, data, subdata);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					if (loopCnt > 0) {
						std::vector<int> begins, ends, times;
						for (uint16_t l = 0; l < loopCnt; ++l) {
							begins.push_back(ctr.readUint16(csr));
							csr += 2;
							ends.push_back(ctr.readUint16(csr));
							csr += 2;
							times.push_back(ctr.readUint8(csr++));
						}
						instMan.lock()->setWaveFormSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::NO_RELEASE, -1);
						break;
					case 0x01:	// Fix
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::FIX, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x02:	// Absolute
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x03:	// Relative
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
						csr += 2;
						break;
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
						int16_t subdata = ctr.readInt16(csr);
						csr += 2;
						if (l == 0)
							instMan.lock()->setToneNoiseSSGSequenceCommand(idx, 0, data, subdata);
						else
							instMan.lock()->addToneNoiseSSGSequenceCommand(idx, data, subdata);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					if (loopCnt > 0) {
						std::vector<int> begins, ends, times;
						for (uint16_t l = 0; l < loopCnt; ++l) {
							begins.push_back(ctr.readUint16(csr));
							csr += 2;
							ends.push_back(ctr.readUint16(csr));
							csr += 2;
							times.push_back(ctr.readUint8(csr++));
						}
						instMan.lock()->setToneNoiseSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::NO_RELEASE, -1);
						break;
					case 0x01:	// Fix
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::FIX, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x02:	// Absolute
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x03:	// Relative
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
						csr += 2;
						break;
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
						int16_t subdata = ctr.readInt16(csr);
						csr += 2;
						if (l == 0)
							instMan.lock()->setEnvelopeSSGSequenceCommand(idx, 0, data, subdata);
						else
							instMan.lock()->addEnvelopeSSGSequenceCommand(idx, data, subdata);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					if (loopCnt > 0) {
						std::vector<int> begins, ends, times;
						for (uint16_t l = 0; l < loopCnt; ++l) {
							begins.push_back(ctr.readUint16(csr));
							csr += 2;
							ends.push_back(ctr.readUint16(csr));
							csr += 2;
							times.push_back(ctr.readUint8(csr++));
						}
						instMan.lock()->setEnvelopeSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::NO_RELEASE, -1);
						break;
					case 0x01:	// Fix
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::FIX, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x02:	// Absolute
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x03:	// Relative
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
						csr += 2;
						break;
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
						int16_t subdata = ctr.readInt16(csr);
						csr += 2;
						if (l == 0)
							instMan.lock()->setArpeggioSSGSequenceCommand(idx, 0, data, subdata);
						else
							instMan.lock()->addArpeggioSSGSequenceCommand(idx, data, subdata);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					if (loopCnt > 0) {
						std::vector<int> begins, ends, times;
						for (uint16_t l = 0; l < loopCnt; ++l) {
							begins.push_back(ctr.readUint16(csr));
							csr += 2;
							ends.push_back(ctr.readUint16(csr));
							csr += 2;
							times.push_back(ctr.readUint8(csr++));
						}
						instMan.lock()->setArpeggioSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::NO_RELEASE, -1);
						break;
					case 0x01:	// Fix
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::FIX, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x02:	// Absolute
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x03:	// Relative
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
						csr += 2;
						break;
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						instMan.lock()->setArpeggioSSGType(idx, ctr.readUint8(csr++));
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
						int16_t subdata = ctr.readInt16(csr);
						csr += 2;
						if (l == 0)
							instMan.lock()->setPitchSSGSequenceCommand(idx, 0, data, subdata);
						else
							instMan.lock()->addPitchSSGSequenceCommand(idx, data, subdata);
					}

					uint16_t loopCnt = ctr.readUint16(csr);
					csr += 2;
					if (loopCnt > 0) {
						std::vector<int> begins, ends, times;
						for (uint16_t l = 0; l < loopCnt; ++l) {
							begins.push_back(ctr.readUint16(csr));
							csr += 2;
							ends.push_back(ctr.readUint16(csr));
							csr += 2;
							times.push_back(ctr.readUint8(csr++));
						}
						instMan.lock()->setPitchSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::NO_RELEASE, -1);
						break;
					case 0x01:	// Fix
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::FIX, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x02:	// Absolute
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
						csr += 2;
						break;
					case 0x03:	// Relative
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
						csr += 2;
						break;
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						instMan.lock()->setPitchSSGType(idx, ctr.readUint8(csr++));
					}

					instPropCsr += ofs;
					break;
				}
				}
			}
		}


		return inst;
	}
}

AbstractBank* FileIO::loadBank(std::string path)
{
	std::string ext = path.substr(path.find_last_of(".")+1);
	if (ext.compare("wopn") == 0) return FileIO::loadWOPNFile(path);
	throw FileInputError(FileIOError::FileType::BANK);
}

AbstractInstrument* FileIO::loadDMPFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIOError::FileType::INST);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	uint8_t insType = 1; // default to FM
	uint8_t fileVersion = ctr.readUint8(csr++);
	if (fileVersion == 0) { // older, unversioned dmp
		if (ctr.size() != 49) throw FileCorruptionError(FileIOError::FileType::INST);
	}
	else {
		if (fileVersion < 9) throw FileCorruptionError(FileIOError::FileType::INST);
		if (fileVersion == 9 && ctr.size() != 51) { // make sure it's not for that discontinued chip
			throw FileCorruptionError(FileIOError::FileType::INST);
		}
		uint8_t system = 2; // default to genesis
		if (fileVersion >= 11) system = ctr.readUint8(csr++);
		if (system != 2 && system != 3 && system != 8) { // genesis, sms and arcade only
			throw FileCorruptionError(FileIOError::FileType::INST);
		}
		insType = ctr.readUint8(csr++);
	}
	AbstractInstrument* inst = nullptr;
	switch (insType) {
	case 0x00:	// SSG
	{
		inst = new InstrumentSSG(instNum, name, instMan.lock().get());
		auto ssg = dynamic_cast<InstrumentSSG*>(inst);
		uint8_t envSize = ctr.readUint8(csr++);
		if (envSize > 0) {
			int idx = instMan.lock()->findFirstFreeEnvelopeSSG();
			if (idx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
			ssg->setEnvelopeEnabled(true);
			ssg->setEnvelopeNumber(idx);
			for (uint8_t l = 0; l < envSize; ++l) {
				int data = ctr.readInt32(csr);
				// compensate SN76489's envelope step of 2dB to SSG's 3dB
				if (data > 0) data = 15 - (15 - data) * 2 / 3;
				csr += 4;
				if (l == 0) instMan.lock()->setEnvelopeSSGSequenceCommand(idx, 0, data, 0);
				else instMan.lock()->addEnvelopeSSGSequenceCommand(idx, data, 0);
			}
			int8_t loop = ctr.readInt8(csr++);
			if (loop >= 0) instMan.lock()->setEnvelopeSSGLoops(idx, {loop}, {envSize - 1}, {1});
		}
		uint8_t arpSize = ctr.readUint8(csr++);
		if (arpSize > 0) {
			int idx = instMan.lock()->findFirstFreeArpeggioSSG();
			if (idx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
			ssg->setArpeggioEnabled(true);
			ssg->setArpeggioNumber(idx);
			uint8_t arpType = ctr.readUint8(csr + arpSize * 4 + 1);
			if (arpType == 1) instMan.lock()->setArpeggioSSGType(idx, 1);
			for (uint8_t l = 0; l < arpSize; ++l) {
				int data = ctr.readInt32(csr) + 36;
				csr += 4;
				if (arpType == 1) data -= 24;
				if (l == 0) instMan.lock()->setArpeggioSSGSequenceCommand(idx, 0, data, 0);
				else instMan.lock()->addArpeggioSSGSequenceCommand(idx, data, 0);
			}
			int8_t loop = ctr.readInt8(csr++);
			if (loop >= 0) instMan.lock()->setArpeggioSSGLoops(idx, {loop}, {arpSize - 1}, {1});
		}
		break;
	}
	case 0x01:	// FM
	{
		int envIdx = instMan.lock()->findFirstFreeEnvelopeFM();
		if (envIdx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
		inst = new InstrumentFM(instNum, name, instMan.lock().get());
		auto fm = dynamic_cast<InstrumentFM*>(inst);
		fm->setEnvelopeNumber(envIdx);
		if (fileVersion == 9) csr++; // skip version 9's total operators field
		uint8_t pms = ctr.readUint8(csr++);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
		uint8_t ams = ctr.readUint8(csr++);
		
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, ctr.readUint8(csr++));
		uint8_t am1 = ctr.readUint8(csr++);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, ctr.readUint8(csr++) & 15); // mask out OPM's DT2
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
		int ssgeg1 = ctr.readUint8(csr++);
		ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);

		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, ctr.readUint8(csr++));
		uint8_t am3 = ctr.readUint8(csr++);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, ctr.readUint8(csr++) & 15);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
		int ssgeg3 = ctr.readUint8(csr++);
		ssgeg3 = ssgeg3 & 8 ? ssgeg3 & 7 : -1;
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);
		
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, ctr.readUint8(csr++));
		uint8_t am2 = ctr.readUint8(csr++);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, ctr.readUint8(csr++) & 15);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
		int ssgeg2 = ctr.readUint8(csr++);
		ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);
		
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, ctr.readUint8(csr++));
		uint8_t am4 = ctr.readUint8(csr++);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, ctr.readUint8(csr++));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, ctr.readUint8(csr++) & 15);
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
		int ssgeg4 = ctr.readUint8(csr++);
		ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);

		if (pms || ams) {
			int lfoIdx = instMan.lock()->findFirstFreeLFOFM();
			if (lfoIdx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
			fm->setLFOEnabled(true);
			fm->setLFONumber(lfoIdx);
			instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, pms);
			instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, ams);
			instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, am1);
			instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, am2);
			instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, am3);
			instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, am4);
		}
		break;
	}
	}
	return inst;
}

AbstractInstrument* FileIO::loadTFIFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIOError::FileType::INST);
	if (ctr.size() != 42) throw FileCorruptionError(FileIOError::FileType::INST);
	int envIdx = instMan.lock()->findFirstFreeEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instMan.lock().get());
	inst->setEnvelopeNumber(envIdx);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, ctr.readUint8(csr++));
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, ctr.readUint8(csr++));
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg1 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, ctr.readUint8(csr++));
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, ctr.readUint8(csr++));
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);
	return inst;
}

AbstractInstrument* FileIO::loadVGIFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIOError::FileType::INST);
	if (ctr.size() != 43) throw FileCorruptionError(FileIOError::FileType::INST);
	int envIdx = instMan.lock()->findFirstFreeEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instMan.lock().get());
	inst->setEnvelopeNumber(envIdx);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
	uint8_t pams = ctr.readUint8(csr++);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, ctr.readUint8(csr++));
	uint8_t drams1 = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, drams1 & 31);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, ctr.readUint8(csr++));
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, ctr.readUint8(csr++));
	uint8_t drams3 = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, drams3 & 31);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, ctr.readUint8(csr++));
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg1 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, ctr.readUint8(csr++));
	uint8_t drams2 = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, drams2 & 31);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, ctr.readUint8(csr++));
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, ctr.readUint8(csr++));
	uint8_t drams4 = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, drams4 & 31);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, ctr.readUint8(csr++));
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);

	if (pams != 0) {
		int lfoIdx = instMan.lock()->findFirstFreeLFOFM();
		if (lfoIdx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
		inst->setLFOEnabled(true);
		inst->setLFONumber(lfoIdx);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, pams & 7);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, pams >> 4);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, drams1 >> 7);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, drams2 >> 7);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, drams3 >> 7);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, drams4 >> 7);
	}
	return inst;
}

AbstractInstrument* FileIO::loadOPNIFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	OPNIFile opni;

	std::ifstream in(path, std::ios::binary);
	in.seekg(0, std::ios::end);
	std::streampos size = in.tellg();

	if (!in)
		throw FileInputError(FileIOError::FileType::INST);
	else {
		std::unique_ptr<char[]> buf(new char[size]);
		in.seekg(0, std::ios::beg);
		if (!in.read(buf.get(), size) || in.gcount() != size)
			throw FileInputError(FileIOError::FileType::INST);
		if (WOPN_LoadInstFromMem(&opni, buf.get(), size) != 0)
			throw FileCorruptionError(FileIOError::FileType::INST);
	}

	return loadWOPNInstrument(opni.inst, instMan, instNum);
}

AbstractInstrument* FileIO::loadWOPNInstrument(const WOPNInstrument &srcInst,
											   std::weak_ptr<InstrumentsManager> instMan,
											   int instNum) {
	int envIdx = instMan.lock()->findFirstFreeEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
	const char *name = srcInst.inst_name;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instMan.lock().get());
	inst->setEnvelopeNumber(envIdx);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, srcInst.fbalg & 7);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, (srcInst.fbalg >> 3) & 7);

	const WOPNOperator *op[4] = {
		&srcInst.operators[0],
		&srcInst.operators[2],
		&srcInst.operators[1],
		&srcInst.operators[3],
	};

#define LOAD_OPERATOR(n)						\
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML##n, op[n - 1]->dtfm_30 & 15); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT##n, (op[n - 1]->dtfm_30 >> 4) & 7); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL##n, op[n - 1]->level_40); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS##n, op[n - 1]->rsatk_50 >> 6); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR##n, op[n - 1]->rsatk_50 & 31); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR##n, op[n - 1]->amdecay1_60 & 31); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR##n, op[n - 1]->decay2_70 & 31); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR##n, op[n - 1]->susrel_80 & 15); \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL##n, op[n - 1]->susrel_80 >> 4); \
	int ssgeg##n = op[n - 1]->ssgeg_90; \
	ssgeg##n = ssgeg##n & 8 ? ssgeg##n & 7 : -1; \
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG##n, ssgeg##n); \
	int am##n = op[n - 1]->amdecay1_60 >> 7;

	LOAD_OPERATOR(1);
	LOAD_OPERATOR(2);
	LOAD_OPERATOR(3);
	LOAD_OPERATOR(4);

#undef LOAD_OPERATOR

	if (srcInst.lfosens != 0) {
		int lfoIdx = instMan.lock()->findFirstFreeLFOFM();
		if (lfoIdx < 0) throw FileCorruptionError(FileIOError::FileType::INST);
		inst->setLFOEnabled(true);
		inst->setLFONumber(lfoIdx);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, srcInst.lfosens & 7);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, (srcInst.lfosens >> 4) & 3);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, am1);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, am2);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, am3);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, am4);
	}

	return inst;
}

AbstractBank* FileIO::loadWOPNFile(std::string path)
{
	struct WOPNDeleter {
		void operator()(WOPNFile *x) { WOPN_Free(x); }
	};

	std::unique_ptr<WOPNFile, WOPNDeleter> wopn;

	std::ifstream in(path, std::ios::binary);
	in.seekg(0, std::ios::end);
	std::streampos size = in.tellg();

	if (!in)
		throw FileInputError(FileIOError::FileType::BANK);
	else {
		std::unique_ptr<char[]> buf(new char[size]);
		in.seekg(0, std::ios::beg);
		if (!in.read(buf.get(), size) || in.gcount() != size)
			throw FileInputError(FileIOError::FileType::BANK);
		wopn.reset(WOPN_LoadBankFromMem(buf.get(), size, nullptr));
		if (!wopn)
			throw FileCorruptionError(FileIOError::FileType::BANK);
	}

	WopnBank *bank = new WopnBank(wopn.get());
	wopn.release();
	return bank;
}

size_t FileIO::loadInstrumentPropertyOperatorSequenceForInstrument(FMEnvelopeParameter param,
																 size_t instMemCsr,
																 std::weak_ptr<InstrumentsManager> instMan,
																 BinaryContainer& ctr,
																 InstrumentFM* inst,
																 int idx,
																 uint32_t version)
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
		int16_t subdata = ctr.readInt16(csr);
		csr += 2;
		if (l == 0)
			instMan.lock()->setOperatorSequenceFMSequenceCommand(param, idx, 0, data, subdata);
		else
			instMan.lock()->addOperatorSequenceFMSequenceCommand(param, idx, data, subdata);
	}

	uint16_t loopCnt = ctr.readUint16(csr);
	csr += 2;
	if (loopCnt > 0) {
		std::vector<int> begins, ends, times;
		for (uint16_t l = 0; l < loopCnt; ++l) {
			begins.push_back(ctr.readUint16(csr));
			csr += 2;
			ends.push_back(ctr.readUint16(csr));
			csr += 2;
			times.push_back(ctr.readUint8(csr++));
		}
		instMan.lock()->setOperatorSequenceFMLoops(param, idx, begins, ends, times);
	}

	switch (ctr.readUint8(csr++)) {
	case 0x00:	// No release
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::NO_RELEASE, -1);
		break;
	case 0x01:	// Fix
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::FIX, ctr.readUint16(csr));
		csr += 2;
		break;
	case 0x02:	// Absolute
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::ABSOLUTE, ctr.readUint16(csr));
		csr += 2;
		break;
	case 0x03:	// Relative
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::RELATIVE, ctr.readUint16(csr));
		csr += 2;
		break;
	}

	if (version >= Version::toBCD(1, 0, 1)) {
		++csr;	// Skip sequence type
	}

	return ofs;
}

void FileIO::writeWave(std::string path, std::vector<int16_t> samples, uint32_t sampRate)
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
		throw FileOutputError(FileIOError::FileType::WAV);
	}
}

void FileIO::writeVgm(std::string path, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
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
		ofs.write("Vgm ", 4);
		// 0x04: EOF offset
		uint32_t offset = 0x100 + samples.size() + 1 + tagLen - 4;
		ofs.write(reinterpret_cast<char*>(&offset), 4);
		// 0x08: Version [v1.71]
		uint32_t version = 0x171;
		ofs.write(reinterpret_cast<char*>(&version), 4);
		// 0x0c-0x03: Unused
		uint32_t zero = 0;
		for (int i = 0; i < 2; ++i) ofs.write(reinterpret_cast<char*>(&zero), 4);
		// 0x14: GD3 offset
		uint32_t gd3Offset = gd3TagEnabled ? (0x100 + samples.size() + 1 - 0x14) : 0;
		ofs.write(reinterpret_cast<char*>(&gd3Offset), 4);
		// 0x18: Total # samples
		ofs.write(reinterpret_cast<char*>(&totalSamples), 4);
		// 0x1c: Loop offset
		uint32_t loopOffset = loopFlag ? (loopPoint + 0x100 - 0x1c) : 0;
		ofs.write(reinterpret_cast<char*>(&loopOffset), 4);
		// 0x20: Loop # samples
		uint32_t loopSamps = loopFlag ? loopSamples : 0;
		ofs.write(reinterpret_cast<char*>(&loopSamps), 4);
		// 0x24: Rate
		ofs.write(reinterpret_cast<char*>(&rate), 4);
		// 0x28-0x33: Unused
		for (int i = 0; i < 3; ++i) ofs.write(reinterpret_cast<char*>(&zero), 4);
		// 0x34: VGM data offset
		uint32_t dataOffset = 0xcc;
		ofs.write(reinterpret_cast<char*>(&dataOffset), 4);
		// 0x38-0x47: Unused
		for (int i = 0; i < 4; ++i) ofs.write(reinterpret_cast<char*>(&zero), 4);
		// 0x48: YM2608 clock
		ofs.write(reinterpret_cast<char*>(&clock), 4);
		// 0x4c-0xff: Unused
		for (int i = 0; i < 45; ++i) ofs.write(reinterpret_cast<char*>(&zero), 4);

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
		throw FileOutputError(FileIOError::FileType::VGM);
	}
}

void FileIO::writeS98(std::string path, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
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
		ofs.write(reinterpret_cast<char*>(&deviceCnt), 4);
		// 0x20-0x2f: Device info
		// 0x20: Device type
		uint32_t deviceType = 4;	// OPNA
		ofs.write(reinterpret_cast<char*>(&deviceType), 4);
		// 0x24: Clock
		ofs.write(reinterpret_cast<char*>(&clock), 4);
		// 0x28: Pan (Unused)
		ofs.write(reinterpret_cast<char*>(&zero), 4);
		// 0x28: Reserved
		ofs.write(reinterpret_cast<char*>(&zero), 4);
		// 0x30-0x7f: Unused
		for (int i = 0; i < 20; ++i) ofs.write(reinterpret_cast<char*>(&zero), 4);

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
		throw FileOutputError(FileIOError::FileType::S98);
	}
}

void FileIO::backupModule(std::string path)
{
	try {
		std::ifstream ifs(path, std::ios::binary);
		std::ofstream ofs(path + ".bak", std::ios::binary);
		ofs << ifs.rdbuf();
	} catch (...) {
		throw FileOutputError(FileIOError::FileType::MOD);
	}
}
