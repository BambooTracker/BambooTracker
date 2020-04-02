#include "bank_io.hpp"
#include <algorithm>
#include <iterator>
#include "file_io.hpp"
#include "file_io_error.hpp"
#include "version.hpp"

BankIO::BankIO() {}

void BankIO::saveBank(BinaryContainer& ctr, std::vector<int> instNums,
					  std::weak_ptr<InstrumentsManager> instMan)
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
				ctr.appendUint8(static_cast<uint8_t>(instADPCM->getWaveformNumber()));
				uint8_t tmp = static_cast<uint8_t>(instADPCM->getEnvelopeNumber());
				ctr.appendUint8(instADPCM->getEnvelopeEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instADPCM->getArpeggioNumber());
				ctr.appendUint8(instADPCM->getArpeggioEnabled() ? tmp : (0x80 | tmp));
				tmp = static_cast<uint8_t>(instADPCM->getPitchNumber());
				ctr.appendUint8(instADPCM->getPitchEnabled() ? tmp : (0x80 | tmp));
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
		std::vector<int> users = instMan.lock()->getEnvelopeFMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) envFMIdcs.push_back(idx);
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
	std::vector<int> lfoFMIdcs;
	for (auto& idx : instMan.lock()->getLFOFMEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getLFOFMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) lfoFMIdcs.push_back(idx);
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
		for (auto& idx : instMan.lock()->getOperatorSequenceFMEntriedIndices(FileIO::ENV_FM_PARAMS[i])) {
			std::vector<int> users = instMan.lock()->getOperatorSequenceFMUsers(FileIO::ENV_FM_PARAMS[i], idx);
			std::vector<int> intersection;
			std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
			if (!intersection.empty()) idcs.push_back(idx);
		}
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
	}

	// FM arpeggio
	std::vector<int> arpFMIdcs;
	for (auto& idx : instMan.lock()->getArpeggioFMEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getArpeggioFMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) arpFMIdcs.push_back(idx);
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
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getArpeggioFMType(idx)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::FIXED_SEQUENCE:		ctr.appendUint8(0x01);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// FM pitch
	std::vector<int> ptFMIdcs;
	for (auto& idx : instMan.lock()->getPitchFMEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getPitchFMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) ptFMIdcs.push_back(idx);
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
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getPitchFMType(idx)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG waveform
	std::vector<int> wfSSGIdcs;
	for (auto& idx : instMan.lock()->getWaveformSSGEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getWaveformSSGUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) wfSSGIdcs.push_back(idx);
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
			}
			auto loop = instMan.lock()->getWaveformSSGLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getWaveformSSGRelease(idx);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// SSG tone/noise
	std::vector<int> tnSSGIdcs;
	for (auto& idx : instMan.lock()->getToneNoiseSSGEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getToneNoiseSSGUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) tnSSGIdcs.push_back(idx);
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
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// SSG envelope
	std::vector<int> envSSGIdcs;
	for (auto& idx : instMan.lock()->getEnvelopeSSGEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getEnvelopeSSGUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) envSSGIdcs.push_back(idx);
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
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
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// SSG arpeggio
	std::vector<int> arpSSGIdcs;
	for (auto& idx : instMan.lock()->getArpeggioSSGEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getArpeggioSSGUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) arpSSGIdcs.push_back(idx);
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
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getArpeggioSSGType(idx)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::FIXED_SEQUENCE:		ctr.appendUint8(0x01);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// SSG pitch
	std::vector<int> ptSSGIdcs;
	for (auto& idx : instMan.lock()->getPitchSSGEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getPitchSSGUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) ptSSGIdcs.push_back(idx);
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
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getPitchSSGType(idx)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// ADPCM waveform
	std::vector<int> wfADPCMIdcs;
	for (auto& idx : instMan.lock()->getWaveformADPCMEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getWaveformADPCMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) wfADPCMIdcs.push_back(idx);
	}
	if (!wfADPCMIdcs.empty()) {
		ctr.appendUint8(0x40);
		ctr.appendUint8(static_cast<uint8_t>(wfADPCMIdcs.size()));
		for (auto& idx : wfADPCMIdcs) {
			ctr.appendUint8(static_cast<uint8_t>(idx));
			size_t ofs = ctr.size();
			ctr.appendUint32(0);	// Dummy offset
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->getWaveformADPCMRootKeyNumber(idx)));
			ctr.appendUint16(static_cast<uint16_t>(instMan.lock()->getWaveformADPCMRootDeltaN(idx)));
			ctr.appendUint8(static_cast<uint8_t>(instMan.lock()->isWaveformADPCMRepeatable(idx)));
			std::vector<uint8_t> samples = instMan.lock()->getWaveformADPCMSamples(idx);
			ctr.appendUint32(samples.size());
			ctr.appendVector(std::move(samples));
			ctr.writeUint32(ofs, ctr.size() - ofs);
		}
	}

	// ADPCM envelope
	std::vector<int> envADPCMIdcs;
	for (auto& idx : instMan.lock()->getEnvelopeADPCMEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getEnvelopeADPCMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) envADPCMIdcs.push_back(idx);
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
				ctr.appendInt32(static_cast<int32_t>(com.data));
			}
			auto loop = instMan.lock()->getEnvelopeADPCMLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getEnvelopeADPCMRelease(idx);

			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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

	// ADPCM arpeggio
	std::vector<int> arpADPCMIdcs;
	for (auto& idx : instMan.lock()->getArpeggioADPCMEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getArpeggioADPCMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) arpADPCMIdcs.push_back(idx);
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getArpeggioADPCMLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getArpeggioADPCMRelease(idx);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getArpeggioADPCMType(idx)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::FIXED_SEQUENCE:		ctr.appendUint8(0x01);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	// ADPCM pitch
	std::vector<int> ptADPCMIdcs;
	for (auto& idx : instMan.lock()->getPitchADPCMEntriedIndices()) {
		std::vector<int> users = instMan.lock()->getPitchADPCMUsers(idx);
		std::vector<int> intersection;
		std::set_intersection(users.begin(), users.end(), instNums.begin(), instNums.end(), std::back_inserter(intersection));
		if (!intersection.empty()) ptADPCMIdcs.push_back(idx);
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
			for (auto& com : seq) {
				ctr.appendUint16(static_cast<uint16_t>(com.type));
			}
			auto loop = instMan.lock()->getPitchADPCMLoops(idx);
			ctr.appendUint16(static_cast<uint16_t>(loop.size()));
			for (auto& l : loop) {
				ctr.appendUint16(static_cast<uint16_t>(l.begin));
				ctr.appendUint16(static_cast<uint16_t>(l.end));
				ctr.appendUint8(static_cast<uint8_t>(l.times));
			}
			auto release = instMan.lock()->getPitchADPCMRelease(idx);
			switch (release.type) {
			case ReleaseType::NoRelease:
				ctr.appendUint8(0x00);
				// If release.type is NO_RELEASE, then release.begin == -1 so omit to save it.
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
			switch (instMan.lock()->getPitchADPCMType(idx)) {
			case SequenceType::ABSOLUTE_SEQUENCE:	ctr.appendUint8(0x00);	break;
			case SequenceType::RELATIVE_SEQUENCE:	ctr.appendUint8(0x02);	break;
			default:												break;
			}
			ctr.writeUint16(ofs, static_cast<uint16_t>(ctr.size() - ofs));
		}
	}

	ctr.writeUint32(instPropOfs, ctr.size() - instPropOfs);

	ctr.writeUint32(eofOfs, ctr.size() - eofOfs);
}

