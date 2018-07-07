#pragma once

#include <string>
#include <memory>
#include "abstruct_instrument.hpp"

class InstrumentPSG : public AbstructInstrument
{
public:
	InstrumentPSG(int number, std::string name);
	std::unique_ptr<AbstructInstrument> clone() override;
};
