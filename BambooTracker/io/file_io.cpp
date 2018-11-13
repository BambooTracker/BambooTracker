#include "file_io.hpp"
#include <fstream>
#include "version.hpp"
#include "misc.hpp"

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

bool FileIO::saveModule(std::string path, std::weak_ptr<Module> mod,
						std::weak_ptr<InstrumentsManager> instMan)
{
	BinaryContainer ctr;

	ctr.appendString("BambooTrackerMod");
	ctr.appendUint32(0);	// Dummy EOF offset
	ctr.appendUint32(Version::ofModuleFileInBCD());


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
	ctr.writeUint8(modOfs, ctr.size() - modOfs);


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


	/***** Instrument memory section *****/
	ctr.appendString("INSTPROP");
	size_t instPropOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument memory section offset

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
					ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);


	/***** Groove section *****/
	ctr.appendString("GROOVE  ");
	size_t grooveOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy groove section offset
	size_t grooveCnt = mod.lock()->getGrooveCount();
	ctr.appendUint8(grooveCnt);
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
		ctr.appendUint8(sng.getGroove());
		ctr.appendUint8(sng.isUsedTempo());
		ctr.appendUint32(sng.getSpeed());
		ctr.appendUint32(sng.getDefaultPatternSize());
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
			ctr.appendUint8(odrSize);
			for (size_t o = 0; o < odrSize; ++o)
				ctr.appendUint8(track.getOrderData(o).patten);

			// Pattern
			std::vector<int> ptnIdcs = track.getEditedPatternIndices();
			ctr.appendUint8(ptnIdcs.size());
			for (auto& idx : ptnIdcs) {
				ctr.appendUint8(idx);
				size_t ptnOfs = ctr.size();
				ctr.appendUint32(0);	// Dummy pattern subblock offset
				auto& pattern = track.getPattern(idx);

				// Step
				std::vector<int> stepIdcs = pattern.getEditedStepIndices();
				ctr.appendUint32(stepIdcs.size());
				for (auto& sidx : stepIdcs) {
					ctr.appendUint32(sidx);
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


	mod.lock()->setFilePath(path);


	return ctr.save(path);
}

bool FileIO::loadModuel(std::string path, std::weak_ptr<Module> mod,
						std::weak_ptr<InstrumentsManager> instMan)
{
	BinaryContainer ctr;

	if (!ctr.load(path)) return false;

	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerMod") return false;
	globCsr += 16;
	size_t eofOfs = ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	globCsr += 4;

	/***** Module section *****/
	if (ctr.readString(globCsr, 8) != "MODULE  ") return false;
	else {
		globCsr += 8;
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
		globCsr += modOfs;
	}


	/***** Instrument section *****/
	if (ctr.readString(globCsr, 8) != "INSTRMNT") return false;
	else {
		globCsr += 8;
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
				instFM->setEnvelopeResetEnabled(tmp ? false : true);
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
		globCsr += instOfs;
	}


	/***** Instrument memory section *****/
	if (ctr.readString(globCsr, 8) != "INSTPROP") return false;
	else {
		globCsr += 8;
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
					uint8_t csr = instPropCsr + 1;
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
					uint8_t csr = instPropCsr + 1;
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
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AL, instPropCsr, instMan, ctr);
				break;
			}
			case 0x03:	// FM FB
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::FB, instPropCsr, instMan, ctr);
				break;
			}
			case 0x04:	// FM AR1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x05:	// FM DR1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x06:	// FM SR1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x07:	// FM RR1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x08:	// FM SL1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x09:	// FM TL1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x0a:	// FM KS1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x0b:	// FM ML1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x0c:	// FM DT1
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT1, instPropCsr, instMan, ctr);
				break;
			}
			case 0x0d:	// FM AR2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x0e:	// FM DR2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x0f:	// FM SR2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x10:	// FM RR2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x11:	// FM SL2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x12:	// FM TL2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x13:	// FM KS2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x14:	// FM ML2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x15:	// FM DT2
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT2, instPropCsr, instMan, ctr);
				break;
			}
			case 0x16:	// FM AR3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x17:	// FM DR3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x18:	// FM SR3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x19:	// FM RR3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x1a:	// FM SL3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x1b:	// FM TL3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x1c:	// FM KS3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x1d:	// FM ML3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x1e:	// FM DT3
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT3, instPropCsr, instMan, ctr);
				break;
			}
			case 0x1f:	// FM AR4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x20:	// FM DR4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x21:	// FM SR4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x22:	// FM RR4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x23:	// FM SL4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x24:	// FM TL4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x25:	// FM KS4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x26:	// FM ML4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x27:	// FM DT4
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instPropCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT4, instPropCsr, instMan, ctr);
				break;
			}
			case 0x28:	// FM arpeggio
			{
				uint8_t cnt = ctr.readUint8(instPropCsr++);
				for (size_t i = 0; i < cnt; ++i) {
					uint8_t idx = ctr.readUint8(instPropCsr++);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
					}

					instPropCsr += ofs;
				}
				break;
			}
			}
		}
	}


	/***** Groove section *****/
	if (ctr.readString(globCsr, 8) != "GROOVE  ") return false;
	else {
		globCsr += 8;
		size_t grvOfs = ctr.readUint32(globCsr);
		size_t grvCsr = globCsr + 4;
		uint8_t cnt = ctr.readUint8(grvCsr++);
		for (uint8_t i = 0; i < cnt; ++i) {
			uint8_t idx = ctr.readUint8(grvCsr++);
			uint8_t seqLen = ctr.readUint8(grvCsr++);
			std::vector<int> seq;
			for (uint8_t l = 0; l < seqLen; ++l) {
				seq.push_back(ctr.readUint8(grvCsr++));
			}
			mod.lock()->setGroove(idx, seq);
		}
		globCsr += grvOfs;
	}


	/***** Song section *****/
	if (ctr.readString(globCsr, 8) != "SONG    ") return false;
	else {
		globCsr += 8;
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
			bool isTempo = ctr.readUint8(scsr) ? true : false;
			scsr += 1;
			uint32_t speed = ctr.readUint32(scsr);
			scsr += 4;
			size_t ptnSize =ctr.readUint32(scsr);
			scsr += 4;
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
				size_t tcsr = scsr + 4;
				uint8_t odrLen = ctr.readUint8(tcsr++);
				for (uint8_t oi = 0; oi < odrLen; ++oi) {
					if (!oi)
						track.registerPatternToOrder(oi, ctr.readUint8(tcsr++));
					else {
						track.insertOrderBelow(oi - 1);
						track.registerPatternToOrder(oi, ctr.readUint8(tcsr++));
					}
				}

				// Pattern
				uint8_t ptnCnt = ctr.readUint8(tcsr++);
				for (uint8_t pi = 0; pi < ptnCnt; ++pi) {
					uint8_t ptnIdx = ctr.readUint8(tcsr++);
					auto& pattern = track.getPattern(ptnIdx);
					size_t ptnOfs = ctr.readUint32(tcsr);
					size_t pcsr = tcsr + 4;

					// Step
					size_t stepCnt = ctr.readUint32(pcsr);
					pcsr += 4;
					for (size_t si = 0; si < stepCnt; ++si) {
						uint32_t stepIdx = ctr.readUint32(pcsr);
						pcsr += 4;
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

					tcsr += ptnOfs;
				}

				scsr += trackOfs;
			}
		}
		globCsr += songOfs;
	}


	mod.lock()->setFilePath(path);


	return true;
}