AbstractBank* BankIO::loadBank(BinaryContainer& ctr, std::string path)
{
	std::string ext = FileIO::getExtension(path);
	if (ext.compare("wopn") == 0) return BankIO::loadWOPNFile(ctr);
	if (ext.compare("btb") == 0) return BankIO::loadBTBFile(ctr);
	if (ext.compare("ppc") == 0) return BankIO::loadPPCFile(ctr);
	if (ext.compare("pvi") == 0) return BankIO::loadPVIFile(ctr);
	throw FileInputError(FileIO::FileType::Bank);
}

AbstractBank* BankIO::loadBTBFile(BinaryContainer& ctr)
{
	size_t globCsr = 0;
	if (ctr.readString(globCsr, 16) != "BambooTrackerBnk")
		throw FileCorruptionError(FileIO::FileType::Bank);
	globCsr += 16;
	/*size_t eofOfs = */ctr.readUint32(globCsr);
	globCsr += 4;
	size_t fileVersion = ctr.readUint32(globCsr);
	if (fileVersion > Version::ofBankFileInBCD())
		throw FileVersionError(FileIO::FileType::Bank);
	globCsr += 4;


	/***** Instrument section *****/
	std::vector<int> ids;
	std::vector<std::string> names;
	std::vector<BinaryContainer> instCtrs;
	if (ctr.readString(globCsr, 8) != "INSTRMNT") throw FileCorruptionError(FileIO::FileType::Bank);
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
			iCsr += nameLen;
		}
		names.push_back(name);
		instCsr += iOfs;	// Jump to next
		instCtrs.push_back(ctr.getSubcontainer(pos, 1 + iOfs));
	}
	globCsr += instOfs;


	/***** Instrument property section *****/
	if (ctr.readString(globCsr, 8) != "INSTPROP") throw FileCorruptionError(FileIO::FileType::Inst);
	globCsr += 8;
	size_t instPropOfs = ctr.readUint32(globCsr);
	BinaryContainer propCtr = ctr.getSubcontainer(globCsr + 4, instPropOfs - 4);

	return new BtBank(std::move(ids), std::move(names), std::move(instCtrs), std::move(propCtr), fileVersion);
}

