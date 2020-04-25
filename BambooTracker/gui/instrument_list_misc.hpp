#ifndef INSTRUMENT_LIST_MISC_HPP
#define INSTRUMENT_LIST_MISC_HPP

#include <unordered_map>
#include <QString>
#include <QListWidgetItem>
#include <QIcon>
#include "instrument.hpp"
#include "enum_hash.hpp"

static const std::unordered_map<InstrumentType, const char*> ICON_SRC = {
	{ InstrumentType::FM, ":/icon/inst_fm" },
	{ InstrumentType::SSG, ":/icon/inst_ssg" },
	{ InstrumentType::ADPCM, ":/icon/inst_adpcm" },
	{ InstrumentType::Drumkit, ":/icon/inst_kit" }
};

inline QString makeInstrumentListText(int num, QString name)
{
	return QString("%1: %2").arg(num, 2, 16, QChar('0')).toUpper().arg(name);
}

inline QListWidgetItem* createInstrumentListItem(int num, InstrumentType type, QString name)
{
	QListWidgetItem *item = new QListWidgetItem(QIcon(ICON_SRC.at(type)), makeInstrumentListText(num, name));
	item->setSizeHint(QSize(130, 17));
	item->setData(Qt::UserRole, num);
	return item;
}

#endif // INSTRUMENT_LIST_MISC_HPP
