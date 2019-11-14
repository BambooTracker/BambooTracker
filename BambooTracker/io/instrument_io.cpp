#include "instrument_io.hpp"
#include "file_io.hpp"
#include <nowide/fstream.hpp>
#include <locale>
#include <vector>
#include <algorithm>
#include <map>
#include "version.hpp"
#include "file_io.hpp"
#include "file_io_error.hpp"
#include "pitch_converter.hpp"
#include "misc.hpp"

InstrumentIO::InstrumentIO() {}

void InstrumentIO::saveInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum)
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

	std::vector<int> fmArpNums, fmPtNums;
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
			uint8_t tmp = static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::All))
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op1) << 1)
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op2) << 2)
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op3) << 3)
						  | static_cast<uint8_t>(instFM->getEnvelopeResetEnabled(FMOperatorType::Op4) << 4);
			ctr.appendUint8(tmp);
			if (instFM->getArpeggioEnabled(FMOperatorType::All))
				fmArpNums.push_back(instFM->getArpeggioNumber(FMOperatorType::All));
			for (auto& t : FileIO::OP_FM_TYPES) {
				if (instFM->getArpeggioEnabled(t)) {
					int n = instFM->getArpeggioNumber(t);
					if (std::find(fmArpNums.begin(), fmArpNums.end(), n) == fmArpNums.end())
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
			for (auto& t : FileIO::OP_FM_TYPES) {
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
			for (auto& t : FileIO::OP_FM_TYPES) {
				if (instFM->getPitchEnabled(t)) {
					int n = instFM->getPitchNumber(t);
					if (std::find(fmPtNums.begin(), fmPtNums.end(), n) == fmPtNums.end())
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
			for (auto& t : FileIO::OP_FM_TYPES) {
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
		case SoundSource::SSG:
		{
			ctr.appendUint8(0x01);
			break;
		}
		case SoundSource::DRUM:
			break;
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
			uint8_t tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AL) << 4)
						  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::FB));
			ctr.appendUint8(tmp);
			// Operator 1
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 0);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS1) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT1) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL1) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL1));
			ctr.appendUint8(tmp);
			int tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG1);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML1));
			ctr.appendUint8(tmp);
			// Operator 2
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 1);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS2) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT2) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL2) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL2));
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG2);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML2));
			ctr.appendUint8(tmp);
			// Operator 3
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 2);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS3) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT3) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL3) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL3));
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG3);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML3));
			ctr.appendUint8(tmp);
			// Operator 4
			tmp = instMan.lock()->getEnvelopeFMOperatorEnabled(envNum, 3);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS4) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT4) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL4) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL4));
			ctr.appendUint8(tmp);
			tmp2 = instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG4);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instMan.lock()->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML4));
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}

		// FM LFO
		if (instFM->getLFOEnabled()) {
			int lfoNum = instFM->getLFONumber();
			ctr.appendUint8(0x01);
			size_t ofs = ctr.size();
			ctr.appendUint8(0);	// Dummy offset
			uint8_t tmp = static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::FREQ) << 4)
						  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::PMS));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM4) << 7)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM3) << 6)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM2) << 5)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AM1) << 4)
				  | static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::AMS));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instMan.lock()->getLFOFMparameter(lfoNum, FMLFOParameter::Count));
			ctr.appendUint8(tmp);
			ctr.writeUint8(ofs, static_cast<uint8_t>(ctr.size() - ofs));
		}

		// FM envelope parameter
		for (size_t i = 0; i < 38; ++i) {
			if (instFM->getOperatorSequenceEnabled(FileIO::ENV_FM_PARAMS[i])) {
				int seqNum = instFM->getOperatorSequenceNumber(FileIO::ENV_FM_PARAMS[i]);
				ctr.appendUint8(0x02 + static_cast<uint8_t>(i));
				size_t ofs = ctr.size();
				ctr.appendUint16(0);	// Dummy offset
				auto seq = instMan.lock()->getOperatorSequenceFMSequence(FileIO::ENV_FM_PARAMS[i], seqNum);
				ctr.appendUint16(static_cast<uint16_t>(seq.size()));
				for (auto& com : seq) {
					ctr.appendUint16(static_cast<uint16_t>(com.type));
				}
				auto loop = instMan.lock()->getOperatorSequenceFMLoops(FileIO::ENV_FM_PARAMS[i], seqNum);
				ctr.appendUint16(static_cast<uint16_t>(loop.size()));
				for (auto& l : loop) {
					ctr.appendUint16(static_cast<uint16_t>(l.begin));
					ctr.appendUint16(static_cast<uint16_t>(l.end));
					ctr.appendUint8(static_cast<uint8_t>(l.times));
				}
				auto release = instMan.lock()->getOperatorSequenceFMRelease(FileIO::ENV_FM_PARAMS[i], seqNum);
				switch (release.type) {
				case ReleaseType::NoRelease:
					ctr.appendUint8(0x00);
					break;
				case ReleaseType::FixedRelease:
					ctr.appendUint8(0x01);
					ctr.appendUint16(static_cast<uint16_t>(release.begin));
					break;
				case ReleaseType::AbsoluteRelease:
					ctr.appendUint8(0x02);
					ctr.appendUint16(static_cast<uint16_t>(release.begin));
					break;
				case ReleaseType::RelativeRelease:
					ctr.appendUint8(0x03);
					ctr.appendUint16(static_cast<uint16_t>(release.begin));
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
			auto seq = instMan.lock()->getArpeggioFMSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getArpeggioFMLoops(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getArpeggioFMRelease(arpNum);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			switch (instMan.lock()->getArpeggioFMType(arpNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::FIXED_SEQUENCE:		ctr.appendUint8(0x01);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}

		// FM pitch
		for (int& ptNum : fmPtNums) {
			ctr.appendUint8(0x29);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instMan.lock()->getPitchFMSequence(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getPitchFMLoops(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getPitchFMRelease(ptNum);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			switch (instMan.lock()->getPitchFMType(ptNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
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
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
			}
			auto loop = instMan.lock()->getWaveFormSSGLoops(wfNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getWaveFormSSGRelease(wfNum);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
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
			auto seq = instMan.lock()->getToneNoiseSSGSequence(tnNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getToneNoiseSSGLoops(tnNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getToneNoiseSSGRelease(tnNum);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
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
			auto seq = instMan.lock()->getEnvelopeSSGSequence(envNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
			}
			auto loop = instMan.lock()->getEnvelopeSSGLoops(envNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getEnvelopeSSGRelease(envNum);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
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
			auto seq = instMan.lock()->getArpeggioSSGSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getArpeggioSSGLoops(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getArpeggioSSGRelease(arpNum);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			switch (instMan.lock()->getArpeggioSSGType(arpNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::FIXED_SEQUENCE:		ctr.appendUint8(0x01);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
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
			auto seq = instMan.lock()->getPitchSSGSequence(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getPitchSSGLoops(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getPitchSSGRelease(ptNum);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				break;
			case ReleaseType::FixedRelease:
				ctr.appendUint8(0x01);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::AbsoluteRelease:
				ctr.appendUint8(0x02);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			case ReleaseType::RelativeRelease:
				ctr.appendUint8(0x03);
				ctr.appendUint16(static_cast<uint16_t>(release.begin));
				break;
			}
			switch (instMan.lock()->getPitchSSGType(ptNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
		break;
	}
	case SoundSource::DRUM:
		break;
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);

	ctr.writeUint32(eofOfs, ctr.size() - eofOfs);

	if (!ctr.save(path)) throw FileOutputError(FileIO::FileType::Inst);
}

AbstractInstrument* InstrumentIO::loadInstrument(std::string path,
												 std::weak_ptr<InstrumentsManager> instMan,
												 int instNum)
{
	std::string ext = FileIO::getExtension(path);
	if (ext.compare("dmp") == 0) return InstrumentIO::loadDMPFile(path, instMan, instNum);
	if (ext.compare("tfi") == 0) return InstrumentIO::loadTFIFile(path, instMan, instNum);
	if (ext.compare("vgi") == 0) return InstrumentIO::loadVGIFile(path, instMan, instNum);
	if (ext.compare("opni") == 0) return InstrumentIO::loadOPNIFile(path, instMan, instNum);
	if (ext.compare("y12") == 0) return InstrumentIO::loadY12File(path, instMan, instNum);
	if (ext.compare("ins") == 0) return InstrumentIO::loadINSFile(path, instMan, instNum);
	if (ext.compare("bti") == 0) return InstrumentIO::loadBTIFile(path, instMan, instNum);
	throw FileInputError(FileIO::FileType::Inst);
}

AbstractInstrument* InstrumentIO::loadBTIFile(std::string path,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum)
{
	BinaryContainer ctr;

	if (!ctr.load(path)) throw FileInputError(FileIO::FileType::Inst);

	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerIst")
		throw FileCorruptionError(FileIO::FileType::Inst);
	globCsr += 16;
	/*size_t eofOfs = */ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofInstrumentFileInBCD())
		throw FileVersionError(fileVersion, Version::ofApplicationInBCD(), FileIO::FileType::Inst);
	globCsr += 4;


	/***** Instrument section *****/
	if (ctr.readString(globCsr, 8) != "INSTRMNT")
		throw FileCorruptionError(FileIO::FileType::Inst);
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
		std::map<FMOperatorType, int> fmArpMap, fmPtMap;
		AbstractInstrument* inst = nullptr;
		switch (ctr.readUint8(instCsr++)) {
		case 0x00:	// FM
		{
			inst = new InstrumentFM(instNum, name, instMan.lock().get());
			auto fm = dynamic_cast<InstrumentFM*>(inst);
			uint8_t tmp = ctr.readUint8(instCsr++);
			fm->setEnvelopeResetEnabled(FMOperatorType::All, (tmp & 0x01));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op1, (tmp & 0x02));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op2, (tmp & 0x04));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op3, (tmp & 0x08));
			fm->setEnvelopeResetEnabled(FMOperatorType::Op4, (tmp & 0x10));
			if (fileVersion >= Version::toBCD(1, 1, 0)) {
				fmArpMap.emplace(FMOperatorType::All, ctr.readUint8(instCsr++));
				for (auto& t : FileIO::OP_FM_TYPES) {
					tmp = ctr.readUint8(instCsr++);
					if (!(tmp & 0x80)) fmArpMap.emplace(t, (tmp & 0x7f));
				}
				fmPtMap.emplace(FMOperatorType::All, ctr.readUint8(instCsr++));
				for (auto& t : FileIO::OP_FM_TYPES) {
					tmp = ctr.readUint8(instCsr++);
					if (!(tmp & 0x80)) fmPtMap.emplace(t, (tmp & 0x7f));
				}
			}
			break;
		}
		case 0x01:	// SSG
		{
			inst = new InstrumentSSG(instNum, name, instMan.lock().get());
			break;
		}
		default:
			throw FileCorruptionError(FileIO::FileType::Inst);
		}
		globCsr += instOfs;


		/***** Instrument property section *****/
		if (ctr.readString(globCsr, 8) != "INSTPROP")
			throw FileCorruptionError(FileIO::FileType::Inst);
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
					nums.push_back(instMan.lock()->findFirstFreePlainEnvelopeFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x01:	// FM LFO
				{
					nums.push_back(instMan.lock()->findFirstFreePlainLFOFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x02:	// FM AL
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::AL));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x03:	// FM FB
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::FB));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x04:	// FM AR1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::AR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x05:	// FM DR1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x06:	// FM SR1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x07:	// FM RR1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::RR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x08:	// FM SL1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SL1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x09:	// FM TL1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::TL1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0a:	// FM KS1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::KS1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0b:	// FM ML1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::ML1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0c:	// FM DT1
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DT1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0d:	// FM AR2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::AR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0e:	// FM DR2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0f:	// FM SR2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x10:	// FM RR2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::RR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x11:	// FM SL2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SL2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x12:	// FM TL2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::TL2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x13:	// FM KS2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::KS2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x14:	// FM ML2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::ML2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x15:	// FM DT2
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DT2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x16:	// FM AR3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::AR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x17:	// FM DR3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x18:	// FM SR3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x19:	// FM RR3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::RR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1a:	// FM SL3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SL3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1b:	// FM TL3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::TL3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1c:	// FM KS3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::KS3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1d:	// FM ML3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::ML3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1e:	// FM DT3
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DT3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1f:	// FM AR4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::AR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x20:	// FM DR4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x21:	// FM SR4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x22:	// FM RR4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::RR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x23:	// FM SL4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::SL4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x24:	// FM TL4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::TL4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x25:	// FM KS4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::KS4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x26:	// FM ML4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::ML4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x27:	// FM DT4
				{
					nums.push_back(instMan.lock()->findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter::DT4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x28:	// FM arpeggio
				{
					nums.push_back(instMan.lock()->findFirstFreePlainArpeggioFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x29:	// FM pitch
				{
					nums.push_back(instMan.lock()->findFirstFreePlainPitchFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x30:	// SSG wave form
				{
					nums.push_back(instMan.lock()->findFirstFreePlainWaveFormSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x31:	// SSG tone/noise
				{
					nums.push_back(instMan.lock()->findFirstFreePlainToneNoiseSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x32:	// SSG envelope
				{
					nums.push_back(instMan.lock()->findFirstFreePlainEnvelopeSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x33:	// SSG arpeggio
				{
					nums.push_back(instMan.lock()->findFirstFreePlainArpeggioSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x34:	// SSG pitch
				{
					nums.push_back(instMan.lock()->findFirstFreePlainPitchSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
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
					instMan.lock()->setLFOFMParameter(idx, FMLFOParameter::Count, tmp);
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
						instMan.lock()->setArpeggioFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
						if (pos < seqLen) instMan.lock()->setArpeggioFMRelease(idx, ReleaseType::FixedRelease, pos);
						else instMan.lock()->setArpeggioFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instMan.lock()->setArpeggioFMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x01:	// Fixed
							instMan.lock()->setArpeggioFMType(idx, SequenceType::FIXED_SEQUENCE);
							break;
						case 0x02:	// Relative
							instMan.lock()->setArpeggioFMType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
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
						instMan.lock()->setPitchFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
						if (pos < seqLen) instMan.lock()->setPitchFMRelease(idx, ReleaseType::FixedRelease, pos);
						else instMan.lock()->setPitchFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instMan.lock()->setPitchFMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x02:	// Relative
							instMan.lock()->setPitchFMType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
						}
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
						if (fileVersion < Version::toBCD(1, 2, 0)) {
							if (data == 3) data = static_cast<int>(SSGWaveFormType::SQM_TRIANGLE);
							else if (data == 4) data = static_cast<int>(SSGWaveFormType::SQM_SAW);
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
						instMan.lock()->setWaveFormSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
						if (pos < seqLen) instMan.lock()->setWaveFormSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instMan.lock()->setWaveFormSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
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
						instMan.lock()->setToneNoiseSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
						if (pos < seqLen) instMan.lock()->setToneNoiseSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instMan.lock()->setToneNoiseSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
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
						instMan.lock()->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
						// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instMan.lock()->setEnvelopeSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instMan.lock()->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instMan.lock()->setEnvelopeSSGRelease(idx, ReleaseType::AbsoluteRelease, pos);
						else instMan.lock()->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instMan.lock()->setEnvelopeSSGRelease(idx, ReleaseType::RelativeRelease, pos);
						else instMan.lock()->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
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
						instMan.lock()->setArpeggioSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
						if (pos < seqLen) instMan.lock()->setArpeggioSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instMan.lock()->setArpeggioSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instMan.lock()->setArpeggioSSGType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x01:	// Fixed
							instMan.lock()->setArpeggioSSGType(idx, SequenceType::FIXED_SEQUENCE);
							break;
						case 0x02:	// Relative
							instMan.lock()->setArpeggioSSGType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
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
						instMan.lock()->setPitchSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
						if (pos < seqLen) instMan.lock()->setPitchSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instMan.lock()->setPitchSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instMan.lock()->setPitchSSGType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x02:	// Relative
							instMan.lock()->setPitchSSGType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
						}
					}

					instPropCsr += ofs;
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
				}
			}
		}


		return inst;
	}
}

size_t InstrumentIO::loadInstrumentPropertyOperatorSequenceForInstrument(
		FMEnvelopeParameter param, size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan,
		BinaryContainer& ctr, InstrumentFM* inst, int idx, uint32_t version)
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
		instMan.lock()->setOperatorSequenceFMRelease(param, idx, ReleaseType::NoRelease, -1);
		break;
	case 0x01:	// Fixed
	{
		uint16_t pos = ctr.readUint16(csr);
		csr += 2;
		// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
		if (pos < seqLen) instMan.lock()->setOperatorSequenceFMRelease(param, idx, ReleaseType::FixedRelease, pos);
		else instMan.lock()->setOperatorSequenceFMRelease(param, idx, ReleaseType::NoRelease, -1);
		break;
	}
	default:
		throw FileCorruptionError(FileIO::FileType::Inst);
	}

	if (version >= Version::toBCD(1, 0, 1)) {
		++csr;	// Skip sequence type
	}

	return ofs;
}

AbstractInstrument* InstrumentIO::loadDMPFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIO::FileType::Inst);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	uint8_t insType = 1; // default to FM
	uint8_t fileVersion = ctr.readUint8(csr++);
	if (fileVersion == 0) { // older, unversioned dmp
		if (ctr.size() != 49) throw FileCorruptionError(FileIO::FileType::Inst);
	}
	else {
		if (fileVersion < 9) throw FileCorruptionError(FileIO::FileType::Inst);
		if (fileVersion == 9 && ctr.size() != 51) { // make sure it's not for that discontinued chip
			throw FileCorruptionError(FileIO::FileType::Inst);
		}
		uint8_t system = 2; // default to genesis
		if (fileVersion >= 11) system = ctr.readUint8(csr++);
		if (system != 2 && system != 3 && system != 8) { // genesis, sms and arcade only
			throw FileCorruptionError(FileIO::FileType::Inst);
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
			int idx = instMan.lock()->findFirstFreePlainEnvelopeSSG();
			if (idx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
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
			int idx = instMan.lock()->findFirstFreePlainArpeggioSSG();
			if (idx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
			ssg->setArpeggioEnabled(true);
			ssg->setArpeggioNumber(idx);
			uint8_t arpType = ctr.readUint8(csr + arpSize * 4 + 1);
			if (arpType == 1) instMan.lock()->setArpeggioSSGType(idx, SequenceType::FIXED_SEQUENCE);
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
		int envIdx = instMan.lock()->findFirstFreePlainEnvelopeFM();
		if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
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
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, convertTFIVGMDT(ctr.readUint8(csr++) & 15)); // mask out OPM's DT2
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
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, convertTFIVGMDT(ctr.readUint8(csr++) & 15));
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
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, convertTFIVGMDT(ctr.readUint8(csr++) & 15));
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
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, convertTFIVGMDT(ctr.readUint8(csr++) & 15));
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
		int ssgeg4 = ctr.readUint8(csr++);
		ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
		instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);

		if (pms || ams) {
			int lfoIdx = instMan.lock()->findFirstFreePlainLFOFM();
			if (lfoIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
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

size_t InstrumentIO::getPropertyPositionForBTB(const BinaryContainer& propCtr, uint8_t subsecType, uint8_t index)
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
				default:	// Sequence
					csr += propCtr.readUint16(csr);
					break;
				}
			}
		}
	}

	throw FileCorruptionError(FileIO::FileType::Inst);
}

int InstrumentIO::convertTFIVGMDT(int dt)
{
	switch (dt) {
	case 0:		return 7;
	case 1:		return 6;
	case 2:		return 5;
	case 3:		return 0;
	case 4:		return 1;
	case 5:		return 2;
	case 6:		return 3;
	default:	throw std::out_of_range("Out of range dt");
	}
}

AbstractInstrument* InstrumentIO::loadTFIFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIO::FileType::Inst);
	if (ctr.size() != 42) throw FileCorruptionError(FileIO::FileType::Inst);
	int envIdx = instMan.lock()->findFirstFreePlainEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instMan.lock().get());
	inst->setEnvelopeNumber(envIdx);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, convertTFIVGMDT(ctr.readUint8(csr++)));
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
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, convertTFIVGMDT(ctr.readUint8(csr++)));
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
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, convertTFIVGMDT(ctr.readUint8(csr++)));
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
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, convertTFIVGMDT(ctr.readUint8(csr++)));
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

AbstractInstrument* InstrumentIO::loadVGIFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIO::FileType::Inst);
	if (ctr.size() != 43) throw FileCorruptionError(FileIO::FileType::Inst);
	int envIdx = instMan.lock()->findFirstFreePlainEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instMan.lock().get());
	inst->setEnvelopeNumber(envIdx);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
	uint8_t pams = ctr.readUint8(csr++);

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, convertTFIVGMDT(ctr.readUint8(csr++)));
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
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, convertTFIVGMDT(ctr.readUint8(csr++)));
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
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, convertTFIVGMDT(ctr.readUint8(csr++)));
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
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, convertTFIVGMDT(ctr.readUint8(csr++)));
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
		int lfoIdx = instMan.lock()->findFirstFreePlainLFOFM();
		if (lfoIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
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

AbstractInstrument* InstrumentIO::loadOPNIFile(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum) {
	OPNIFile opni;

	nowide::ifstream in(path, std::ios::binary);
	in.seekg(0, std::ios::end);
	std::streampos size = in.tellg();

	if (!in)
		throw FileInputError(FileIO::FileType::Inst);
	else {
		std::unique_ptr<char[]> buf(new char[static_cast<size_t>(size)]);
		in.seekg(0, std::ios::beg);
		if (!in.read(buf.get(), static_cast<int>(size)) || in.gcount() != size)
			throw FileInputError(FileIO::FileType::Inst);
		if (WOPN_LoadInstFromMem(&opni, buf.get(), static_cast<size_t>(size)) != 0)
			throw FileCorruptionError(FileIO::FileType::Inst);
	}

	return loadWOPNInstrument(opni.inst, instMan, instNum);
}

AbstractInstrument* InstrumentIO::loadY12File(std::string path,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum)
{
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIO::FileType::Inst);
	if (ctr.size() != 128) throw FileCorruptionError(FileIO::FileType::Inst);
	int envIdx = instMan.lock()->findFirstFreePlainEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instMan.lock().get());
	inst->setEnvelopeNumber(envIdx);
	uint8_t tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, 0x1f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, 0x1f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg3 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, 0x1f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, 0x1f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);
	csr += 9;

	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
	csr += 14;

	return inst;
}

