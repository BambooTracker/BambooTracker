#include "instruments_manager.hpp"
#include <utility>
#include "instrument_fm.hpp"
#include "instrument_psg.hpp"

InstrumentsManager::InstrumentsManager() {}

AbstructInstrument InstrumentsManager::addInstrument(int num, SoundSource source, std::string name)
{
	switch (source) {
	case SoundSource::FM:	map_.emplace(num, InstrumentFM(num, name));		break;
	case SoundSource::PSG:	map_.emplace(num, InstrumentPSG(num, name));	break;
	}

	return map_.at(num);
}

AbstructInstrument InstrumentsManager::addInstrument(AbstructInstrument inst)
{
	map_.emplace(inst.getNumber(), inst);
	return map_.at(inst.getNumber());
}

AbstructInstrument InstrumentsManager::removeInstrument(int n)
{
	AbstructInstrument ret = map_.at(n);
	map_.erase(n);
	return std::move(ret);
}
