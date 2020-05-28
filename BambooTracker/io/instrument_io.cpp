#include "instrument_io.hpp"
#include "file_io.hpp"
#include <locale>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include "version.hpp"
#include "file_io.hpp"
#include "file_io_error.hpp"
#include "pitch_converter.hpp"
#include "enum_hash.hpp"
#include "misc.hpp"

InstrumentIO::InstrumentIO() {}

void InstrumentIO::saveInstrument(BinaryContainer& ctr,
								  const std::weak_ptr<InstrumentsManager> instMan, int instNum)
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
			ctr.appendUint8(keys.size());
			int sampCnt = 0;
			std::unordered_map<int, int> sampMap;
			for (const int& key : keys) {
				ctr.appendUint8(static_cast<uint8_t>(key));
				int samp = kit->getSampleNumber(key);
				if (!sampMap.count(samp)) sampMap[samp] = sampCnt++;
				ctr.appendUint8(static_cast<uint8_t>(sampMap[samp]));
				ctr.appendInt8(kit->getPitch(key));
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
			// Operator 1
			tmp = instManLocked->getEnvelopeFMOperatorEnabled(envNum, 0);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS1) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT1) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL1) << 4)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR1));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL1));
			ctr.appendUint8(tmp);
			int tmp2 = instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG1);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML1));
			ctr.appendUint8(tmp);
			// Operator 2
			tmp = instManLocked->getEnvelopeFMOperatorEnabled(envNum, 1);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS2) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT2) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL2) << 4)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR2));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL2));
			ctr.appendUint8(tmp);
			tmp2 = instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG2);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML2));
			ctr.appendUint8(tmp);
			// Operator 3
			tmp = instManLocked->getEnvelopeFMOperatorEnabled(envNum, 2);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS3) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT3) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL3) << 4)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR3));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL3));
			ctr.appendUint8(tmp);
			tmp2 = instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG3);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML3));
			ctr.appendUint8(tmp);
			// Operator 4
			tmp = instManLocked->getEnvelopeFMOperatorEnabled(envNum, 3);
			tmp = static_cast<uint8_t>(tmp << 5) | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS4) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT4) << 5)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL4) << 4)
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR4));
			ctr.appendUint8(tmp);
			tmp = static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL4));
			ctr.appendUint8(tmp);
			tmp2 = instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG4);
			tmp = ((tmp2 == -1) ? 0x80 : static_cast<uint8_t>(tmp2 << 4))
				  | static_cast<uint8_t>(instManLocked->getEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML4));
			ctr.appendUint8(tmp);
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
			if (instFM->getOperatorSequenceEnabled(FileIO::ENV_FM_PARAMS[i])) {
				int seqNum = instFM->getOperatorSequenceNumber(FileIO::ENV_FM_PARAMS[i]);
				ctr.appendUint8(0x02 + static_cast<uint8_t>(i));
				size_t ofs = ctr.size();
				ctr.appendUint16(0);	// Dummy offset
				auto seq = instManLocked->getOperatorSequenceFMSequence(FileIO::ENV_FM_PARAMS[i], seqNum);
				ctr.appendUint16(static_cast<uint16_t>(seq.size()));
				for (auto& com : seq) {
					ctr.appendUint16(static_cast<uint16_t>(com.type));
				}
				auto loop = instManLocked->getOperatorSequenceFMLoops(FileIO::ENV_FM_PARAMS[i], seqNum);
				ctr.appendUint16(static_cast<uint16_t>(loop.size()));
				for (auto& l : loop) {
					ctr.appendUint16(static_cast<uint16_t>(l.begin));
					ctr.appendUint16(static_cast<uint16_t>(l.end));
					ctr.appendUint8(static_cast<uint8_t>(l.times));
				}
				auto release = instManLocked->getOperatorSequenceFMRelease(FileIO::ENV_FM_PARAMS[i], seqNum);
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
			auto seq = instManLocked->getArpeggioFMSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instManLocked->getArpeggioFMLoops(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getArpeggioFMRelease(arpNum);
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
			switch (instManLocked->getArpeggioFMType(arpNum)) {
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
			auto seq = instManLocked->getPitchFMSequence(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instManLocked->getPitchFMLoops(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getPitchFMRelease(ptNum);
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
			switch (instManLocked->getPitchFMType(ptNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
			}
			auto loop = instManLocked->getWaveformSSGLoops(wfNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getWaveformSSGRelease(wfNum);
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
			auto seq = instManLocked->getToneNoiseSSGSequence(tnNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instManLocked->getToneNoiseSSGLoops(tnNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getToneNoiseSSGRelease(tnNum);
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
			auto seq = instManLocked->getEnvelopeSSGSequence(envNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
			}
			auto loop = instManLocked->getEnvelopeSSGLoops(envNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getEnvelopeSSGRelease(envNum);
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
			auto seq = instManLocked->getArpeggioSSGSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instManLocked->getArpeggioSSGLoops(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getArpeggioSSGRelease(arpNum);
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
			switch (instManLocked->getArpeggioSSGType(arpNum)) {
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
			auto seq = instManLocked->getPitchSSGSequence(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instManLocked->getPitchSSGLoops(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getPitchSSGRelease(ptNum);
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
			switch (instManLocked->getPitchSSGType(ptNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
			}
			auto loop = instManLocked->getEnvelopeADPCMLoops(envNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getEnvelopeADPCMRelease(envNum);
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

		// ADPCM arpeggio
		if (instADPCM->getArpeggioEnabled()) {
			int arpNum = instADPCM->getArpeggioNumber();
			ctr.appendUint8(0x42);
			size_t ofs = ctr.size();
			ctr.appendUint16(0);	// Dummy offset
			auto seq = instManLocked->getArpeggioADPCMSequence(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(seq.size()));
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instManLocked->getArpeggioADPCMLoops(arpNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getArpeggioADPCMRelease(arpNum);
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
			switch (instManLocked->getArpeggioADPCMType(arpNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::FIXED_SEQUENCE:		ctr.appendUint8(0x01);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instManLocked->getPitchADPCMLoops(ptNum);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instManLocked->getPitchADPCMRelease(ptNum);
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
			switch (instManLocked->getPitchADPCMType(ptNum)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
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

AbstractInstrument* InstrumentIO::loadInstrument(const BinaryContainer& ctr, const std::string& path,
												 std::weak_ptr<InstrumentsManager> instMan,
												 int instNum)
{
	std::string ext = FileIO::getExtension(path);
	if (ext.compare("dmp") == 0) return InstrumentIO::loadDMPFile(ctr, path, instMan, instNum);
	if (ext.compare("tfi") == 0) return InstrumentIO::loadTFIFile(ctr, path, instMan, instNum);
	if (ext.compare("vgi") == 0) return InstrumentIO::loadVGIFile(ctr, path, instMan, instNum);
	if (ext.compare("opni") == 0) return InstrumentIO::loadOPNIFile(ctr, instMan, instNum);
	if (ext.compare("y12") == 0) return InstrumentIO::loadY12File(ctr, path, instMan, instNum);
	if (ext.compare("ins") == 0) return InstrumentIO::loadINSFile(ctr, instMan, instNum);
	if (ext.compare("bti") == 0) return InstrumentIO::loadBTIFile(ctr, instMan, instNum);
	throw FileInputError(FileIO::FileType::Inst);
}

AbstractInstrument* InstrumentIO::loadBTIFile(const BinaryContainer& ctr,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerIst")
		throw FileCorruptionError(FileIO::FileType::Inst);
	globCsr += 16;
	/*size_t eofOfs = */ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofInstrumentFileInBCD())
		throw FileVersionError(FileIO::FileType::Inst);
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
			int kitSampCnt = 0;
			int adpcmSampIdx = 0;
			std::vector<int> nums;
			// Check memory range
			while (instPropCsr < globCsr) {
				switch (ctr.readUint8(instPropCsr++)) {
				case 0x00:	// FM envelope
				{
					nums.push_back(instManLocked->findFirstAssignableEnvelopeFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x01:	// FM LFO
				{
					nums.push_back(instManLocked->findFirstAssignableLFOFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint8(instPropCsr);
					break;
				}
				case 0x02:	// FM AL
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AL));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x03:	// FM FB
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::FB));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x04:	// FM AR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x05:	// FM DR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x06:	// FM SR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x07:	// FM RR1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x08:	// FM SL1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x09:	// FM TL1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0a:	// FM KS1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0b:	// FM ML1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0c:	// FM DT1
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT1));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0d:	// FM AR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0e:	// FM DR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x0f:	// FM SR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x10:	// FM RR2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x11:	// FM SL2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x12:	// FM TL2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x13:	// FM KS2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x14:	// FM ML2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x15:	// FM DT2
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT2));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x16:	// FM AR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x17:	// FM DR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x18:	// FM SR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x19:	// FM RR3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1a:	// FM SL3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1b:	// FM TL3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1c:	// FM KS3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1d:	// FM ML3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1e:	// FM DT3
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT3));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x1f:	// FM AR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::AR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x20:	// FM DR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x21:	// FM SR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x22:	// FM RR4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::RR4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x23:	// FM SL4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::SL4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x24:	// FM TL4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::TL4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x25:	// FM KS4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::KS4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x26:	// FM ML4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::ML4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x27:	// FM DT4
				{
					nums.push_back(instManLocked->findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter::DT4));
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x28:	// FM arpeggio
				{
					nums.push_back(instManLocked->findFirstAssignableArpeggioFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x29:	// FM pitch
				{
					nums.push_back(instManLocked->findFirstAssignablePitchFM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x30:	// SSG waveform
				{
					nums.push_back(instManLocked->findFirstAssignableWaveformSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x31:	// SSG tone/noise
				{
					nums.push_back(instManLocked->findFirstAssignableToneNoiseSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x32:	// SSG envelope
				{
					nums.push_back(instManLocked->findFirstAssignableEnvelopeSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x33:	// SSG arpeggio
				{
					nums.push_back(instManLocked->findFirstAssignableArpeggioSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x34:	// SSG pitch
				{
					nums.push_back(instManLocked->findFirstAssignablePitchSSG());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x40:	// ADPCM sample
				{
					adpcmSampIdx = instManLocked->findFirstAssignableSampleADPCM(adpcmSampIdx);
					if (adpcmSampIdx == -1) throw FileCorruptionError(FileIO::FileType::Inst);
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
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x42:	// ADPCM arpeggio
				{
					nums.push_back(instManLocked->findFirstAssignableArpeggioADPCM());
					if (nums.back() == -1) throw FileCorruptionError(FileIO::FileType::Inst);
					instPropCsr += ctr.readUint16(instPropCsr);
					break;
				}
				case 0x43:	// ADPCM pitch
				{
					nums.push_back(instManLocked->findFirstAssignablePitchADPCM());
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
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AL, tmp >> 4);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::FB, tmp & 0x0f);
					// Operator 1
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMOperatorEnabled(idx, 0, (0x20 & tmp) ? true : false);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR1, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS1, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR1, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT1, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR1, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL1, tmp >> 4);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR1, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL1, tmp);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML1, tmp & 0x0f);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG1,
														  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
					// Operator 2
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMOperatorEnabled(idx, 1, (0x20 & tmp) ? true : false);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR2, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS2, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR2, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT2, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR2, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL2, tmp >> 4);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR2, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL2, tmp);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML2, tmp & 0x0f);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG2,
														  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
					// Operator 3
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMOperatorEnabled(idx, 2, (0x20 & tmp) ? true : false);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR3, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS3, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR3, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT3, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR3, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL3, tmp >> 4);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR3, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL3, tmp);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML3, tmp & 0x0f);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG3,
														  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
					// Operator 4
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMOperatorEnabled(idx, 3, (0x20 & tmp) ? true : false);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::AR4, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::KS4, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DR4, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::DT4, tmp >> 5);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SR4, tmp & 0x1f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SL4, tmp >> 4);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::RR4, tmp & 0x0f);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::TL4, tmp);
					tmp = ctr.readUint8(csr++);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::ML4, tmp & 0x0f);
					instManLocked->setEnvelopeFMParameter(idx, FMEnvelopeParameter::SSGEG4,
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
							instManLocked->setArpeggioFMSequenceCommand(idx, 0, data, 0);
						else
							instManLocked->addArpeggioFMSequenceCommand(idx, data, 0);
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
						instManLocked->setArpeggioFMLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioFMRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setArpeggioFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setArpeggioFMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x01:	// Fixed
							instManLocked->setArpeggioFMType(idx, SequenceType::FIXED_SEQUENCE);
							break;
						case 0x02:	// Relative
							instManLocked->setArpeggioFMType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setArpeggioFMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
								break;
							}
							else {
								throw FileCorruptionError(FileIO::FileType::Inst);
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
							instManLocked->setPitchFMSequenceCommand(idx, 0, data, 0);
						else
							instManLocked->addPitchFMSequenceCommand(idx, data, 0);
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
						instManLocked->setPitchFMLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPitchFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchFMRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setPitchFMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setPitchFMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x02:	// Relative
							instManLocked->setPitchFMType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setPitchFMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
								break;
							}
							else {
								throw FileCorruptionError(FileIO::FileType::Inst);
							}
						}
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
							if (data == 3) data = static_cast<int>(SSGWaveformType::SQM_TRIANGLE);
							else if (data == 4) data = static_cast<int>(SSGWaveformType::SQM_SAW);
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
							instManLocked->setWaveformSSGSequenceCommand(idx, 0, data, subdata);
						else
							instManLocked->addWaveformSSGSequenceCommand(idx, data, subdata);
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
						instManLocked->setWaveformSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setWaveformSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setWaveformSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setWaveformSSGRelease(idx, ReleaseType::NoRelease, -1);
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
							instManLocked->setToneNoiseSSGSequenceCommand(idx, 0, data, 0);
						else
							instManLocked->addToneNoiseSSGSequenceCommand(idx, data, 0);
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
						instManLocked->setToneNoiseSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setToneNoiseSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setToneNoiseSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setToneNoiseSSGRelease(idx, ReleaseType::NoRelease, -1);
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
							instManLocked->setEnvelopeSSGSequenceCommand(idx, 0, data, subdata);
						else
							instManLocked->addEnvelopeSSGSequenceCommand(idx, data, subdata);
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
						instManLocked->setEnvelopeSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(idx, ReleaseType::AbsoluteRelease, pos);
						else instManLocked->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(idx, ReleaseType::RelativeRelease, pos);
						else instManLocked->setEnvelopeSSGRelease(idx, ReleaseType::NoRelease, -1);
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
							instManLocked->setArpeggioSSGSequenceCommand(idx, 0, data, 0);
						else
							instManLocked->addArpeggioSSGSequenceCommand(idx, data, 0);
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
						instManLocked->setArpeggioSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setArpeggioSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setArpeggioSSGType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x01:	// Fixed
							instManLocked->setArpeggioSSGType(idx, SequenceType::FIXED_SEQUENCE);
							break;
						case 0x02:	// Relative
							instManLocked->setArpeggioSSGType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setArpeggioSSGType(idx, SequenceType::ABSOLUTE_SEQUENCE);
								break;
							}
							else {
								throw FileCorruptionError(FileIO::FileType::Inst);
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
							instManLocked->setPitchSSGSequenceCommand(idx, 0, data, 0);
						else
							instManLocked->addPitchSSGSequenceCommand(idx, data, 0);
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
						instManLocked->setPitchSSGLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPitchSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchSSGRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setPitchSSGRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					if (fileVersion >= Version::toBCD(1, 0, 1)) {
						switch (ctr.readUint8(csr++)) {
						case 0x00:	// Absolute
							instManLocked->setPitchSSGType(idx, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						case 0x02:	// Relative
							instManLocked->setPitchSSGType(idx, SequenceType::RELATIVE_SEQUENCE);
							break;
						default:
							if (fileVersion < Version::toBCD(1, 2, 3)) {
								// Recover deep clone bug
								// https://github.com/rerrahkr/BambooTracker/issues/170
								instManLocked->setPitchSSGType(idx, SequenceType::ABSOLUTE_SEQUENCE);
								break;
							}
							else {
								throw FileCorruptionError(FileIO::FileType::Inst);
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
					csr += len;
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
							instManLocked->setEnvelopeADPCMSequenceCommand(idx, 0, data, subdata);
						else
							instManLocked->addEnvelopeADPCMSequenceCommand(idx, data, subdata);
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
						instManLocked->setEnvelopeADPCMLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setEnvelopeADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(idx, ReleaseType::AbsoluteRelease, pos);
						else instManLocked->setEnvelopeADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(idx, ReleaseType::RelativeRelease, pos);
						else instManLocked->setEnvelopeADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
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
							instManLocked->setArpeggioADPCMSequenceCommand(idx, 0, data, 0);
						else
							instManLocked->addArpeggioADPCMSequenceCommand(idx, data, 0);
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
						instManLocked->setArpeggioADPCMLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioADPCMRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setArpeggioADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// Absolute
						instManLocked->setArpeggioADPCMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
						break;
					case 0x01:	// Fixed
						instManLocked->setArpeggioADPCMType(idx, SequenceType::FIXED_SEQUENCE);
						break;
					case 0x02:	// Relative
						instManLocked->setArpeggioADPCMType(idx, SequenceType::RELATIVE_SEQUENCE);
						break;
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
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
							instManLocked->setPitchADPCMSequenceCommand(idx, 0, data, 0);
						else
							instManLocked->addPitchADPCMSequenceCommand(idx, data, 0);
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
						instManLocked->setPitchADPCMLoops(idx, begins, ends, times);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// No release
						instManLocked->setPitchADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = ctr.readUint16(csr);
						csr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchADPCMRelease(idx, ReleaseType::FixedRelease, pos);
						else instManLocked->setPitchADPCMRelease(idx, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
					}

					switch (ctr.readUint8(csr++)) {
					case 0x00:	// Absolute
						instManLocked->setPitchADPCMType(idx, SequenceType::ABSOLUTE_SEQUENCE);
						break;
					case 0x02:	// Relative
						instManLocked->setPitchADPCMType(idx, SequenceType::RELATIVE_SEQUENCE);
						break;
					default:
						throw FileCorruptionError(FileIO::FileType::Inst);
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
			instManLocked->setOperatorSequenceFMSequenceCommand(param, idx, 0, data, 0);
		else
			instManLocked->addOperatorSequenceFMSequenceCommand(param, idx, data, 0);
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
		instManLocked->setOperatorSequenceFMLoops(param, idx, begins, ends, times);
	}

	switch (ctr.readUint8(csr++)) {
	case 0x00:	// No release
		instManLocked->setOperatorSequenceFMRelease(param, idx, ReleaseType::NoRelease, -1);
		break;
	case 0x01:	// Fixed
	{
		uint16_t pos = ctr.readUint16(csr);
		csr += 2;
		// Release point check (prevents a bug)
		// https://github.com/rerrahkr/BambooTracker/issues/11
		if (pos < seqLen) instManLocked->setOperatorSequenceFMRelease(param, idx, ReleaseType::FixedRelease, pos);
		else instManLocked->setOperatorSequenceFMRelease(param, idx, ReleaseType::NoRelease, -1);
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

AbstractInstrument* InstrumentIO::loadDMPFile(const BinaryContainer& ctr, std::string path,
											  std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
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
		inst = new InstrumentSSG(instNum, name, instManLocked.get());
		auto ssg = dynamic_cast<InstrumentSSG*>(inst);
		uint8_t envSize = ctr.readUint8(csr++);
		if (envSize > 0) {
			int idx = instManLocked->findFirstAssignableEnvelopeSSG();
			if (idx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
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
			if (idx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
			ssg->setArpeggioEnabled(true);
			ssg->setArpeggioNumber(idx);
			uint8_t arpType = ctr.readUint8(csr + arpSize * 4 + 1);
			if (arpType == 1) instManLocked->setArpeggioSSGType(idx, SequenceType::FIXED_SEQUENCE);
			for (uint8_t l = 0; l < arpSize; ++l) {
				int data = ctr.readInt32(csr) + 36;
				csr += 4;
				if (arpType == 1) data -= 24;
				if (l == 0) instManLocked->setArpeggioSSGSequenceCommand(idx, 0, data, 0);
				else instManLocked->addArpeggioSSGSequenceCommand(idx, data, 0);
			}
			int8_t loop = ctr.readInt8(csr++);
			if (loop >= 0) instManLocked->setArpeggioSSGLoops(idx, {loop}, {arpSize - 1}, {1});
		}
		break;
	}
	case 0x01:	// FM
	{
		int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
		if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
		inst = new InstrumentFM(instNum, name, instManLocked.get());
		auto fm = dynamic_cast<InstrumentFM*>(inst);
		fm->setEnvelopeNumber(envIdx);
		if (fileVersion == 9) csr++; // skip version 9's total operators field
		uint8_t pms = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
		uint8_t ams = ctr.readUint8(csr++);

		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, ctr.readUint8(csr++));
		uint8_t am1 = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, convertDTInTFIVGIDMP(ctr.readUint8(csr++) & 15)); // mask out OPM's DT2
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
		int ssgeg1 = ctr.readUint8(csr++);
		ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);

		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, ctr.readUint8(csr++));
		uint8_t am3 = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, convertDTInTFIVGIDMP(ctr.readUint8(csr++) & 15));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
		int ssgeg3 = ctr.readUint8(csr++);
		ssgeg3 = ssgeg3 & 8 ? ssgeg3 & 7 : -1;
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);

		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, ctr.readUint8(csr++));
		uint8_t am2 = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, convertDTInTFIVGIDMP(ctr.readUint8(csr++) & 15));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
		int ssgeg2 = ctr.readUint8(csr++);
		ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);

		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, ctr.readUint8(csr++));
		uint8_t am4 = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, convertDTInTFIVGIDMP(ctr.readUint8(csr++) & 15));
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
		int ssgeg4 = ctr.readUint8(csr++);
		ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
		instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);

		if (pms || ams) {
			int lfoIdx = instManLocked->findFirstAssignableLFOFM();
			if (lfoIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
			fm->setLFOEnabled(true);
			fm->setLFONumber(lfoIdx);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, pms);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, ams);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, am1);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, am2);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, am3);
			instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, am4);
		}
		break;
	}
	}
	return inst;
}

size_t InstrumentIO::getPropertyPositionForBTB(const BinaryContainer& propCtr,
											   uint8_t subsecType, uint8_t index)
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

int InstrumentIO::convertDTInTFIVGIDMP(int dt)
{
	switch (dt) {
	case 0:		return 7;
	case 1:		return 6;
	case 2:		return 5;
	case 3:		return 0;
	case 4:		return 1;
	case 5:		return 2;
	case 6:		return 3;
	case 7:		return 3;
	default:	throw std::out_of_range("Out of range dt");
	}
}

AbstractInstrument* InstrumentIO::loadTFIFile(const BinaryContainer& ctr, std::string path,
											  std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	if (ctr.size() != 42) throw FileCorruptionError(FileIO::FileType::Inst);
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, ctr.readUint8(csr++));
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, ctr.readUint8(csr++));
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, ctr.readUint8(csr++));
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, ctr.readUint8(csr++));
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);
	return inst;
}

