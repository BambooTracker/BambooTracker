#include "lfo_fm.hpp"

LFOFM::LFOFM(int n)
	: AbstructInstrumentProperty (n),
	  freq_(0),
	  pms_(0),
	  ams_(0),
	  cnt_(0)
{
	for (int i = 0; i < 4; ++i)
		amOp_[i] = 0;
}

LFOFM::LFOFM(const LFOFM& other)
	: AbstructInstrumentProperty (other)
{
	freq_ = other.freq_;
	ams_ = other.ams_;
	pms_ = other.pms_;
	cnt_ = other.cnt_;
	for (int i = 0; i < 4; ++i)
		amOp_[i] = other.amOp_[i];
}

std::unique_ptr<LFOFM> LFOFM::clone()
{
	return std::unique_ptr<LFOFM>(std::make_unique<LFOFM>(*this));
}

void LFOFM::setParameterValue(FMLFOParameter param, int value)
{
	switch (param) {
	case FMLFOParameter::FREQ:	freq_ = value;		break;
	case FMLFOParameter::PMS:	pms_ = value;		break;
	case FMLFOParameter::AMS:	ams_ = value;		break;
	case FMLFOParameter::COUNT:	cnt_ = value;		break;
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
	case FMLFOParameter::COUNT:	return cnt_;
	case FMLFOParameter::AM1:	return amOp_[0];
	case FMLFOParameter::AM2:	return amOp_[1];
	case FMLFOParameter::AM3:	return amOp_[2];
	case FMLFOParameter::AM4:	return amOp_[3];
	}
}