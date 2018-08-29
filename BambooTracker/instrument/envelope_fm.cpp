#include "envelope_fm.hpp"
#include <algorithm>

EnvelopeFM::EnvelopeFM(int num) :
	num_(num), al_(4), fb_(0)
{
	op_[0] = FMOperator{ true, 31, 0, 0, 7, 0, 32, 0, 0, 0, -1 };
	op_[1] = FMOperator{ true, 31, 0, 0, 7, 0, 0, 0, 0, 0, -1 };
	op_[2] = FMOperator{ true, 31, 0, 0, 7, 0, 32, 0, 0, 0, -1 };
	op_[3] = FMOperator{ true, 31, 0, 0, 7, 0, 0, 0, 0, 0, -1 };

	initParamMap();
}

void EnvelopeFM::initParamMap()
{
	paramMap_ = {
		{ FMParameter::AL, al_ },
		{ FMParameter::FB, fb_ },
		{ FMParameter::AR1, op_[0].ar_ },
		{ FMParameter::DR1, op_[0].dr_ },
		{ FMParameter::SR1, op_[0].sr_ },
		{ FMParameter::RR1, op_[0].rr_ },
		{ FMParameter::SL1, op_[0].sl_ },
		{ FMParameter::TL1, op_[0].tl_ },
		{ FMParameter::KS1, op_[0].ks_ },
		{ FMParameter::ML1, op_[0].ml_ },
		{ FMParameter::DT1, op_[0].dt_ },
		{ FMParameter::SSGEG1, op_[0].ssgeg_ },
		{ FMParameter::AR2, op_[1].ar_ },
		{ FMParameter::DR2, op_[1].dr_ },
		{ FMParameter::SR2, op_[1].sr_ },
		{ FMParameter::RR2, op_[1].rr_ },
		{ FMParameter::SL2, op_[1].sl_ },
		{ FMParameter::TL2, op_[1].tl_ },
		{ FMParameter::KS2, op_[1].ks_ },
		{ FMParameter::ML2, op_[1].ml_ },
		{ FMParameter::DT2, op_[1].dt_ },
		{ FMParameter::SSGEG2, op_[1].ssgeg_ },
		{ FMParameter::AR3, op_[2].ar_ },
		{ FMParameter::DR3, op_[2].dr_ },
		{ FMParameter::SR3, op_[2].sr_ },
		{ FMParameter::RR3, op_[2].rr_ },
		{ FMParameter::SL3, op_[2].sl_ },
		{ FMParameter::TL3, op_[2].tl_ },
		{ FMParameter::KS3, op_[2].ks_ },
		{ FMParameter::ML3, op_[2].ml_ },
		{ FMParameter::DT3, op_[2].dt_ },
		{ FMParameter::SSGEG3, op_[2].ssgeg_ },
		{ FMParameter::AR4, op_[3].ar_ },
		{ FMParameter::DR4, op_[3].dr_ },
		{ FMParameter::SR4, op_[3].sr_ },
		{ FMParameter::RR4, op_[3].rr_ },
		{ FMParameter::SL4, op_[3].sl_ },
		{ FMParameter::TL4, op_[3].tl_ },
		{ FMParameter::KS4, op_[3].ks_ },
		{ FMParameter::ML4, op_[3].ml_ },
		{ FMParameter::DT4, op_[3].dt_ },
		{ FMParameter::SSGEG4, op_[3].ssgeg_ }
	};
}

EnvelopeFM::EnvelopeFM(const EnvelopeFM &other)
{
	num_ = other.num_;
	al_ = other.al_;
	fb_ = other.fb_;

	for (int i = 0; i < 4; ++i) {
		op_[i].enable_ = other.op_[i].enable_;
		op_[i].ar_ = other.op_[i].ar_;
		op_[i].dr_ = other.op_[i].dr_;
		op_[i].sr_ = other.op_[i].sr_;
		op_[i].rr_ = other.op_[i].rr_;
		op_[i].sl_ = other.op_[i].sl_;
		op_[i].tl_ = other.op_[i].tl_;
		op_[i].ks_ = other.op_[i].ks_;
		op_[i].ml_ = other.op_[i].ml_;
		op_[i].dt_ = other.op_[i].dt_;
		op_[i].ssgeg_ = other.op_[i].ssgeg_;
	}

	initParamMap();
}

std::unique_ptr<EnvelopeFM> EnvelopeFM::clone()
{
	return std::unique_ptr<EnvelopeFM>(std::make_unique<EnvelopeFM>(*this));
}

void EnvelopeFM::setNumber(int num)
{
	num_ = num;
}

int EnvelopeFM::getNumber() const
{
	return num_;
}

bool EnvelopeFM::getOperatorEnable(int num) const
{
	return op_[num].enable_;
}

void EnvelopeFM::setOperatorEnable(int num, bool enable)
{
	op_[num].enable_ = enable;
}

int EnvelopeFM::getParameterValue(FMParameter param) const
{
	return paramMap_.at(param);
}

void EnvelopeFM::setParameterValue(FMParameter param, int value)
{
	paramMap_.at(param) = value;
}

void EnvelopeFM::registerUserInstrument(int instNum)
{
	instsUseThis_.push_back(instNum);
	std::sort(instsUseThis_.begin(), instsUseThis_.end());
}

void EnvelopeFM::deregisterUserInstrument(int instNum)
{
	instsUseThis_.erase(std::find(instsUseThis_.begin(), instsUseThis_.end(), instNum));
}

bool EnvelopeFM::isUserInstrument() const
{
	return !instsUseThis_.empty();
}

std::vector<int> EnvelopeFM::getUserInstruments() const
{
	return instsUseThis_;
}