AbstractBank* BankIO::loadWOPNFile(BinaryContainer& ctr)
{
	struct WOPNDeleter {
		void operator()(WOPNFile *x) { WOPN_Free(x); }
	};

	std::unique_ptr<WOPNFile, WOPNDeleter> wopn;
	wopn.reset(WOPN_LoadBankFromMem(const_cast<char*>(ctr.getPointer()), ctr.size(), nullptr));
	if (!wopn)
		throw FileCorruptionError(FileIO::FileType::Bank);

	WopnBank *bank = new WopnBank(wopn.get());
	wopn.release();
	return bank;
}

AbstractBank* BankIO::loadPPCFile(BinaryContainer& ctr)
{
	size_t globCsr = 0;
	if (ctr.readString(globCsr, 30) != "ADPCM DATA for  PMD ver.4.4-  ")
		throw FileCorruptionError(FileIO::FileType::Bank);
	globCsr += 30;
	uint16_t nextAddr = ctr.readUint16(globCsr);
	if ((nextAddr - 0x26u) * 0x20u + 0x420u != ctr.size())	// File size check
		throw FileCorruptionError(FileIO::FileType::Bank);
	globCsr += 2;

	size_t sampOffs = globCsr + 256 * 4;
	if (ctr.size() < sampOffs) throw FileCorruptionError(FileIO::FileType::Bank);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	BankIO::extractADPCMSamples(ctr, globCsr, sampOffs, 256, ids, samples);

	return new PpcBank(std::move(ids), std::move(samples));
}

AbstractBank* BankIO::loadPVIFile(BinaryContainer& ctr)
{
	size_t globCsr = 0;
	if (ctr.readString(globCsr, 4) != "PVI2")
		throw FileCorruptionError(FileIO::FileType::Bank);
	globCsr += 0x10;

	size_t sampOffs = globCsr + 128 * 4;
	if (ctr.size() < sampOffs) throw FileCorruptionError(FileIO::FileType::Bank);

	std::vector<int> ids;
	std::vector<std::vector<uint8_t>> samples;
	BankIO::extractADPCMSamples(ctr, globCsr, sampOffs, 128, ids, samples);

	return new PviBank(std::move(ids), std::move(samples));
}

void BankIO::extractADPCMSamples(const BinaryContainer& ctr, size_t addrPos, size_t sampOffs, int maxCnt,
								 std::vector<int>& ids, std::vector<std::vector<uint8_t>>& samples)
{
	size_t ofs = 0;
	for (int i = 0; i < maxCnt; ++i) {
		uint16_t start = ctr.readUint16(addrPos);
		addrPos += 2;
		uint16_t stop = ctr.readUint16(addrPos);
		addrPos += 2;

		if (stop && start <= stop) {
			if (ids.empty()) ofs = start;
			ids.push_back(i);
			size_t st = sampOffs + static_cast<size_t>((start - ofs) << 5);
			size_t sampSize = std::min(static_cast<size_t>((stop + 1 - start) << 5), ctr.size() - st);
			samples.push_back(ctr.getSubcontainer(st, sampSize).toVector());
		}
	}
}
