#pragma once

#include <map>
#include <memory>
#include "abstract_instrument_property.hpp"

enum class FMEnvelopeParameter;

class EnvelopeFM : public AbstractInstrumentProperty
{
public:
	explicit EnvelopeFM(int num);
	EnvelopeFM(const EnvelopeFM& other);

	std::unique_ptr<EnvelopeFM> clone();

	bool getOperatorEnabled(int num) const;
	void setOperatorEnabled(int num, bool enabled);

	int getParameterValue(FMEnvelopeParameter param) const;
	void setParameterValue(FMEnvelopeParameter param, int value);

private:
	int al_;
	int fb_;
	struct FMOperator
	{
		bool enabled_;
		int ar_;
		int dr_;
		int sr_;
		int rr_;
		int sl_;
		int tl_;
		int ks_;
		int ml_;
		int dt_;
		int ssgeg_;	// -1: No use
	};
	FMOperator op_[4];
	std::map<FMEnvelopeParameter, int&> paramMap_;

	void initParamMap();
};


enum class FMEnvelopeParameter
{
	AL, FB,
	AR1, DR1, SR1, RR1, SL1, TL1, KS1, ML1, DT1,
	AR2, DR2, SR2, RR2, SL2, TL2, KS2, ML2, DT2,
	AR3, DR3, SR3, RR3, SL3, TL3, KS3, ML3, DT3,
	AR4, DR4, SR4, RR4, SL4, TL4, KS4, ML4, DT4,
	SSGEG1, SSGEG2, SSGEG3, SSGEG4
};