AbstractInstrument* InstrumentIO::loadINSFile(std::string path,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum)
{
	BinaryContainer ctr;
	if (!ctr.load(path)) throw FileInputError(FileIO::FileType::Inst);
	size_t csr = 0;
	if (ctr.readString(csr, 4).compare("MVSI") != 0) throw FileInputError(FileIO::FileType::Inst);
	csr += 4;
	/*uint8_t fileVersion = */std::stoi(ctr.readString(csr++, 1));
	size_t nameCsr = 0;
	while (ctr.readChar(nameCsr++) != '\0')
		;
	std::string name = ctr.readString(csr, nameCsr - csr);
	csr = nameCsr;

	if (ctr.size() - csr != 25) throw FileInputError(FileIO::FileType::Inst);

	int envIdx = instMan.lock()->findFirstFreePlainEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);

	InstrumentFM* inst = new InstrumentFM(instNum, name, instMan.lock().get());
	inst->setEnvelopeNumber(envIdx);
	uint8_t tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, tmp >> 4);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, 0x3f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, 0x3f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, 0x3f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, 0x3f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, 0x0f & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);

	tmp = ctr.readUint8(csr++);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, 0x07 & tmp);
	instMan.lock()->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, tmp >> 3);

	return inst;
}

AbstractInstrument* InstrumentIO::loadWOPNInstrument(const WOPNInstrument &srcInst,
													 std::weak_ptr<InstrumentsManager> instMan,
													 int instNum) {
	int envIdx = instMan.lock()->findFirstFreePlainEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
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

	LOAD_OPERATOR(1)
	LOAD_OPERATOR(2)
	LOAD_OPERATOR(3)
	LOAD_OPERATOR(4)

#undef LOAD_OPERATOR

	if (srcInst.lfosens != 0) {
		int lfoIdx = instMan.lock()->findFirstFreePlainLFOFM();
		if (lfoIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
		inst->setLFOEnabled(true);
		inst->setLFONumber(lfoIdx);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, srcInst.lfosens & 7);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, (srcInst.lfosens >> 4) & 3);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, am1);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, am2);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, am3);
		instMan.lock()->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, am4);
	}

	if (srcInst.note_offset != 0) {
		int arpIdx = instMan.lock()->findFirstFreePlainArpeggioFM();
		if (arpIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
		inst->setArpeggioEnabled(FMOperatorType::All, true);
		inst->setArpeggioNumber(FMOperatorType::All, arpIdx);
		instMan.lock()->setArpeggioFMSequenceCommand(arpIdx, 0, srcInst.note_offset + 48, -1);
		instMan.lock()->setArpeggioFMType(arpIdx, SequenceType::ABSOLUTE_SEQUENCE);
	}

	return inst;
}

