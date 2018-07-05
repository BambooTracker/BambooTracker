#include "instrument_psg.hpp"
#include "misc.hpp"

#include <QDebug>
#include <QString>

InstrumentPSG::InstrumentPSG(int number, std::string name) :
	AbstructInstrument(number, SoundSource::PSG, name)
{
	qDebug() << QString::fromUtf8(name.c_str(), name.length());
}
