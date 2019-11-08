#pragma once

#include <memory>
#include "abstract_instrument_property.hpp"

enum class FMLFOParameter;


class LFOFM : public AbstractInstrumentProperty
{
public:
	explicit LFOFM(int n);
	LFOFM(const LFOFM& other);

	std::unique_ptr<LFOFM> clone();

	void setParameterValue(FMLFOParameter param, int value);
	int getParameterValue(FMLFOParameter param) const;

	bool isEdited() const;

private:
	int freq_;
	int pms_;
	int ams_;
	int amOp_[4];
	int cnt_;

	static constexpr int DEF_FREQ = 0;
	static constexpr int DEF_PMS = 0;
	static constexpr int DEF_AMS = 0;
	static constexpr int DEF_AM_OP[4] = { 0, 0, 0, 0 };
	static constexpr int DEF_CNT = 0;
};

enum class FMLFOParameter
{
	FREQ, AMS, PMS, Count,
	AM1, AM2, AM3, AM4
};