size_t FileIO::loadInstrumentMemoryOperatorSequence(FMEnvelopeParameter param,
		size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan, BinaryContainer& ctr)
{
	uint8_t idx = ctr.readUint8(instMemCsr++);
	uint16_t ofs = ctr.readUint16(instMemCsr);
	uint8_t csr = instMemCsr + 2;

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
					param, idx, ReleaseType::FIX, ctr.readUint8(csr++));
		break;
	case 0x02:	// Absolute
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
		break;
	case 0x03:	// Relative
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
		break;
	}

	return ofs;
}

bool FileIO::saveInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	BinaryContainer ctr;

	ctr.appendString("BambooTrackerIst");
	ctr.appendUint32(0);	// Dummy EOF offset
	ctr.appendUint32(Version::ofInstrumentFileInBCD());


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


	/***** Instrument memory section *****/
	ctr.appendString("INSTPROP");
	size_t instPropOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument memory section offset

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
					ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
				ctr.appendUint16(l.times);
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
			ctr.writeUint16(ofs, ctr.size() - ofs);
		}
		break;
	}
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);

	return ctr.save(path);
}

AbstractInstrument* FileIO::loadInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	BinaryContainer ctr;

	if (!ctr.load(path)) return nullptr;

	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerIst") return nullptr;
	globCsr += 16;
	size_t eofOfs = ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	globCsr += 4;


	/***** Instrument section *****/
	if (ctr.readString(globCsr, 8) != "INSTRMNT") return nullptr;
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


		/***** Instrument memory section *****/
		if (ctr.readString(globCsr, 8) != "INSTPROP") return nullptr;
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
					uint8_t csr = instPropCsr + 1;
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
					uint8_t csr = instPropCsr + 1;
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
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::AL, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x03:	// FM FB
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::FB, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x04:	// FM AR1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::AR1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x05:	// FM DR1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DR1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x06:	// FM SR1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SR1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x07:	// FM RR1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::RR1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x08:	// FM SL1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SL1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x09:	// FM TL1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::TL1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x0a:	// FM KS1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::KS1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x0b:	// FM ML1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::ML1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x0c:	// FM DT1
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DT1, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x0d:	// FM AR2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::AR2, instPropCsr, instMan,
									  ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x0e:	// FM DR2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DR2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x0f:	// FM SR2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SR2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x10:	// FM RR2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::RR2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x11:	// FM SL2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SL2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x12:	// FM TL2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::TL2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x13:	// FM KS2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::KS2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x14:	// FM ML2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::ML2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x15:	// FM DT2
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DT2, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x16:	// FM AR3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::AR3, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x17:	// FM DR3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DR3, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x18:	// FM SR3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SR3, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x19:	// FM RR3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::RR3, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x1a:	// FM SL3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SL3, instPropCsr, instMan, ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x1b:	// FM TL3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::TL3, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x1c:	// FM KS3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::KS3, instPropCsr, instMan, ctr, dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x1d:	// FM ML3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::ML3, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x1e:	// FM DT3
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DT3, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x1f:	// FM AR4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::AR4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x20:	// FM DR4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DR4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x21:	// FM SR4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SR4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x22:	// FM RR4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::RR4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x23:	// FM SL4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::SL4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x24:	// FM TL4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::TL4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x25:	// FM KS4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::KS4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x26:	// FM ML4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::ML4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x27:	// FM DT4
				{
					instPropCsr += loadInstrumentMemoryOperatorSequenceForInstrument(
									  FMEnvelopeParameter::DT4, instPropCsr, instMan, ctr,
									  dynamic_cast<InstrumentFM*>(inst), *numIt++);
					break;
				}
				case 0x28:	// FM arpeggio
				{
					int idx = *numIt++;
					auto fm = dynamic_cast<InstrumentFM*>(inst);
					fm->setArpeggioEnabled(true);
					fm->setArpeggioNumber(idx);
					uint16_t ofs = ctr.readUint16(instPropCsr);
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setArpeggioFMRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setPitchFMRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setWaveFormSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setToneNoiseSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setEnvelopeSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setArpeggioSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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
					uint8_t csr = instPropCsr + 2;

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
									idx, ReleaseType::FIX, ctr.readUint8(csr++));
						break;
					case 0x02:	// Absolute
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
						break;
					case 0x03:	// Relative
						instMan.lock()->setPitchSSGRelease(
									idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
						break;
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

size_t FileIO::loadInstrumentMemoryOperatorSequenceForInstrument(FMEnvelopeParameter param,
																 size_t instMemCsr,
																 std::weak_ptr<InstrumentsManager> instMan,
																 BinaryContainer& ctr,
																 InstrumentFM* inst,
																 int idx)
{
	inst->setOperatorSequenceEnabled(param, true);
	inst->setOperatorSequenceNumber(param, idx);
	uint16_t ofs = ctr.readUint16(instMemCsr);
	uint8_t csr = instMemCsr + 2;

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
					param, idx, ReleaseType::FIX, ctr.readUint8(csr++));
		break;
	case 0x02:	// Absolute
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::ABSOLUTE, ctr.readUint8(csr++));
		break;
	case 0x03:	// Relative
		instMan.lock()->setOperatorSequenceFMRelease(
					param, idx, ReleaseType::RELATIVE, ctr.readUint8(csr++));
		break;
	}

	return ofs;
}

bool FileIO::writeWave(std::string path, std::vector<int16_t> samples, uint32_t sampRate)
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

		return true;
	}
	catch (...) {
		return false;
	}
}

bool FileIO::writeVgm(std::string path, std::vector<uint8_t> samples, uint32_t clock, uint32_t rate,
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

		return true;
	} catch (...) {
		return false;
	}
}

bool FileIO::backupModule(std::string path)
{
	try {
		std::ifstream ifs(path, std::ios::binary);
		std::ofstream ofs(path + ".bak", std::ios::binary);
		ofs << ifs.rdbuf();
		return true;
	} catch (...) {
		return false;
	}
}
