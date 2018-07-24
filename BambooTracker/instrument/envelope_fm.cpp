#include "envelope_fm.hpp"

EnvelopeFM::EnvelopeFM() :
	al_(4), fb_(0)
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