AbstractInstrument* InstrumentIO::loadVGIFile(const BinaryContainer& ctr, std::string path,
											  std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	if (ctr.size() != 43) throw FileCorruptionError(FileIO::FileType::Inst);
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
	uint8_t pams = ctr.readUint8(csr++);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, ctr.readUint8(csr++));
	uint8_t drams1 = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, drams1 & 31);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, ctr.readUint8(csr++));
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, ctr.readUint8(csr++));
	uint8_t drams3 = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, drams3 & 31);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, ctr.readUint8(csr++));
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, ctr.readUint8(csr++));
	uint8_t drams2 = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, drams2 & 31);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, ctr.readUint8(csr++));
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, convertDTInTFIVGIDMP(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, ctr.readUint8(csr++));
	uint8_t drams4 = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, drams4 & 31);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, ctr.readUint8(csr++));
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);

	if (pams != 0) {
		int lfoIdx = instManLocked->findFirstAssignableLFOFM();
		if (lfoIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
		inst->setLFOEnabled(true);
		inst->setLFONumber(lfoIdx);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, pams & 7);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, pams >> 4);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, drams1 >> 7);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, drams2 >> 7);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, drams3 >> 7);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, drams4 >> 7);
	}
	return inst;
}

AbstractInstrument* InstrumentIO::loadOPNIFile(const BinaryContainer& ctr,
											   std::weak_ptr<InstrumentsManager> instMan, int instNum)
{
	OPNIFile opni;
	if (WOPN_LoadInstFromMem(&opni, const_cast<char*>(ctr.getPointer()), static_cast<size_t>(ctr.size())) != 0)
		throw FileCorruptionError(FileIO::FileType::Inst);

	return loadWOPNInstrument(opni.inst, instMan, instNum);
}

