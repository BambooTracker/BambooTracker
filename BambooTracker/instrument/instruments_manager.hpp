#pragma once

#include <map>
#include <string>
#include "abstruct_instrument.hpp"
#include "misc.hpp"

class InstrumentsManager
{
public:
	InstrumentsManager();

	AbstructInstrument addInstrument(int num, SoundSource source, std::string name);
	AbstructInstrument addInstrument(AbstructInstrument inst);
	AbstructInstrument removeInstrument(int n);

private:
	std::map<int, AbstructInstrument> map_;
};
