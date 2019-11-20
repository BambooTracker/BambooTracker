#include "lfo_fm.hpp"
#include <stdexcept>

constexpr int LFOFM::DEF_AM_OP[4];

LFOFM::LFOFM(int n)
	: AbstractInstrumentProperty (n),
	  freq_(DEF_FREQ),
	  pms_(DEF_PMS),
	  ams_(DEF_AMS),
	  cnt_(DEF_CNT)
{
	for (int i = 0; i < 4; ++i)
		amOp_[i] = DEF_AM_OP[i];
}

LFOFM::LFOFM(const LFOFM& other)
	: AbstractInstrumentProperty (other)
{
	freq_ = other.freq_;
	ams_ = other.ams_;
	pms_ = other.pms_;
	cnt_ = other.cnt_;
	for (int i = 0; i < 4; ++i)
		amOp_[i] = other.amOp_[i];
}

bool operator==(const LFOFM& a, const LFOFM& b)
{
	return (a.freq_ == b.freq_ && a.ams_ == b.ams_ && a.pms_ == b.pms_ && a.cnt_ == b.cnt_
			&& a.amOp_[0] == b.amOp_[0] && a.amOp_[1] == b.amOp_[1]
			&& a.amOp_[2] == b.amOp_[2] && a.amOp_[3] == b.amOp_[3]);
}

std::unique_ptr<LFOFM> LFOFM::clone()
{
	std::unique_ptr<LFOFM> clone = std::make_unique<LFOFM>(*this);
	clone->clearUserInstruments();
	return clone;
}

void LFOFM::setParameterValue(FMLFOParameter param, int value)
{
	switch (param) {
	case FMLFOParameter::FREQ:	freq_ = value;		break;
	case FMLFOParameter::PMS:	pms_ = value;		break;
	case FMLFOParameter::AMS:	ams_ = value;		break;
	case FMLFOParameter::Count:	cnt_ = value;		break;
	case FMLFOParameter::AM1:	amOp_[0] = value;	break;
	case FMLFOParameter::AM2:	amOp_[1] = value;	break;
	case FMLFOParameter::AM3:	amOp_[2] = value;	break;
	case FMLFOParameter::AM4:	amOp_[3] = value;	break;
	}
}

int LFOFM::getParameterValue(FMLFOParameter param) const
{
	switch (param) {
	case FMLFOParameter::FREQ:	return freq_;
	case FMLFOParameter::PMS:	return pms_;
	case FMLFOParameter::AMS:	return ams_;
	case FMLFOParameter::Count:	return cnt_;
	case FMLFOParameter::AM1:	return amOp_[0];
	case FMLFOParameter::AM2:	return amOp_[1];
	case FMLFOParameter::AM3:	return amOp_[2];
	case FMLFOParameter::AM4:	return amOp_[3];
	default:	throw std::invalid_argument("Unexpected FMLFOParameter.");
	}
}

bool LFOFM::isEdited() const
{
	if (freq_ != DEF_FREQ
			|| pms_ != DEF_PMS
			|| ams_ != DEF_AMS
			|| cnt_ != DEF_CNT)
		return true;
	for (int i = 0; i < 4; ++i) {
		if (amOp_[i] != DEF_AM_OP[i]) return true;
	}
	return false;
}
