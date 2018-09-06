#include "lfo_fm.hpp"

LFOFM::LFOFM(int n)
	: AbstructInstrumentProperty (n),
	  freq_(0),
	  pms_(0),
	  ams_(0)
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
	for (int i = 0; i < 4; ++i)
		amOp_[i] = other.amOp_[i];
}

std::unique_ptr<LFOFM> LFOFM::clone()
{
	return std::unique_ptr<LFOFM>(std::make_unique<LFOFM>(*this));
}

void LFOFM::setParameterValue(FMLFOParamter param, int value)
{
	switch (param) {
	case FMLFOParamter::FREQ:	freq_ = value;		break;
	case FMLFOParamter::PMS:	pms_ = value;		break;
	case FMLFOParamter::AMS:	ams_ = value;		break;
	case FMLFOParamter::AM1:	amOp_[0] = value;	break;
	case FMLFOParamter::AM2:	amOp_[1] = value;	break;
	case FMLFOParamter::AM3:	amOp_[2] = value;	break;
	case FMLFOParamter::AM4:	amOp_[3] = value;	break;
	}
}

int LFOFM::getParameterValue(FMLFOParamter param) const
{
	switch (param) {
	case FMLFOParamter::FREQ:	return freq_;
	case FMLFOParamter::PMS:	return pms_;
	case FMLFOParamter::AMS:	return ams_;
	case FMLFOParamter::AM1:	return amOp_[0];
	case FMLFOParamter::AM2:	return amOp_[1];
	case FMLFOParamter::AM3:	return amOp_[2];
	case FMLFOParamter::AM4:	return amOp_[3];
	}
}
