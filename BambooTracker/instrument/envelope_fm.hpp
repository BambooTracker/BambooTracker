#pragma once

#include <map>
#include <memory>
#include "instrument.hpp"

enum class FMParameter;

class EnvelopeFM
{
public:
	EnvelopeFM();
	~EnvelopeFM() = default;
	EnvelopeFM(const EnvelopeFM& other);

	std::unique_ptr<EnvelopeFM> clone();

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
		int ssgeg_;	// -1: No use
	};
	FMOperator op_[4];

	std::map<FMParameter, int&> paramMap_;
	void initParamMap();
};
