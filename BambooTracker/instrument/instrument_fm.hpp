#pragma once

#include <string>
#include <memory>
#include <map>
#include "abstruct_instrument.hpp"

enum class FMParameter;

class InstrumentFM : public AbstructInstrument
{
public:
	InstrumentFM(int number, std::string name);
	InstrumentFM(const InstrumentFM& other);
	~InstrumentFM() = default;
	std::unique_ptr<AbstructInstrument> clone() override;

	bool getOperatorEnable(int num) const;
	void setOperatorEnable(int num, bool enable);

	int getParameterValue(FMParameter param) const;
	void setParameterValue(FMParameter param, int value);

private:
	int al_;
	int fb_;
	struct FMOperator
	{
		bool enable_;
		int ar_;
		int dr_;
		int sr_;
		int rr_;
		int sl_;
		int tl_;
		int ks_;
		int ml_;
		int dt_;
		int am_;
		int ssgeg_;	// -1: No use
	};
	FMOperator op_[4];

	std::map<FMParameter, int&> paramMap_;
	void initParamMap();
};

enum class FMParameter
{
	AL, FB,
	AR1, DR1, SR1, RR1, SL1, TL1, KS1, ML1, DT1, AM1,
	AR2, DR2, SR2, RR2, SL2, TL2, KS2, ML2, DT2, AM2,
	AR3, DR3, SR3, RR3, SL3, TL3, KS3, ML3, DT3, AM3,
	AR4, DR4, SR4, RR4, SL4, TL4, KS4, ML4, DT4, AM4,
	SSGEG1, SSGEG2, SSGEG3, SSGEG4
};