AbstractInstrument* InstrumentIO::loadY12File(const BinaryContainer& ctr, std::string path,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	if (ctr.size() != 128) throw FileCorruptionError(FileIO::FileType::Inst);
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);
	size_t fnpos = path.find_last_of("/");
	std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	uint8_t tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg3 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);
	csr += 9;

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
	csr += 14;

	return inst;
}

AbstractInstrument* InstrumentIO::loadINSFile(const BinaryContainer& ctr,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
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

	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst);

	InstrumentFM* inst = new InstrumentFM(instNum, name, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	uint8_t tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, 0x07 & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, tmp >> 3);

	return inst;
}

AbstractInstrument* InstrumentIO::loadWOPNInstrument(const WOPNInstrument &srcInst,
													 std::weak_ptr<InstrumentsManager> instMan,
													 int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Bank);
	const char *name = srcInst.inst_name;

	InstrumentFM* inst = new InstrumentFM(instNum, name, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, srcInst.fbalg & 7);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, (srcInst.fbalg >> 3) & 7);

	const WOPNOperator *op[4] = {
		&srcInst.operators[0],
		&srcInst.operators[2],
		&srcInst.operators[1],
		&srcInst.operators[3],
	};

#define LOAD_OPERATOR(n)						\
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML##n, op[n - 1]->dtfm_30 & 15); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT##n, (op[n - 1]->dtfm_30 >> 4) & 7); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL##n, op[n - 1]->level_40); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS##n, op[n - 1]->rsatk_50 >> 6); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR##n, op[n - 1]->rsatk_50 & 31); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR##n, op[n - 1]->amdecay1_60 & 31); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR##n, op[n - 1]->decay2_70 & 31); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR##n, op[n - 1]->susrel_80 & 15); \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL##n, op[n - 1]->susrel_80 >> 4); \
	int ssgeg##n = op[n - 1]->ssgeg_90; \
	ssgeg##n = ssgeg##n & 8 ? ssgeg##n & 7 : -1; \
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG##n, ssgeg##n); \
	int am##n = op[n - 1]->amdecay1_60 >> 7;

	LOAD_OPERATOR(1)
			LOAD_OPERATOR(2)
			LOAD_OPERATOR(3)
			LOAD_OPERATOR(4)

		#undef LOAD_OPERATOR

			if (srcInst.lfosens != 0) {
		int lfoIdx = instManLocked->findFirstAssignableLFOFM();
		if (lfoIdx < 0) throw FileCorruptionError(FileIO::FileType::Bank);
		inst->setLFOEnabled(true);
		inst->setLFONumber(lfoIdx);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::PMS, srcInst.lfosens & 7);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AMS, (srcInst.lfosens >> 4) & 3);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM1, am1);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM2, am2);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM3, am3);
		instManLocked->setLFOFMParameter(lfoIdx, FMLFOParameter::AM4, am4);
	}

	if (srcInst.note_offset != 0) {
		int arpIdx = instManLocked->findFirstAssignableArpeggioFM();
		if (arpIdx < 0) throw FileCorruptionError(FileIO::FileType::Bank);
		inst->setArpeggioEnabled(FMOperatorType::All, true);
		inst->setArpeggioNumber(FMOperatorType::All, arpIdx);
		instManLocked->setArpeggioFMSequenceCommand(arpIdx, 0, srcInst.note_offset + 48, -1);
		instManLocked->setArpeggioFMType(arpIdx, SequenceType::ABSOLUTE_SEQUENCE);
	}

	return inst;
}

