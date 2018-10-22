#include "file_io.hpp"
#include "version.hpp"
#include "binary_container.hpp"
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

	ctr.appendString("BambooTrackerMod ");
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
				int tmp = instFM->getLFONumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				for (auto& param : ENV_FM_PARAMS) {
					tmp = instFM->getOperatorSequenceNumber(param);
					ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				}
				tmp = instFM->getArpeggioNumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				tmp = instFM->getPitchNumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				ctr.appendUint8(instFM->getEnvelopeResetEnabled());
				break;
			}
			case SoundSource::SSG:
			{
				ctr.appendUint8(0x01);
				auto instSSG = std::dynamic_pointer_cast<InstrumentSSG>(inst);
				int tmp = instSSG->getWaveFormNumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				tmp = instSSG->getToneNoiseNumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				tmp = instSSG->getEnvelopeNumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				tmp = instSSG->getArpeggioNumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
				tmp = instSSG->getPitchNumber();
				ctr.appendUint8((tmp == -1) ? 0x80 : tmp);
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


	/***** Instrument section *****/
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


	return ctr.save(path);
}

