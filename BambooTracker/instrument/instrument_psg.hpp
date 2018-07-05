#pragma once

#include <string>
#include "abstruct_instrument.hpp"

class InstrumentPSG : public AbstructInstrument
{
public:
	InstrumentPSG(int number, std::string name);
};
