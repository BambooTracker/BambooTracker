#include "instrument_fm.hpp"
#include "misc.hpp"

#include <QDebug>
#include <QString>

InstrumentFM::InstrumentFM(int number, std::string name) :
	AbstructInstrument(number, SoundSource::FM, name)
{
	qDebug() << QString::fromUtf8(name.c_str(), name.length());
}
