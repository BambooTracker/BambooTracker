#pragma once

#include <string>
#include <memory>
#include "instrument.hpp"
#include "envelope_fm.hpp"
#include "misc.hpp"

class AbstructInstrument;
enum class FMParameter;
class EnvelopeFM;

class InstrumentsManager
{
public:
	InstrumentsManager();

	void addInstrument(int num, SoundSource source, std::string name);
	void addInstrument(std::unique_ptr<AbstructInstrument> inst);
	std::unique_ptr<AbstructInstrument> removeInstrument(int n);
	std::unique_ptr<AbstructInstrument> getInstrumentCopy(int n);

	void setInstrumentName(int num, std::string name);

	void setFMEnvelopeParameter(int envNum, FMParameter param, int value);
	int getFMEnvelopeParameter(int envNum, FMParameter param) const;

	void setFMOperatorEnable(int envNum, int opNum, bool enable);
	bool getFMOperatorEnable(int envNum, int opNum) const;

private:
	std::unique_ptr<AbstructInstrument> insts_[128];
	std::shared_ptr<EnvelopeFM> envFM_[128];
};
