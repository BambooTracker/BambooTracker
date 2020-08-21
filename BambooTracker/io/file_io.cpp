#include "file_io.hpp"
#include <stdexcept>
#include <algorithm>

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

const FMOperatorType FileIO::OP_FM_TYPES[4] = {
	FMOperatorType::Op1,
	FMOperatorType::Op2,
	FMOperatorType::Op3,
	FMOperatorType::Op4
};

std::string FileIO::getExtension(const std::string path)
{
	std::string ext = path.substr(path.find_last_of(".") + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	return ext;
}

FileIO::FileType FileIO::judgeFileTypeFromExtension(std::string ext)
{
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == "btm") return FileType::Mod;

	if (ext == "bti") return FileType::Inst;
	if (ext == "dmp") return FileType::Inst;
	if (ext == "tfi") return FileType::Inst;
	if (ext == "vgi") return FileType::Inst;
	if (ext == "opni") return FileType::Inst;
	if (ext == "y12") return FileType::Inst;
	if (ext == "ins") return FileType::Inst;

	if (ext == "btb") return FileType::Bank;
	if (ext == "wopn") return FileType::Bank;
	if (ext == "ff") return FileType::Bank;
	if (ext == "ppc") return FileType::Bank;
	if (ext == "pvi") return FileType::Bank;
	if (ext == "dat") return FileType::Bank;

	if (ext == "vgm") return FileType::VGM;
	if (ext == "s98") return FileType::S98;
	if (ext == "wav") return FileType::WAV;

	return FileType::Unknown;
}
