#pragma once

#include <memory>
#include "abstruct_instrument_property.hpp"

enum class FMLFOParamter;


class LFOFM : public AbstructInstrumentProperty
{
public:
	explicit LFOFM(int n);
	LFOFM(const LFOFM& other);

	std::unique_ptr<LFOFM> clone();

	void setParameterValue(FMLFOParamter param, int value);
	int getParameterValue(FMLFOParamter param) const;

private:
	int freq_;
	int pms_;
	int ams_;
	int amOp_[4];
};


enum class FMLFOParamter
{
	FREQ, AMS, PMS,
	AM1, AM2, AM3, AM4
};
