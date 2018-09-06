#pragma once

#include <string>
#include <memory>
#include <array>
#include <vector>
#include "instrument.hpp"
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "misc.hpp"

class AbstructInstrument;
enum class FMEnvelopeParameter;
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

	int findFirstFreeInstrument() const;

	void setInstrumentGateCount(int instNum, int count);

	//----- FM methods -----
	void setInstrumentFMEnvelope(int instNum, int envNum);
    int getInstrumentFMEnvelope(int instNum) const;
	void setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value);
	int getEnvelopeFMParameter(int envNum, FMEnvelopeParameter param) const;
	void setEnvelopeFMOperatorEnabled(int envNum, int opNum, bool enabled);
	bool getEnvelopeFMOperatorEnabled(int envNum, int opNum) const;
	std::vector<int> getEnvelopeFMUsers(int envNum) const;

	void setInstrumentFMLFO(int instNum, int lfoNum);
	int getInstrumentFMLFO(int instNum) const;
	void setLFOFMParameter(int lfoNum, FMLFOParamter param, int value);
	int getLFOFMparameter(int lfoNum, FMLFOParamter param) const;
	std::vector<int> getLFOFMUsers(int lfoNum) const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, bool enabled);

private:
	std::array<std::shared_ptr<AbstructInstrument>, 128> insts_;
	std::array<std::shared_ptr<EnvelopeFM>, 128> envFM_;
	std::array<std::shared_ptr<LFOFM>, 128> lfoFM_;

	int cloneFMEnvelope(int srcNum);
	int cloneFMLFO(int srcNum);
};
