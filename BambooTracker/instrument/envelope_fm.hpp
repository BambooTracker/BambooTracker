#pragma once

#include <map>
#include "instrument.hpp"

class EnvelopeFM
{
public:
	EnvelopeFM();

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
