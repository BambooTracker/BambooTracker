#include "file_io.hpp"
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
	ctr.appendString("INSTMEM ");
	size_t instMemOfs = ctr.size();
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

	ctr.writeUint32(instMemOfs, ctr.size() - instMemOfs);


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
							ctr.appendUint16(tmp);
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
	if (ctr.readString(globCsr, 8) != "INSTMEM ") return false;
	else {
		globCsr += 8;
		size_t instMemOfs = ctr.readUint32(globCsr);
		size_t instMemCsr = globCsr + 4;
		globCsr += instMemOfs;
		while (instMemCsr < globCsr) {
			switch (ctr.readUint8(instMemCsr++)) {
			case 0x00:	// FM envelope
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
					uint8_t idx = ctr.readUint8(instMemCsr++);
					uint8_t ofs = ctr.readUint8(instMemCsr);
					uint8_t csr = instMemCsr + 1;
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
					instMemCsr += ofs;
				}
				break;
			}
			case 0x01:	// FM LFO
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
					uint8_t idx = ctr.readUint8(instMemCsr++);
					uint8_t ofs = ctr.readUint8(instMemCsr);
					uint8_t csr = instMemCsr + 1;
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
					instMemCsr += ofs;
				}
				break;
			}
			case 0x02:	// FM AL
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AL, instMemCsr, instMan, ctr);
				break;
			}
			case 0x03:	// FM FB
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::FB, instMemCsr, instMan, ctr);
				break;
			}
			case 0x04:	// FM AR1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x05:	// FM DR1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x06:	// FM SR1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x07:	// FM RR1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x08:	// FM SL1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x09:	// FM TL1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x0a:	// FM KS1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x0b:	// FM ML1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x0c:	// FM DT1
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT1, instMemCsr, instMan, ctr);
				break;
			}
			case 0x0d:	// FM AR2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x0e:	// FM DR2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x0f:	// FM SR2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x10:	// FM RR2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x11:	// FM SL2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x12:	// FM TL2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x13:	// FM KS2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x14:	// FM ML2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x15:	// FM DT2
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT2, instMemCsr, instMan, ctr);
				break;
			}
			case 0x16:	// FM AR3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x17:	// FM DR3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x18:	// FM SR3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x19:	// FM RR3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x1a:	// FM SL3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x1b:	// FM TL3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x1c:	// FM KS3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x1d:	// FM ML3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x1e:	// FM DT3
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT3, instMemCsr, instMan, ctr);
				break;
			}
			case 0x1f:	// FM AR4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::AR4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x20:	// FM DR4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DR4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x21:	// FM SR4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SR4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x22:	// FM RR4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::RR4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x23:	// FM SL4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::SL4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x24:	// FM TL4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::TL4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x25:	// FM KS4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::KS4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x26:	// FM ML4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::ML4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x27:	// FM DT4
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i)
					instMemCsr += loadInstrumentMemoryOperatorSequence(
									  FMEnvelopeParameter::DT4, instMemCsr, instMan, ctr);
				break;
			}
			case 0x28:	// FM arpeggio
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
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

					instMemCsr += ofs;
				}
				break;
			}
			case 0x29:	// FM pitch
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
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

					instMemCsr += ofs;
				}
				break;
			}
			case 0x30:	// SSG wave form
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
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

					instMemCsr += ofs;
				}
				break;
			}
			case 0x31:	// SSG tone/noise
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
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

					instMemCsr += ofs;
				}
				break;
			}
			case 0x32:	// SSG envelope
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
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

					instMemCsr += ofs;
				}
				break;
			}
			case 0x33:	// SSG arpeggio
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
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

					instMemCsr += ofs;
				}
				break;
			}
			case 0x34:	// SSG pitch
			{
				uint8_t cnt = ctr.readUint8(instMemCsr++);
				for (size_t i = 0; i < cnt; ++i) {
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

					instMemCsr += ofs;
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
			auto& song = mod.lock()->getSong(idx);
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
