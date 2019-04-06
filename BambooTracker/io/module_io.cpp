#include "module_io.hpp"
#include <fstream>
#include "version.hpp"
#include "file_io_error.hpp"
#include "file_io.hpp"
#include "pitch_converter.hpp"

ModuleIO::ModuleIO() {}

void ModuleIO::saveModule(std::string path, std::weak_ptr<Module> mod,
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
	ctr.appendUint32(mod.lock()->getStepHighlight1Distance());
	ctr.appendUint32(mod.lock()->getStepHighlight2Distance());
	ctr.writeUint32(modOfs, ctr.size() - modOfs);


	/***** Instrument section *****/
	ctr.appendString("INSTRMNT");
	size_t instOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy instrument section offset
	std::vector<int> instIdcs = instMan.lock()->getEntriedInstrumentIndices();
	ctr.appendUint8(static_cast<uint8_t>(instIdcs.size()));
	for (auto& idx : instIdcs) {
		if (std::shared_ptr<AbstractInstrument> inst = instMan.lock()->getInstrumentSharedPtr(idx)) {
			ctr.appendUint8(static_cast<uint8_t>(inst->getNumber()));
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
				ctr.appendUint8(static_cast<uint8_t>(instFM->getEnvelopeNumber()));
				uint8_t tmp = static_cast<uint8_t>(instFM->getLFONumber());
				ctr.appendUint8(instFM->getLFOEnabled() ? tmp : (0x80 | tmp));
				for (auto& param : FileIO::ENV_FM_PARAMS) {
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
				for (auto& type : FileIO::OP_FM_TYPES) {
					tmp = static_cast<uint8_t>(instFM->getArpeggioNumber(type));
					ctr.appendUint8(instFM->getArpeggioEnabled(type) ? tmp : (0x80 | tmp));
				}
				for (auto& type : FileIO::OP_FM_TYPES) {
					tmp = static_cast<uint8_t>(instFM->getPitchNumber(type));
					ctr.appendUint8(instFM->getPitchEnabled(type) ? tmp : (0x80 | tmp));
				}
				break;
			}
			case SoundSource::SSG:
			{
				ctr.appendUint8(0x01);
				auto instSSG = std::dynamic_pointer_cast<InstrumentSSG>(inst);
				uint8_t tmp = static_cast<uint8_t>(instSSG->getWaveFormNumber());
				ctr.appendUint8(instSSG->getWaveFormEnabled() ? tmp : (0x80 | tmp));
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
			case SoundSource::DRUM:
				break;
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
		ctr.appendUint8(static_cast<uint8_t>(envFMIdcs.size()));
		for (auto& idx : envFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AL) << 4)
						  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::FB));
			ctr.appendUint8(tmp);
			// Operator 1
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 0);
			tmp = static_cast<uint8_t>((tmp << 5)) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS1) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT1) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL1) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL1));
			ctr.appendUint8(tmp);
			int tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG1);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML1));
			ctr.appendUint8(tmp);
			// Operator 2
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 1);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS2) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT2) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL2) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL2));
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG2);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML2));
			ctr.appendUint8(tmp);
			// Operator 3
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 2);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS3) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT3) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL3) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL3));
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG3);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML3));
			ctr.appendUint8(tmp);
			// Operator 4
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(idx, 3);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::AR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::KS4) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::DT4) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SL4) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::RR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::TL4));
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG4);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(idx, FMEnvelopeParameter::ML4));
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}
	}

	// FM LFO
	std::vector<int> lfoFMIdcs = instMan.lock()->getLFOFMEntriedIndices();
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
			tmp = static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(idx, FMLFOParameter::COUNT));
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}
	}

	// FM envelope parameter
	for (size_t i = 0; i < 38; ++i) {
		std::vector<int> idcs = instMan.lock()->getOperatorSequenceFMEntriedIndices(FileIO::ENV_FM_PARAMS[i]);
		if (!idcs.empty()) {
			ctr.appendUint8(0x02 + static_cast<uint8_t>(i));
			ctr.appendUint8(static_cast<uint8_t>(idcs.size()));
			for (auto& idx : idcs) {
				ctr.appendUint8(static_cast<uint8_t>(idx));
				size_t ofs = ctr.size();
				ctr.appendUint16(0);	// Dummy offset
				auto seq = instMan.lock()->getOperatorSequenceFMSequence(FileIO::ENV_FM_PARAMS[i], idx);
				ctr.appendUint16(static_cast<uint16_t>(seq.size()));
				for (auto& com : seq) {
					ctr.appendUint16(static_cast<uint16_t>(com.type));
					ctr.appendInt16(static_cast<int16_t>(com.data));
				}
				auto loop = instMan.lock()->getOperatorSequenceFMLoops(FileIO::ENV_FM_PARAMS[i], idx);
				ctr.appendUint16(static_cast<uint16_t>(loop.size()));
				for (auto& l : loop) {
					ctr.appendUint16(static_cast<uint16_t>(l.begin));
					ctr.appendUint16(static_cast<uint16_t>(l.end));
					ctr.appendUint8(static_cast<uint8_t>(l.times));
				}
				auto release = instMan.lock()->getOperatorSequenceFMRelease(FileIO::ENV_FM_PARAMS[i], idx);
				switch (release.type) {
				case ReleaseType::NO_RELEASE:
					ctr.appendUint8(0x00);
					break;
				case ReleaseType::FIX:
					ctr.appendUint8(0x01);
					ctr.appendUint16(static_cast<uint16_t>(release.begin));
					break;
				case ReleaseType::ABSOLUTE:
					ctr.appendUint8(0x02);
					ctr.appendUint16(static_cast<uint16_t>(release.begin));
					break;
				case ReleaseType::RELATIVE:
					ctr.appendUint8(0x03);
					ctr.appendUint16(static_cast<uint16_t>(release.begin));
					break;
				}
				ctr.appendUint8(0);	// Skip sequence type
				ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
			}
		}
	}

	// FM arpeggio
	std::vector<int> arpFMIdcs = instMan.lock()->getArpeggioFMEntriedIndices();
	if (!arpFMIdcs.empty()) {
		ctr.appendUint8(0x28);
		ctr.appendUint8(static_cast<uint8_t>(arpFMIdcs.size()));
		for (auto& idx : arpFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioFMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getArpeggioFMLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getArpeggioFMRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->getArpeggioFMType(idx)));
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// FM pitch
	std::vector<int> ptFMIdcs = instMan.lock()->getPitchFMEntriedIndices();
	if (!ptFMIdcs.empty()) {
		ctr.appendUint8(0x29);
		ctr.appendUint8(static_cast<uint8_t>(ptFMIdcs.size()));
		for (auto& idx : ptFMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchFMSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getPitchFMLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getPitchFMRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->getPitchFMType(idx)));
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG wave form
	std::vector<int> wfSSGIdcs = instMan.lock()->getWaveFormSSGEntriedIndices();
	if (!wfSSGIdcs.empty()) {
		ctr.appendUint8(0x30);
		ctr.appendUint8(static_cast<uint8_t>(wfSSGIdcs.size()));
		for (auto& idx : wfSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getWaveFormSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendUint16(static_cast<uint16_t>(com.data));
			}
			auto loop = instMan.lock()->getWaveFormSSGLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getWaveFormSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			ctr.appendUint8(0);	// Skip sequence type
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG tone/noise
	std::vector<int> tnSSGIdcs = instMan.lock()->getToneNoiseSSGEntriedIndices();
	if (!tnSSGIdcs.empty()) {
		ctr.appendUint8(0x31);
		ctr.appendUint8(static_cast<uint8_t>(tnSSGIdcs.size()));
		for (auto& idx : tnSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getToneNoiseSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getToneNoiseSSGLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getToneNoiseSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			ctr.appendUint8(0);	// Skip sequence type
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG envelope
	std::vector<int> envSSGIdcs = instMan.lock()->getEnvelopeSSGEntriedIndices();
	if (!envSSGIdcs.empty()) {
		ctr.appendUint8(0x32);
		ctr.appendUint8(static_cast<uint8_t>(envSSGIdcs.size()));
		for (auto& idx : envSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getEnvelopeSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendUint16(static_cast<uint16_t>(com.data));
			}
			auto loop = instMan.lock()->getEnvelopeSSGLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getEnvelopeSSGRelease(idx);

			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			ctr.appendUint8(0);	// Skip sequence type
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG arpeggio
	std::vector<int> arpSSGIdcs = instMan.lock()->getArpeggioSSGEntriedIndices();
	if (!arpSSGIdcs.empty()) {
		ctr.appendUint8(0x33);
		ctr.appendUint8(static_cast<uint8_t>(arpSSGIdcs.size()));
		for (auto& idx : arpSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getArpeggioSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getArpeggioSSGLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getArpeggioSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->getArpeggioSSGType(idx)));
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG pitch
	std::vector<int> ptSSGIdcs = instMan.lock()->getPitchSSGEntriedIndices();
	if (!ptSSGIdcs.empty()) {
		ctr.appendUint8(0x34);
		ctr.appendUint8(static_cast<uint8_t>(ptSSGIdcs.size()));
		for (auto& idx : ptSSGIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchSSGSequence(idx);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getPitchSSGLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getPitchSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NO_RELEASE:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
				break;
			case ReleaseType::FIX:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::ABSOLUTE:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RELATIVE:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->getPitchSSGType(idx)));
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);


	/***** Groove section *****/
	ctr.appendString("GROOVE  ");
	size_t grooveOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy groove section offset
	size_t grooveCnt = mod.lock()->getGrooveCount();
	ctr.appendUint8(static_cast<uint8_t>(grooveCnt - 1));
	for (size_t i = 0; i < grooveCnt; ++i) {
		ctr.appendUint8(static_cast<uint8_t>(i));
		auto seq = mod.lock()->getGroove(static_cast<int>(i)).getSequence();
		ctr.appendUint8(static_cast<uint8_t>(seq.size()));
		for (auto& g : seq) {
			ctr.appendUint8(static_cast<uint8_t>(g));
		}
	}
	ctr.writeUint32(grooveOfs, ctr.size() - grooveOfs);


	/***** Song section *****/
	ctr.appendString("SONG    ");
	size_t songSecOfs = ctr.size();
	ctr.appendUint32(0);	// Dummy song section offset
	size_t songCnt = mod.lock()->getSongCount();
	ctr.appendUint8(static_cast<uint8_t>(songCnt));

	// Song
	for (size_t i = 0; i < songCnt; ++i) {
		ctr.appendUint8(static_cast<uint8_t>(i));
		size_t songOfs = ctr.size();
		ctr.appendUint32(0);	// Dummy song block offset
		auto& sng = mod.lock()->getSong(static_cast<int>(i));
		std::string title = sng.getTitle();
		ctr.appendUint32(title.length());
		if (!title.empty()) ctr.appendString(title);
		ctr.appendUint32(static_cast<uint32_t>(sng.getTempo()));
		uint8_t tmp = static_cast<uint8_t>(sng.getGroove());
		ctr.appendUint8(sng.isUsedTempo() ? (0x80 | tmp) : tmp);
		ctr.appendUint32(static_cast<uint32_t>(sng.getSpeed()));
		ctr.appendUint8(static_cast<uint8_t>(sng.getDefaultPatternSize()) - 1);
		auto style = sng.getStyle();
		switch (style.type) {
		case SongType::STD:		ctr.appendUint8(0x00);	break;
		case SongType::FMEX:	ctr.appendUint8(0x01);	break;
		default:	throw FileOutputError(FileIO::FileType::MOD);
		}

		// Track
		for (auto& attrib : style.trackAttribs) {
			ctr.appendUint8(static_cast<uint8_t>(attrib.number));
			size_t trackOfs = ctr.size();
			ctr.appendUint32(0);	// Dummy track subblock offset
			auto& track = sng.getTrack(attrib.number);

			// Order
			size_t odrSize = track.getOrderSize();
			ctr.appendUint8(static_cast<uint8_t>(odrSize) - 1);
			for (size_t o = 0; o < odrSize; ++o)
				ctr.appendUint8(static_cast<uint8_t>(track.getOrderData(static_cast<int>(o)).patten));

			// Pattern
			for (auto& idx : track.getEditedPatternIndices()) {
				ctr.appendUint8(static_cast<uint8_t>(idx));
				size_t ptnOfs = ctr.size();
				ctr.appendUint32(0);	// Dummy pattern subblock offset
				auto& pattern = track.getPattern(idx);

				// Step
				std::vector<int> stepIdcs = pattern.getEditedStepIndices();
				for (auto& sidx : stepIdcs) {
					ctr.appendUint8(static_cast<uint8_t>(sidx));
					size_t evFlagOfs = ctr.size();
					ctr.appendUint16(0);	// Dummy set event flag
					auto& step = pattern.getStep(sidx);
					uint16_t eventFlag = 0;
					int tmp = step.getNoteNumber();
					if (tmp != -1) {
						eventFlag |= 0x0001;
						ctr.appendInt8(static_cast<int8_t>(tmp));
					}
					tmp = step.getInstrumentNumber();
					if (tmp != -1) {
						eventFlag |= 0x0002;
						ctr.appendUint8(static_cast<uint8_t>(tmp));
					}
					tmp = step.getVolume();
					if (tmp != -1) {
						eventFlag |= 0x0004;
						ctr.appendUint8(static_cast<uint8_t>(tmp));
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
							ctr.appendUint8(static_cast<uint8_t>(tmp));
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

	if (!ctr.save(path)) throw FileOutputError(FileIO::FileType::MOD);
}

void ModuleIO::loadModule(std::string path, std::weak_ptr<Module> mod,
						  std::weak_ptr<InstrumentsManager> instMan)
{
	BinaryContainer ctr;

	if (!ctr.load(path)) throw FileInputError(FileIO::FileType::MOD);

	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerMod")
		throw FileCorruptionError(FileIO::FileType::MOD);
	globCsr += 16;
	size_t eofOfs = ctr.readUint32(globCsr);
	size_t eof = globCsr + eofOfs;
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofModuleFileInBCD())
		throw FileVersionError(fileVersion, Version::ofApplicationInBCD(), FileIO::FileType::MOD);
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
			throw FileCorruptionError(FileIO::FileType::MOD);
	}

	mod.lock()->setFilePath(path);
}

size_t ModuleIO::loadModuleSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr,
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
	mod.lock()->setStepHighlight1Distance(ctr.readUint32(modCsr));
	modCsr += 4;
	if (version >= Version::toBCD(1, 0, 3)) {
		mod.lock()->setStepHighlight2Distance(ctr.readUint32(modCsr));
		modCsr += 4;
	}
	else {
		mod.lock()->setStepHighlight2Distance(mod.lock()->getStepHighlight1Distance() * 4);
	}

	return globCsr + modOfs;
}

size_t ModuleIO::loadInstrumentSectionInModule(std::weak_ptr<InstrumentsManager> instMan,
											   BinaryContainer& ctr, size_t globCsr, uint32_t version)
{
	(void)version;

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
			for (auto& param : FileIO::ENV_FM_PARAMS) {
				tmp = ctr.readUint8(iCsr);
				instFM->setOperatorSequenceEnabled(param, (0x80 & tmp) ? false : true);
				instFM->setOperatorSequenceNumber(param, 0x7f & tmp);
				iCsr += 1;
			}

			tmp = ctr.readUint8(iCsr);
			instFM->setArpeggioEnabled(FMOperatorType::All, (0x80 & tmp) ? false : true);
			instFM->setArpeggioNumber(FMOperatorType::All, 0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instFM->setPitchEnabled(FMOperatorType::All, (0x80 & tmp) ? false : true);
			instFM->setPitchNumber(FMOperatorType::All, 0x7f & tmp);
			iCsr += 1;
			tmp = ctr.readUint8(iCsr);
			instFM->setEnvelopeResetEnabled(FMOperatorType::All, (tmp & 0x01));
			instFM->setEnvelopeResetEnabled(FMOperatorType::Op1, (tmp & 0x02));
			instFM->setEnvelopeResetEnabled(FMOperatorType::Op2, (tmp & 0x04));
			instFM->setEnvelopeResetEnabled(FMOperatorType::Op3, (tmp & 0x08));
			instFM->setEnvelopeResetEnabled(FMOperatorType::Op4, (tmp & 0x10));
			iCsr += 1;
			if (version >= Version::toBCD(1, 1, 0)) {
				for (auto& t : FileIO::OP_FM_TYPES) {
					tmp = ctr.readUint8(iCsr);
					instFM->setArpeggioEnabled(t, (0x80 & tmp) ? false : true);
					instFM->setArpeggioNumber(t, 0x7f & tmp);
					iCsr += 1;
				}
				for (auto& t : FileIO::OP_FM_TYPES) {
					tmp = ctr.readUint8(iCsr);
					instFM->setPitchEnabled(t, (0x80 & tmp) ? false : true);
					instFM->setPitchNumber(t, 0x7f & tmp);
					iCsr += 1;
				}
			}
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

size_t ModuleIO::loadInstrumentPropertySectionInModule(std::weak_ptr<InstrumentsManager> instMan,
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
					if (version < Version::toBCD(1, 2, 0)) csr += 2;
					if (l == 0)
						instMan.lock()->setArpeggioFMSequenceCommand(idx, 0, data, 0);
					else
						instMan.lock()->addArpeggioFMSequenceCommand(idx, data, 0);
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
					if (version < Version::toBCD(1, 2, 0)) csr += 2;
					if (l == 0)
						instMan.lock()->setPitchFMSequenceCommand(idx, 0, data, 0);
					else
						instMan.lock()->addPitchFMSequenceCommand(idx, data, 0);
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
					if (version < Version::toBCD(1, 2, 0)) {
						if (data == 3) data = static_cast<int>(SSGWaveFormType::SQM_TRIANGLE);
						else if (data == 4) data = static_cast<int>(SSGWaveFormType::SQM_SAW);
					}
					uint16_t subdata = ctr.readUint16(csr);
					csr += 2;
					if (version < Version::toBCD(1, 2, 0)) {
						if (subdata != 0xffff)
							subdata = PitchConverter::getPitchSSGSquare(subdata);
					}
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
					if (version < Version::toBCD(1, 2, 0)) csr += 2;
					if (l == 0)
						instMan.lock()->setToneNoiseSSGSequenceCommand(idx, 0, data, 0);
					else
						instMan.lock()->addToneNoiseSSGSequenceCommand(idx, data, 0);
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
					uint16_t subdata = ctr.readUint16(csr);
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
					if (version < Version::toBCD(1, 2, 0)) csr += 2;
					if (l == 0)
						instMan.lock()->setArpeggioSSGSequenceCommand(idx, 0, data, 0);
					else
						instMan.lock()->addArpeggioSSGSequenceCommand(idx, data, 0);
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
					if (version < Version::toBCD(1, 2, 0)) csr += 2;
					if (l == 0)
						instMan.lock()->setPitchSSGSequenceCommand(idx, 0, data, 0);
					else
						instMan.lock()->addPitchSSGSequenceCommand(idx, data, 0);
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

size_t ModuleIO::loadInstrumentPropertyOperatorSequence(FMEnvelopeParameter param,
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
		if (version < Version::toBCD(1, 2, 0)) csr += 2;
		if (l == 0)
			instMan.lock()->setOperatorSequenceFMSequenceCommand(param, idx, 0, data, 0);
		else
			instMan.lock()->addOperatorSequenceFMSequenceCommand(param, idx, data, 0);
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

size_t ModuleIO::loadGrooveSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr,
										   size_t globCsr, uint32_t version)
{
	(void)version;

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

size_t ModuleIO::loadSongSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr, size_t globCsr, uint32_t version)
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
			mod.lock()->addSong(idx, SongType::STD, title, isTempo,
								static_cast<int>(tempo), groove, static_cast<int>(speed), ptnSize);
			break;
		}
		case 0x01:	// FM3ch expanded
		{
			mod.lock()->addSong(idx, SongType::FMEX, title, isTempo,
								static_cast<int>(tempo), groove, static_cast<int>(speed), ptnSize);
			break;
		}
		default:
			throw FileCorruptionError(FileIO::FileType::MOD);
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
					auto& step = pattern.getStep(static_cast<int>(stepIdx));
					uint16_t eventFlag = ctr.readUint16(pcsr);
					pcsr += 2;
					if (eventFlag & 0x0001)	{
						if (version >= Version::toBCD(1, 0, 2)) {
							step.setNoteNumber(ctr.readInt8(pcsr++));
						}
						else {
							// Change FM octave (song type is only 0x00 before v1.0.2)
							int8_t nn = ctr.readInt8(pcsr++);
							if (trackIdx < 6 && 0 <= nn && nn < 84)
								step.setNoteNumber(nn + 12);
							else
								step.setNoteNumber(nn);
						}
					}
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

void ModuleIO::backupModule(std::string path)
{
	try {
		std::ifstream ifs(path, std::ios::binary);
		std::ofstream ofs(path + ".bak", std::ios::binary);
		ofs << ifs.rdbuf();
	} catch (...) {
		throw FileOutputError(FileIO::FileType::MOD);
	}
}
