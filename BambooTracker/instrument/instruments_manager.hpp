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

	void addInstrument(int instNum, SoundSource source, std::string name);
	void addInstrument(std::unique_ptr<AbstructInstrument> inst);
	std::unique_ptr<AbstructInstrument> removeInstrument(int instNum);
	std::shared_ptr<AbstructInstrument> getInstrumentSharedPtr(int instNum);

	void setInstrumentName(int instNum, std::string name);
	std::string getInstrumentName(int instNum) const;
	void setInstrumentFMEnvelope(int instNum, int envNum);
    int getInstrumentFMEnvelope(int instNum) const;

	void setEnvelopeFMParameter(int envNum, FMParameter param, int value);
	int getEnvelopeFMParameter(int envNum, FMParameter param) const;
	void setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable);
	bool getEnvelopeFMOperatorEnable(int envNum, int opNum) const;

private:
	std::shared_ptr<AbstructInstrument> insts_[128];
	std::shared_ptr<EnvelopeFM> envFM_[128];
};
