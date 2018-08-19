#pragma once

#include <string>
#include <memory>
#include <array>
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
	void cloneInstrument(int cloneInstNum, int resInstNum);
	void deepCloneInstrument(int cloneInstNum, int resInstNum);
	std::shared_ptr<AbstructInstrument> getInstrumentSharedPtr(int instNum);

	void setInstrumentName(int instNum, std::string name);
	std::string getInstrumentName(int instNum) const;
	void setInstrumentFMEnvelope(int instNum, int envNum);
    int getInstrumentFMEnvelope(int instNum) const;

	void setEnvelopeFMParameter(int envNum, FMParameter param, int value);
	int getEnvelopeFMParameter(int envNum, FMParameter param) const;
	void setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable);
	bool getEnvelopeFMOperatorEnable(int envNum, int opNum) const;

	int findFirstFreeInstrument() const;

private:
	std::array<std::shared_ptr<AbstructInstrument>, 128> insts_;
	std::array<std::shared_ptr<EnvelopeFM>, 128> envFM_;

	int cloneFMEnvelope(int srcNum);
};