AbstractInstrument* InstrumentIO::loadBTBInstrument(const BinaryContainer& instCtr,
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
			if (envNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
			fm->setEnvelopeNumber(envNum);
			size_t envCsr = getPropertyPositionForBTB(propCtr, 0x00, orgEnvNum);
			if (envCsr != std::numeric_limits<size_t>::max()) {
				uint8_t tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AL, tmp >> 4);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::FB, tmp & 0x0f);
				// Operator 1
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMOperatorEnabled(envNum, 0, (0x20 & tmp) ? true : false);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR1, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS1, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR1, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT1, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR1, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL1, tmp >> 4);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR1, tmp & 0x0f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL1, tmp);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML1, tmp & 0x0f);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG1,
													  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				// Operator 2
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMOperatorEnabled(envNum, 1, (0x20 & tmp) ? true : false);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR2, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS2, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR2, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT2, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR2, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL2, tmp >> 4);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR2, tmp & 0x0f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL2, tmp);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML2, tmp & 0x0f);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG2,
													  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				// Operator 3
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMOperatorEnabled(envNum, 2, (0x20 & tmp) ? true : false);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR3, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS3, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR3, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT3, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR3, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL3, tmp >> 4);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR3, tmp & 0x0f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL3, tmp);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML3, tmp & 0x0f);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG3,
													  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
				// Operator 4
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMOperatorEnabled(envNum, 3, (0x20 & tmp) ? true : false);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::AR4, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::KS4, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DR4, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::DT4, tmp >> 5);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SR4, tmp & 0x1f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SL4, tmp >> 4);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::RR4, tmp & 0x0f);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::TL4, tmp);
				tmp = propCtr.readUint8(envCsr++);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::ML4, tmp & 0x0f);
				instManLocked->setEnvelopeFMParameter(envNum, FMEnvelopeParameter::SSGEG4,
													  (tmp & 0x80) ? -1 : ((tmp >> 4) & 0x07));
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
				if (lfoNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				fm->setLFONumber(lfoNum);
				size_t lfoCsr = getPropertyPositionForBTB(propCtr, 0x01, orgLFONum);
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
				int opSeqNum = instManLocked->findFirstAssignableOperatorSequenceFM(param);
				if (opSeqNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				fm->setOperatorSequenceNumber(param, opSeqNum);
				size_t opSeqCsr = getPropertyPositionForBTB(propCtr, 0x02 + tmpCnt, orgOpSeqNum);

				if (opSeqCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(opSeqCsr);
					opSeqCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(opSeqCsr);
						opSeqCsr += 2;
						if (l == 0)
							instManLocked->setOperatorSequenceFMSequenceCommand(param, opSeqNum, 0, data, 0);
						else
							instManLocked->addOperatorSequenceFMSequenceCommand(param, opSeqNum, data, 0);
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
						instManLocked->setOperatorSequenceFMLoops(param, opSeqNum, begins, ends, times);
					}

					switch (propCtr.readUint8(opSeqCsr++)) {
					case 0x00:	// No release
						instManLocked->setOperatorSequenceFMRelease(param, opSeqNum, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(opSeqCsr);
						opSeqCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setOperatorSequenceFMRelease(param, opSeqNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setOperatorSequenceFMRelease(param, opSeqNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
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
						if (arpNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
						fm->setArpeggioNumber(pair.first, arpNum);
						size_t arpCsr = getPropertyPositionForBTB(propCtr, 0x28, orgArpNum);

						if (arpCsr != std::numeric_limits<size_t>::max()) {
							uint16_t seqLen = propCtr.readUint16(arpCsr);
							arpCsr += 2;
							for (uint16_t l = 0; l < seqLen; ++l) {
								uint16_t data = propCtr.readUint16(arpCsr);
								arpCsr += 2;
								if (l == 0)
									instManLocked->setArpeggioFMSequenceCommand(arpNum, 0, data, 0);
								else
									instManLocked->addArpeggioFMSequenceCommand(arpNum, data, 0);
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
								instManLocked->setArpeggioFMLoops(arpNum, begins, ends, times);
							}

							switch (propCtr.readUint8(arpCsr++)) {
							case 0x00:	// No release
								instManLocked->setArpeggioFMRelease(arpNum, ReleaseType::NoRelease, -1);
								break;
							case 0x01:	// Fixed
							{
								uint16_t pos = propCtr.readUint16(arpCsr);
								arpCsr += 2;
								// Release point check (prevents a bug)
								// https://github.com/rerrahkr/BambooTracker/issues/11
								if (pos < seqLen) instManLocked->setArpeggioFMRelease(arpNum, ReleaseType::FixedRelease, pos);
								else instManLocked->setArpeggioFMRelease(arpNum, ReleaseType::NoRelease, -1);
								break;
							}
							default:
								throw FileCorruptionError(FileIO::FileType::Bank);
							}

							switch (propCtr.readUint8(arpCsr++)) {
							case 0x00:	// Absolute
								instManLocked->setArpeggioFMType(arpNum, SequenceType::ABSOLUTE_SEQUENCE);
								break;
							case 0x01:	// Fixed
								instManLocked->setArpeggioFMType(arpNum, SequenceType::FIXED_SEQUENCE);
								break;
							case 0x02:	// Relative
								instManLocked->setArpeggioFMType(arpNum, SequenceType::RELATIVE_SEQUENCE);
								break;
							default:
								if (bankVersion < Version::toBCD(1, 0, 2)) {
									// Recover deep clone bug
									// https://github.com/rerrahkr/BambooTracker/issues/170
									instManLocked->setArpeggioFMType(arpNum, SequenceType::ABSOLUTE_SEQUENCE);
									break;
								}
								else {
									throw FileCorruptionError(FileIO::FileType::Bank);
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
						if (ptNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
						fm->setPitchNumber(pair.first, ptNum);
						size_t ptCsr = getPropertyPositionForBTB(propCtr, 0x29, orgPtNum);

						if (ptCsr != std::numeric_limits<size_t>::max()) {
							uint16_t seqLen = propCtr.readUint16(ptCsr);
							ptCsr += 2;
							for (uint16_t l = 0; l < seqLen; ++l) {
								uint16_t data = propCtr.readUint16(ptCsr);
								ptCsr += 2;
								if (l == 0)
									instManLocked->setPitchFMSequenceCommand(ptNum, 0, data, 0);
								else
									instManLocked->addPitchFMSequenceCommand(ptNum, data, 0);
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
								instManLocked->setPitchFMLoops(ptNum, begins, ends, times);
							}

							switch (propCtr.readUint8(ptCsr++)) {
							case 0x00:	// No release
								instManLocked->setPitchFMRelease(ptNum, ReleaseType::NoRelease, -1);
								break;
							case 0x01:	// Fixed
							{
								uint16_t pos = propCtr.readUint16(ptCsr);
								ptCsr += 2;
								// Release point check (prevents a bug)
								// https://github.com/rerrahkr/BambooTracker/issues/11
								if (pos < seqLen) instManLocked->setPitchFMRelease(ptNum, ReleaseType::FixedRelease, pos);
								else instManLocked->setPitchFMRelease(ptNum, ReleaseType::NoRelease, -1);
								break;
							}
							default:
								throw FileCorruptionError(FileIO::FileType::Bank);
							}

							switch (propCtr.readUint8(ptCsr++)) {
							case 0x00:	// Absolute
								instManLocked->setPitchFMType(ptNum, SequenceType::ABSOLUTE_SEQUENCE);
								break;
							case 0x02:	// Relative
								instManLocked->setPitchFMType(ptNum, SequenceType::RELATIVE_SEQUENCE);
								break;
							default:
								if (bankVersion < Version::toBCD(1, 0, 2)) {
									// Recover deep clone bug
									// https://github.com/rerrahkr/BambooTracker/issues/170
									instManLocked->setPitchFMType(ptNum, SequenceType::ABSOLUTE_SEQUENCE);
									break;
								}
								else {
									throw FileCorruptionError(FileIO::FileType::Bank);
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
				if (wfNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				ssg->setWaveformNumber(wfNum);
				size_t wfCsr = getPropertyPositionForBTB(propCtr, 0x30, orgWfNum);

				if (wfCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(wfCsr);
					wfCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(wfCsr);
						wfCsr += 2;
						int32_t subdata;
						subdata = propCtr.readInt32(wfCsr);
						wfCsr += 4;
						if (l == 0)
							instManLocked->setWaveformSSGSequenceCommand(wfNum, 0, data, subdata);
						else
							instManLocked->addWaveformSSGSequenceCommand(wfNum, data, subdata);
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
						instManLocked->setWaveformSSGLoops(wfNum, begins, ends, times);
					}

					switch (propCtr.readUint8(wfCsr++)) {
					case 0x00:	// No release
						instManLocked->setWaveformSSGRelease(wfNum, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(wfCsr);
						wfCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setWaveformSSGRelease(wfNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setWaveformSSGRelease(wfNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
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
				if (tnNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				ssg->setToneNoiseNumber(tnNum);
				size_t tnCsr = getPropertyPositionForBTB(propCtr, 0x31, orgTnNum);

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
							instManLocked->setToneNoiseSSGSequenceCommand(tnNum, 0, data, 0);
						else
							instManLocked->addToneNoiseSSGSequenceCommand(tnNum, data, 0);
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
						instManLocked->setToneNoiseSSGLoops(tnNum, begins, ends, times);
					}

					switch (propCtr.readUint8(tnCsr++)) {
					case 0x00:	// No release
						instManLocked->setToneNoiseSSGRelease(tnNum, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(tnCsr);
						tnCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setToneNoiseSSGRelease(tnNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setToneNoiseSSGRelease(tnNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
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
				if (envNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				ssg->setEnvelopeNumber(envNum);
				size_t envCsr = getPropertyPositionForBTB(propCtr, 0x32, orgEnvNum);

				if (envCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(envCsr);
					envCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(envCsr);
						envCsr += 2;
						int32_t subdata;
						subdata = propCtr.readInt32(envCsr);
						envCsr += 4;
						if (l == 0)
							instManLocked->setEnvelopeSSGSequenceCommand(envNum, 0, data, subdata);
						else
							instManLocked->addEnvelopeSSGSequenceCommand(envNum, data, subdata);
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
						instManLocked->setEnvelopeSSGLoops(envNum, begins, ends, times);
					}

					switch (propCtr.readUint8(envCsr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
						break;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(envNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(envNum, ReleaseType::AbsoluteRelease, pos);
						else instManLocked->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeSSGRelease(envNum, ReleaseType::RelativeRelease, pos);
						else instManLocked->setEnvelopeSSGRelease(envNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
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
				if (arpNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				ssg->setArpeggioNumber(arpNum);
				size_t arpCsr = getPropertyPositionForBTB(propCtr, 0x33, orgArpNum);

				if (arpCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						if (l == 0)
							instManLocked->setArpeggioSSGSequenceCommand(arpNum, 0, data, 0);
						else
							instManLocked->addArpeggioSSGSequenceCommand(arpNum, data, 0);
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
						instManLocked->setArpeggioSSGLoops(arpNum, begins, ends, times);
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioSSGRelease(arpNum, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioSSGRelease(arpNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setArpeggioSSGRelease(arpNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setArpeggioSSGType(arpNum, SequenceType::ABSOLUTE_SEQUENCE);
						break;
					case 0x01:	// Fixed
						instManLocked->setArpeggioSSGType(arpNum, SequenceType::FIXED_SEQUENCE);
						break;
					case 0x02:	// Relative
						instManLocked->setArpeggioSSGType(arpNum, SequenceType::RELATIVE_SEQUENCE);
						break;
					default:
						if (bankVersion < Version::toBCD(1, 0, 2)) {
							// Recover deep clone bug
							// https://github.com/rerrahkr/BambooTracker/issues/170
							instManLocked->setArpeggioSSGType(arpNum, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						}
						else {
							throw FileCorruptionError(FileIO::FileType::Bank);
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
				if (ptNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				ssg->setPitchNumber(ptNum);
				size_t ptCsr = getPropertyPositionForBTB(propCtr, 0x34, orgPtNum);

				if (ptCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						if (l == 0)
							instManLocked->setPitchSSGSequenceCommand(ptNum, 0, data, 0);
						else
							instManLocked->addPitchSSGSequenceCommand(ptNum, data, 0);
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
						instManLocked->setPitchSSGLoops(ptNum, begins, ends, times);
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// No release
						instManLocked->setPitchSSGRelease(ptNum, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchSSGRelease(ptNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setPitchSSGRelease(ptNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setPitchSSGType(ptNum, SequenceType::ABSOLUTE_SEQUENCE);
						break;
					case 0x02:	// Relative
						instManLocked->setPitchSSGType(ptNum, SequenceType::RELATIVE_SEQUENCE);
						break;
					default:
						if (bankVersion < Version::toBCD(1, 0, 2)) {
							// Recover deep clone bug
							// https://github.com/rerrahkr/BambooTracker/issues/170
							instManLocked->setPitchSSGType(ptNum, SequenceType::ABSOLUTE_SEQUENCE);
							break;
						}
						else {
							throw FileCorruptionError(FileIO::FileType::Bank);
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
			if (sampNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
			adpcm->setSampleNumber(sampNum);
			size_t sampCsr = getPropertyPositionForBTB(propCtr, 0x40, orgSampNum);
			if (sampCsr != std::numeric_limits<size_t>::max()) {
				instManLocked->setSampleADPCMRootKeyNumber(sampNum, propCtr.readUint8(sampCsr++));
				instManLocked->setSampleADPCMRootDeltaN(sampNum, propCtr.readUint16(sampCsr));
				sampCsr += 2;
				instManLocked->setSampleADPCMRepeatEnabled(sampNum, (propCtr.readUint8(sampCsr++) & 0x01) != 0);
				uint32_t len = propCtr.readUint32(sampCsr);
				sampCsr += 4;
				std::vector<uint8_t> samples = propCtr.getSubcontainer(sampCsr, len).toVector();
				sampCsr += len;
				instManLocked->storeSampleADPCMRawSample(sampNum, std::move(samples));
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
				if (envNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				adpcm->setEnvelopeNumber(envNum);
				size_t envCsr = getPropertyPositionForBTB(propCtr, 0x41, orgEnvNum);

				if (envCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(envCsr);
					envCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(envCsr);
						envCsr += 2;
						int32_t subdata;
						subdata = propCtr.readInt32(envCsr);
						envCsr += 4;
						if (l == 0)
							instManLocked->setEnvelopeADPCMSequenceCommand(envNum, 0, data, subdata);
						else
							instManLocked->addEnvelopeADPCMSequenceCommand(envNum, data, subdata);
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
						instManLocked->setEnvelopeADPCMLoops(envNum, begins, ends, times);
					}

					switch (propCtr.readUint8(envCsr++)) {
					case 0x00:	// No release
						instManLocked->setEnvelopeADPCMRelease(envNum, ReleaseType::NoRelease, -1);
						break;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(envNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setEnvelopeADPCMRelease(envNum, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x02:	// Absolute
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(envNum, ReleaseType::AbsoluteRelease, pos);
						else instManLocked->setEnvelopeADPCMRelease(envNum, ReleaseType::NoRelease, -1);
						break;
					}
					case 0x03:	// Relative
					{
						uint16_t pos = propCtr.readUint16(envCsr);
						envCsr += 2;
						if (pos < seqLen) instManLocked->setEnvelopeADPCMRelease(envNum, ReleaseType::RelativeRelease, pos);
						else instManLocked->setEnvelopeADPCMRelease(envNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
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
				if (arpNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				adpcm->setArpeggioNumber(arpNum);
				size_t arpCsr = getPropertyPositionForBTB(propCtr, 0x42, orgArpNum);

				if (arpCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(arpCsr);
					arpCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						if (l == 0)
							instManLocked->setArpeggioADPCMSequenceCommand(arpNum, 0, data, 0);
						else
							instManLocked->addArpeggioADPCMSequenceCommand(arpNum, data, 0);
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
						instManLocked->setArpeggioADPCMLoops(arpNum, begins, ends, times);
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// No release
						instManLocked->setArpeggioADPCMRelease(arpNum, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(arpCsr);
						arpCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setArpeggioADPCMRelease(arpNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setArpeggioADPCMRelease(arpNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
					}

					switch (propCtr.readUint8(arpCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setArpeggioADPCMType(arpNum, SequenceType::ABSOLUTE_SEQUENCE);
						break;
					case 0x01:	// Fixed
						instManLocked->setArpeggioADPCMType(arpNum, SequenceType::FIXED_SEQUENCE);
						break;
					case 0x02:	// Relative
						instManLocked->setArpeggioADPCMType(arpNum, SequenceType::RELATIVE_SEQUENCE);
						break;
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
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
				if (ptNum == -1) throw FileCorruptionError(FileIO::FileType::Bank);
				adpcm->setPitchNumber(ptNum);
				size_t ptCsr = getPropertyPositionForBTB(propCtr, 0x43, orgPtNum);

				if (ptCsr != std::numeric_limits<size_t>::max()) {
					uint16_t seqLen = propCtr.readUint16(ptCsr);
					ptCsr += 2;
					for (uint16_t l = 0; l < seqLen; ++l) {
						uint16_t data = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						if (l == 0)
							instManLocked->setPitchADPCMSequenceCommand(ptNum, 0, data, 0);
						else
							instManLocked->addPitchADPCMSequenceCommand(ptNum, data, 0);
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
						instManLocked->setPitchADPCMLoops(ptNum, begins, ends, times);
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// No release
						instManLocked->setPitchADPCMRelease(ptNum, ReleaseType::NoRelease, -1);
						break;
					case 0x01:	// Fixed
					{
						uint16_t pos = propCtr.readUint16(ptCsr);
						ptCsr += 2;
						// Release point check (prevents a bug)
						// https://github.com/rerrahkr/BambooTracker/issues/11
						if (pos < seqLen) instManLocked->setPitchADPCMRelease(ptNum, ReleaseType::FixedRelease, pos);
						else instManLocked->setPitchADPCMRelease(ptNum, ReleaseType::NoRelease, -1);
						break;
					}
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
					}

					switch (propCtr.readUint8(ptCsr++)) {
					case 0x00:	// Absolute
						instManLocked->setPitchADPCMType(ptNum, SequenceType::ABSOLUTE_SEQUENCE);
						break;
					case 0x02:	// Relative
						instManLocked->setPitchADPCMType(ptNum, SequenceType::RELATIVE_SEQUENCE);
						break;
					default:
						throw FileCorruptionError(FileIO::FileType::Bank);
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
					if (newSamp == -1) throw FileCorruptionError(FileIO::FileType::Bank);
					kit->setSampleNumber(key, newSamp);
					sampMap[orgSamp] = newSamp;
					size_t sampCsr = getPropertyPositionForBTB(propCtr, 0x40, orgSamp);
					if (sampCsr != std::numeric_limits<size_t>::max()) {
						instManLocked->setSampleADPCMRootKeyNumber(newSamp, propCtr.readUint8(sampCsr++));
						instManLocked->setSampleADPCMRootDeltaN(newSamp, propCtr.readUint16(sampCsr));
						sampCsr += 2;
						instManLocked->setSampleADPCMRepeatEnabled(newSamp, (propCtr.readUint8(sampCsr++) & 0x01) != 0);
						uint32_t len = propCtr.readUint32(sampCsr);
						sampCsr += 4;
						std::vector<uint8_t> samples = propCtr.getSubcontainer(sampCsr, len).toVector();
						sampCsr += len;
						instManLocked->storeSampleADPCMRawSample(newSamp, std::move(samples));
						++newSamp;	// Increment for search
					}
				}
			}

			/* Pitch */
			kit->setPitch(key, instCtr.readInt8(instCsr++));
		}

		return kit;
	}
	default:
		throw FileCorruptionError(FileIO::FileType::Bank);
	}
}

AbstractInstrument* InstrumentIO::loadFfInstrument(const BinaryContainer& instCtr,
												   const std::string& name,
												   std::weak_ptr<InstrumentsManager> instMan,
												   int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Bank);

	InstrumentFM* fm = new InstrumentFM(instNum, name, instManLocked.get());
	fm->setEnvelopeNumber(envIdx);

	size_t csr = 0;
	uint8_t tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, tmp >> 3);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, tmp & 0x07);

	return fm;
}

AbstractInstrument* InstrumentIO::loadPPCInstrument(const std::vector<uint8_t> sample,
													std::weak_ptr<InstrumentsManager> instMan,
													int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int sampIdx = instManLocked->findFirstAssignableSampleADPCM();
	if (sampIdx < 0) throw FileCorruptionError(FileIO::FileType::Bank);

	InstrumentADPCM* adpcm = new InstrumentADPCM(instNum, "", instManLocked.get());
	adpcm->setSampleNumber(sampIdx);

	instManLocked->storeSampleADPCMRawSample(sampIdx, sample);
	instManLocked->setSampleADPCMRootKeyNumber(sampIdx, 67);	// o5g
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, calcADPCMDeltaN(16000));

	return adpcm;
}

AbstractInstrument* InstrumentIO::loadPVIInstrument(const std::vector<uint8_t> sample,
													std::weak_ptr<InstrumentsManager> instMan,
													int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int sampIdx = instManLocked->findFirstAssignableSampleADPCM();
	if (sampIdx < 0) throw FileCorruptionError(FileIO::FileType::Bank);

	InstrumentADPCM* adpcm = new InstrumentADPCM(instNum, "", instManLocked.get());
	adpcm->setSampleNumber(sampIdx);

	instManLocked->storeSampleADPCMRawSample(sampIdx, sample);
	instManLocked->setSampleADPCMRootKeyNumber(sampIdx, 60);	// o5c
	instManLocked->setSampleADPCMRootDeltaN(sampIdx, calcADPCMDeltaN(16000));

	return adpcm;
}
