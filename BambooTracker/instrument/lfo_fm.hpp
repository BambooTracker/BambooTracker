#pragma once

#include <memory>
#include "abstruct_instrument_property.hpp"

enum class FMLFOParameter;


class LFOFM : public AbstructInstrumentProperty
{
public:
	explicit LFOFM(int n);
	LFOFM(const LFOFM& other);

	std::unique_ptr<LFOFM> clone();

	void setParameterValue(FMLFOParameter param, int value);
	int getParameterValue(FMLFOParameter param) const;

private:
	int freq_;
	int pms_;
	int ams_;
	int amOp_[4];
	int cnt_;
};


enum class FMLFOParameter
{
	FREQ, AMS, PMS, COUNT,
	AM1, AM2, AM3, AM4
};
