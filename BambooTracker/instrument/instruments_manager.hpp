#pragma once

#include <map>
#include <string>
#include <memory>
#include "instrument.hpp"
#include "misc.hpp"

class AbstructInstrument;
enum class FMParameter;

class InstrumentsManager
{
public:
	std::unique_ptr<AbstructInstrument> addInstrument(int num, SoundSource source, std::string name);
	std::unique_ptr<AbstructInstrument> addInstrument(std::unique_ptr<AbstructInstrument> inst);
	std::unique_ptr<AbstructInstrument> removeInstrument(int n);
	std::unique_ptr<AbstructInstrument> getInstrumentCopy(int n);

	void setInstrumentName(int num, std::string name);
	void setFMParameterValue(int n, FMParameter param, int value);
	void setFMOperatorEnable(int instNum, int opNum, bool enable);

private:
	std::map<int, std::unique_ptr<AbstructInstrument>> instMap_;
};
