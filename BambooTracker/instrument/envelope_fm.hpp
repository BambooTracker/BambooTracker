#pragma once

#include <map>
#include <memory>
#include <vector>
#include "instrument.hpp"

enum class FMParameter;

class EnvelopeFM
{
public:
	explicit EnvelopeFM(int num);
	~EnvelopeFM() = default;
	EnvelopeFM(const EnvelopeFM& other);

	std::unique_ptr<EnvelopeFM> clone();

	void setNumber(int num);
	int getNumber() const;

	bool getOperatorEnable(int num) const;
	void setOperatorEnable(int num, bool enable);

	int getParameterValue(FMParameter param) const;
	void setParameterValue(FMParameter param, int value);

	void registerInstrumentUsingThis(int instNum);
	void deregisterInstrumentUsingThis(int instNum);
	bool isUsedInInstrument() const;

private:
	int num_;

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
		int ssgeg_;	// -1: No use
	};
	FMOperator op_[4];
	std::map<FMParameter, int&> paramMap_;

	std::vector<int> instsUseThis_;

	void initParamMap();
};
