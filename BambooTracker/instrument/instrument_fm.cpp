#include "instrument_fm.hpp"
#include "misc.hpp"

#include <QDebug>
#include <QString>

InstrumentFM::InstrumentFM(int number, std::string name) :
	AbstructInstrument(number, SoundSource::FM, name),
	al_(4), fb_(0)
{
	for (auto& o : op_) {
		o = FMOperator{ 31, 0, 0, 7, 0, 32, 0, 0, 0, 0 };
	}

	qDebug() << QString::fromUtf8(name.c_str(), name.length());
}

std::unique_ptr<AbstructInstrument> InstrumentFM::clone()
{
	return std::unique_ptr<AbstructInstrument>(std::make_unique<InstrumentFM>(*this));
}
