#include "instrument_psg.hpp"
#include "misc.hpp"

InstrumentPSG::InstrumentPSG(int number, std::string name) :
	AbstructInstrument(number, SoundSource::PSG, name)
{
}

std::unique_ptr<AbstructInstrument> InstrumentPSG::clone()
{
	return std::unique_ptr<AbstructInstrument>(std::make_unique<InstrumentPSG>(*this));
}
