#include "instrument_psg.hpp"
#include "misc.hpp"

#include <QDebug>
#include <QString>

InstrumentPSG::InstrumentPSG(int number, std::string name) :
	AbstructInstrument(number, SoundSource::PSG, name)
{
	qDebug() << QString::fromUtf8(name.c_str(), name.length());
}

std::unique_ptr<AbstructInstrument> InstrumentPSG::clone()
{
	return std::unique_ptr<AbstructInstrument>(std::make_unique<InstrumentPSG>(*this));
}
