#pragma once

#include <map>
#include <string>
#include <memory>
#include "chip.hpp"
#include "abstruct_instrument.hpp"
#include "misc.hpp"

class InstrumentsManager
{
public:
	InstrumentsManager(chip::Chip& chip);

	std::unique_ptr<AbstructInstrument> addInstrument(int num, SoundSource source, std::string name);
	std::unique_ptr<AbstructInstrument> addInstrument(std::unique_ptr<AbstructInstrument> inst);
	std::unique_ptr<AbstructInstrument> removeInstrument(int n);
	std::unique_ptr<AbstructInstrument> getInstrument(int n) const;

private:
	chip::Chip& chip_;
	std::map<int, std::unique_ptr<AbstructInstrument>> map_;
};