AbstractInstrument* InstrumentIO::loadBTBInstrument(BinaryContainer instCtr,
													BinaryContainer propCtr,
													std::weak_ptr<InstrumentsManager> instMan,
													int instNum, uint32_t bankVersion)
{
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
		auto fm = new InstrumentFM(instNum, name, instMan.lock().get());

		/* Envelope */
		{
			auto orgEnvNum = instCtr.readUint8(instCsr++);
			int envNum = instMan.lock()->findFirstFreePlainEnvelopeFM();
			if (envNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
			fm->setEnvelopeNumber(envNum);
			size_t envCsr = getPropertyPositionForBTB(propCtr, 0x00, orgEnvNum);
			uint8_t tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AL, tmp >> 4);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::FB, tmp & 0x0f);
			// Operator 1
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMOperatorEnabled(envNum, 0, (0x20 & tmp) ? true : false);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR1, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS1, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR1, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT1, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR1, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL1, tmp >> 4);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR1, tmp & 0x0f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL1, tmp);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML1, tmp & 0x0f);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG1,
												   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
			// Operator 2
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMOperatorEnabled(envNum, 1, (0x20 & tmp) ? true : false);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR2, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS2, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR2, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT2, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR2, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL2, tmp >> 4);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR2, tmp & 0x0f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL2, tmp);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML2, tmp & 0x0f);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG2,
												   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
			// Operator 3
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMOperatorEnabled(envNum, 2, (0x20 & tmp) ? true : false);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR3, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS3, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR3, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT3, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR3, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL3, tmp >> 4);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR3, tmp & 0x0f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL3, tmp);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML3, tmp & 0x0f);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG3,
												   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
			// Operator 4
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMOperatorEnabled(envNum, 3, (0x20 & tmp) ? true : false);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR4, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS4, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR4, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT4, tmp >> 5);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR4, tmp & 0x1f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL4, tmp >> 4);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR4, tmp & 0x0f);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL4, tmp);
			tmp = propCtr.readUint8(envCsr++);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML4, tmp & 0x0f);
			instMan.lock()->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG4,
												   (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
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
				int lfoNum = instMan.lock()->findFirstFreePlainLFOFM();
				if (lfoNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
				fm->setLFONumber(lfoNum);
				size_t lfoCsr = getPropertyPositionForBTB(propCtr, 0x01, orgLFONum);
				tmp = propCtr.readUint8(lfoCsr++);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::FREQ, tmp >> 4);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::PMS, tmp & 0x0f);
				tmp = propCtr.readUint8(lfoCsr++);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::AMS, tmp & 0x0f);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::AM1, (tmp & 0x10) ? true : false);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::AM2, (tmp & 0x20) ? true : false);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::AM3, (tmp & 0x40) ? true : false);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::AM4, (tmp & 0x80) ? true : false);
				tmp = propCtr.readUint8(lfoCsr++);
				instMan.lock()->setLFOFMParameter(lfoNum, FMLFOParameter::Count, tmp);
			}
		}

		/* Operator sequence */
		uint8_t tmpCnt = 0;
		for (auto& param : FileIO::ENV_FM_PARAMS) {
			++tmpCnt;
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				fm->setOperatorSequenceEnabled(param, false);
				fm->setOperatorSequenceNumber(param, 0x7f & tmp);
			}
			else {
				fm->setOperatorSequenceEnabled(param, true);
				uint8_t orgOpSeqNum = 0x7f & tmp;
				int opSeqNum = instMan.lock()->findFirstFreePlainOperatorSequenceFM(param);
				if (opSeqNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
				fm->setOperatorSequenceNumber(param, opSeqNum);
				size_t opSeqCsr = getPropertyPositionForBTB(propCtr, 0x02 + tmpCnt, orgOpSeqNum);

				uint16_t seqLen = propCtr.readUint16(opSeqCsr);
				opSeqCsr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = propCtr.readUint16(opSeqCsr);
					opSeqCsr += 2;
					if (l == 0)
						instMan.lock()->setOperatorSequenceFMSequenceCommand(param, opSeqNum, 0, data, 0);
					else
						instMan.lock()->addOperatorSequenceFMSequenceCommand(param, opSeqNum, data, 0);
				}

				uint16_t loopCnt = propCtr.readUint16(opSeqCsr);
				opSeqCsr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(propCtr.readUint16(opSeqCsr));
						opSeqCsr += 2;
						ends.push_back(propCtr.readUint16(opSeqCsr));
						opSeqCsr += 2;
						times.push_back(propCtr.readUint8(opSeqCsr++));
					}
					instMan.lock()->setOperatorSequenceFMLoops(param, opSeqNum, begins, ends, times);
				}

				switch (propCtr.readUint8(opSeqCsr++)) {
				case 0x00:	// No release
					instMan.lock()->setOperatorSequenceFMRelease(param, opSeqNum, ReleaseType::NoRelease, -1);
					break;
				case 0x01:	// Fixed
				{
					uint16_t pos = propCtr.readUint16(opSeqCsr);
					opSeqCsr += 2;
					// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
					if (pos < seqLen) instMan.lock()->setOperatorSequenceFMRelease(param, opSeqNum, ReleaseType::FixedRelease, pos);
					else instMan.lock()->setOperatorSequenceFMRelease(param, opSeqNum, ReleaseType::NoRelease, -1);
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
				}
			}
		}

		/* Arpeggio */
		{
			std::map<FMOperatorType, int> tmpMap;
			std::map<int, FMOperatorType> orgNumMap;
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
						int arpNum = instMan.lock()->findFirstFreePlainArpeggioFM();
						if (arpNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
						fm->setArpeggioNumber(pair.first, arpNum);
						size_t arpCsr = getPropertyPositionForBTB(propCtr, 0x28, orgArpNum);

						uint16_t seqLen = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						for (uint16_t l = 0; l < seqLen; ++l) {
							uint16_t data = propCtr.readUint16(arpCsr);
							arpCsr += 2;
							if (l == 0)
								instMan.lock()->setArpeggioFMSequenceCommand(arpNum, 0, data, 0);
							else
								instMan.lock()->addArpeggioFMSequenceCommand(arpNum, data, 0);
						}

						uint16_t loopCnt = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						if (loopCnt > 0) {
							std::vector<int> begins, ends, times;
							for (uint16_t l = 0; l < loopCnt; ++l) {
								begins.push_back(propCtr.readUint16(arpCsr));
								arpCsr += 2;
								ends.push_back(propCtr.readUint16(arpCsr));
								arpCsr += 2;
								times.push_back(propCtr.readUint8(arpCsr++));
							}
							instMan.lock()->setArpeggioFMLoops(arpNum, begins, ends, times);
						}

						switch (propCtr.readUint8(arpCsr++)) {
						case 0x00:	// No release
							instMan.lock()->setArpeggioFMRelease(arpNum, ReleaseType::NoRelease, -1);
							break;
						case 0x01:	// Fixed
						{
							uint16_t pos = propCtr.readUint16(arpCsr);
							arpCsr += 2;
							// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
							if (pos < seqLen) instMan.lock()->setArpeggioFMRelease(arpNum, ReleaseType::FixedRelease, pos);
							else instMan.lock()->setArpeggioFMRelease(arpNum, ReleaseType::NoRelease, -1);
							break;
						}
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
						}

						switch (propCtr.readUint8(arpCsr++)) {
						case 0x00:	// Absolute
							instMan.lock()->setArpeggioFMType(arpNum, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x01:	// Fixed
							instMan.lock()->setArpeggioFMType(arpNum, SequenceType::FIXED_SEQUENCE);
							break;
						case 0x02:	// Relative
							instMan.lock()->setArpeggioFMType(arpNum, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
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
			std::map<FMOperatorType, int> tmpMap;
			std::map<int, FMOperatorType> orgNumMap;
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
						int ptNum = instMan.lock()->findFirstFreePlainPitchFM();
						if (ptNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
						fm->setPitchNumber(pair.first, ptNum);
						size_t ptCsr = getPropertyPositionForBTB(propCtr, 0x29, orgPtNum);

						uint16_t seqLen = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						for (uint16_t l = 0; l < seqLen; ++l) {
							uint16_t data = propCtr.readUint16(ptCsr);
							ptCsr += 2;
							if (l == 0)
								instMan.lock()->setPitchFMSequenceCommand(ptNum, 0, data, 0);
							else
								instMan.lock()->addPitchFMSequenceCommand(ptNum, data, 0);
						}

						uint16_t loopCnt = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						if (loopCnt > 0) {
							std::vector<int> begins, ends, times;
							for (uint16_t l = 0; l < loopCnt; ++l) {
								begins.push_back(propCtr.readUint16(ptCsr));
								ptCsr += 2;
								ends.push_back(propCtr.readUint16(ptCsr));
								ptCsr += 2;
								times.push_back(propCtr.readUint8(ptCsr++));
							}
							instMan.lock()->setPitchFMLoops(ptNum, begins, ends, times);
						}

						switch (propCtr.readUint8(ptCsr++)) {
						case 0x00:	// No release
							instMan.lock()->setPitchFMRelease(ptNum, ReleaseType::NoRelease, -1);
							break;
						case 0x01:	// Fixed
						{
							uint16_t pos = propCtr.readUint16(ptCsr);
							ptCsr += 2;
							// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
							if (pos < seqLen) instMan.lock()->setPitchFMRelease(ptNum, ReleaseType::FixedRelease, pos);
							else instMan.lock()->setPitchFMRelease(ptNum, ReleaseType::NoRelease, -1);
							break;
						}
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
						}

						switch (propCtr.readUint8(ptCsr++)) {
						case 0x00:	// Absolute
							instMan.lock()->setPitchFMType(ptNum, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x02:	// Relative
							instMan.lock()->setPitchFMType(ptNum, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							throw FileCorruptionError(FileIO::FileType::Inst);
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

		return fm;
	}
	case 0x01:	// SSG
	{
		auto ssg = new InstrumentSSG(instNum, name, instMan.lock().get());

		/* Wave form */
		{
			uint8_t tmp = instCtr.readUint8(instCsr++);
			if (0x80 & tmp) {
				ssg->setWaveFormEnabled(false);
				ssg->setWaveFormNumber(0x7f & tmp);
			}
			else {
				ssg->setWaveFormEnabled(true);
				uint8_t orgWfNum = 0x7f & tmp;
				int wfNum = instMan.lock()->findFirstFreePlainWaveFormSSG();
				if (wfNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
				ssg->setWaveFormNumber(wfNum);
				size_t wfCsr = getPropertyPositionForBTB(propCtr, 0x30, orgWfNum);

				uint16_t seqLen = propCtr.readUint16(wfCsr);
				wfCsr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = propCtr.readUint16(wfCsr);
					wfCsr += 2;
					int32_t subdata;
					subdata = propCtr.readInt32(wfCsr);
					wfCsr += 4;
					if (l == 0)
						instMan.lock()->setWaveFormSSGSequenceCommand(wfNum, 0, data, subdata);
					else
						instMan.lock()->addWaveFormSSGSequenceCommand(wfNum, data, subdata);
				}

				uint16_t loopCnt = propCtr.readUint16(wfCsr);
				wfCsr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(propCtr.readUint16(wfCsr));
						wfCsr += 2;
						ends.push_back(propCtr.readUint16(wfCsr));
						wfCsr += 2;
						times.push_back(propCtr.readUint8(wfCsr++));
					}
					instMan.lock()->setWaveFormSSGLoops(wfNum, begins, ends, times);
				}

				switch (propCtr.readUint8(wfCsr++)) {
				case 0x00:	// No release
					instMan.lock()->setWaveFormSSGRelease(wfNum, ReleaseType::NoRelease, -1);
					break;
				case 0x01:	// Fixed
				{
					uint16_t pos = propCtr.readUint16(wfCsr);
					wfCsr += 2;
					// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
					if (pos < seqLen) instMan.lock()->setWaveFormSSGRelease(wfNum, ReleaseType::FixedRelease, pos);
					else instMan.lock()->setWaveFormSSGRelease(wfNum, ReleaseType::NoRelease, -1);
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
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
				int tnNum = instMan.lock()->findFirstFreePlainToneNoiseSSG();
				if (tnNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
				ssg->setToneNoiseNumber(tnNum);
				size_t tnCsr = getPropertyPositionForBTB(propCtr, 0x31, orgTnNum);

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
						instMan.lock()->setToneNoiseSSGSequenceCommand(tnNum, 0, data, 0);
					else
						instMan.lock()->addToneNoiseSSGSequenceCommand(tnNum, data, 0);
				}

				uint16_t loopCnt = propCtr.readUint16(tnCsr);
				tnCsr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(propCtr.readUint16(tnCsr));
						tnCsr += 2;
						ends.push_back(propCtr.readUint16(tnCsr));
						tnCsr += 2;
						times.push_back(propCtr.readUint8(tnCsr++));
					}
					instMan.lock()->setToneNoiseSSGLoops(tnNum, begins, ends, times);
				}

				switch (propCtr.readUint8(tnCsr++)) {
				case 0x00:	// No release
					instMan.lock()->setToneNoiseSSGRelease(tnNum, ReleaseType::NoRelease, -1);
					break;
				case 0x01:	// Fixed
				{
					uint16_t pos = propCtr.readUint16(tnCsr);
					tnCsr += 2;
					// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
					if (pos < seqLen) instMan.lock()->setToneNoiseSSGRelease(tnNum, ReleaseType::FixedRelease, pos);
					else instMan.lock()->setToneNoiseSSGRelease(tnNum, ReleaseType::NoRelease, -1);
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
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
				int envNum = instMan.lock()->findFirstFreePlainEnvelopeSSG();
				if (envNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
				ssg->setEnvelopeNumber(envNum);
				size_t envCsr = getPropertyPositionForBTB(propCtr, 0x32, orgEnvNum);

				uint16_t seqLen = propCtr.readUint16(envCsr);
				envCsr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = propCtr.readUint16(envCsr);
					envCsr += 2;
					int32_t subdata;
					subdata = propCtr.readInt32(envCsr);
					envCsr += 4;
					if (l == 0)
						instMan.lock()->setEnvelopeSSGSequenceCommand(envNum, 0, data, subdata);
					else
						instMan.lock()->addEnvelopeSSGSequenceCommand(envNum, data, subdata);
				}

				uint16_t loopCnt = propCtr.readUint16(envCsr);
				envCsr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(propCtr.readUint16(envCsr));
						envCsr += 2;
						ends.push_back(propCtr.readUint16(envCsr));
						envCsr += 2;
						times.push_back(propCtr.readUint8(envCsr++));
					}
					instMan.lock()->setEnvelopeSSGLoops(envNum, begins, ends, times);
				}

				switch (propCtr.readUint8(envCsr++)) {
				case 0x00:	// No release
					instMan.lock()->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
					break;
					// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
				case 0x01:	// Fixed
				{
					uint16_t pos = propCtr.readUint16(envCsr);
					envCsr += 2;
					if (pos < seqLen) instMan.lock()->setEnvelopeSSGRelease(envNum, ReleaseType::FixedRelease, pos);
					else instMan.lock()->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
					break;
				}
				case 0x02:	// Absolute
				{
					uint16_t pos = propCtr.readUint16(envCsr);
					envCsr += 2;
					if (pos < seqLen) instMan.lock()->setEnvelopeSSGRelease(envNum, ReleaseType::AbsoluteRelease, pos);
					else instMan.lock()->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
					break;
				}
				case 0x03:	// Relative
				{
					uint16_t pos = propCtr.readUint16(envCsr);
					envCsr += 2;
					if (pos < seqLen) instMan.lock()->setEnvelopeSSGRelease(envNum, ReleaseType::RelativeRelease, pos);
					else instMan.lock()->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
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
				int arpNum = instMan.lock()->findFirstFreePlainArpeggioSSG();
				if (arpNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
				ssg->setArpeggioNumber(arpNum);
				size_t arpCsr = getPropertyPositionForBTB(propCtr, 0x33, orgArpNum);

				uint16_t seqLen = propCtr.readUint16(arpCsr);
				arpCsr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					if (l == 0)
						instMan.lock()->setArpeggioSSGSequenceCommand(arpNum, 0, data, 0);
					else
						instMan.lock()->addArpeggioSSGSequenceCommand(arpNum, data, 0);
				}

				uint16_t loopCnt = propCtr.readUint16(arpCsr);
				arpCsr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(propCtr.readUint16(arpCsr));
						arpCsr += 2;
						ends.push_back(propCtr.readUint16(arpCsr));
						arpCsr += 2;
						times.push_back(propCtr.readUint8(arpCsr++));
					}
					instMan.lock()->setArpeggioSSGLoops(arpNum, begins, ends, times);
				}

				switch (propCtr.readUint8(arpCsr++)) {
				case 0x00:	// No release
					instMan.lock()->setArpeggioSSGRelease(arpNum, ReleaseType::NoRelease, -1);
					break;
				case 0x01:	// Fixed
				{
					uint16_t pos = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
					if (pos < seqLen) instMan.lock()->setArpeggioSSGRelease(arpNum, ReleaseType::FixedRelease, pos);
					else instMan.lock()->setArpeggioSSGRelease(arpNum, ReleaseType::NoRelease, -1);
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
				}

				switch (propCtr.readUint8(arpCsr++)) {
				case 0x00:	// Absolute
					instMan.lock()->setArpeggioSSGType(arpNum, SequenceType::ABSOLUTE_SEQUENCE);
					break;
				case 0x01:	// Fixed
					instMan.lock()->setArpeggioSSGType(arpNum, SequenceType::FIXED_SEQUENCE);
					break;
				case 0x02:	// Relative
					instMan.lock()->setArpeggioSSGType(arpNum, SequenceType::RELATIVE_SEQUENCE);
					break;
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
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
				int ptNum = instMan.lock()->findFirstFreePlainPitchSSG();
				if (ptNum == -1) throw FileCorruptionError(FileIO::FileType::Inst);
				ssg->setPitchNumber(ptNum);
				size_t ptCsr = getPropertyPositionForBTB(propCtr, 0x34, orgPtNum);

				uint16_t seqLen = propCtr.readUint16(ptCsr);
				ptCsr += 2;
				for (uint16_t l = 0; l < seqLen; ++l) {
					uint16_t data = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					if (l == 0)
						instMan.lock()->setPitchSSGSequenceCommand(ptNum, 0, data, 0);
					else
						instMan.lock()->addPitchSSGSequenceCommand(ptNum, data, 0);
				}

				uint16_t loopCnt = propCtr.readUint16(ptCsr);
				ptCsr += 2;
				if (loopCnt > 0) {
					std::vector<int> begins, ends, times;
					for (uint16_t l = 0; l < loopCnt; ++l) {
						begins.push_back(propCtr.readUint16(ptCsr));
						ptCsr += 2;
						ends.push_back(propCtr.readUint16(ptCsr));
						ptCsr += 2;
						times.push_back(propCtr.readUint8(ptCsr++));
					}
					instMan.lock()->setPitchSSGLoops(ptNum, begins, ends, times);
				}

				switch (propCtr.readUint8(ptCsr++)) {
				case 0x00:	// No release
					instMan.lock()->setPitchSSGRelease(ptNum, ReleaseType::NoRelease, -1);
					break;
				case 0x01:	// Fixed
				{
					uint16_t pos = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					// Release point check (prevents a bug; see rerrahkr/BambooTracker issue #11)
					if (pos < seqLen) instMan.lock()->setPitchSSGRelease(ptNum, ReleaseType::FixedRelease, pos);
					else instMan.lock()->setPitchSSGRelease(ptNum, ReleaseType::NoRelease, -1);
					break;
				}
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
				}

				switch (propCtr.readUint8(ptCsr++)) {
				case 0x00:	// Absolute
					instMan.lock()->setPitchSSGType(ptNum, SequenceType::ABSOLUTE_SEQUENCE);
					break;
				case 0x02:	// Relative
					instMan.lock()->setPitchSSGType(ptNum, SequenceType::RELATIVE_SEQUENCE);
					break;
				default:
					throw FileCorruptionError(FileIO::FileType::Inst);
				}
			}
		}

		return ssg;
	}
	default:
		throw FileCorruptionError(FileIO::FileType::Inst);
	}
}
