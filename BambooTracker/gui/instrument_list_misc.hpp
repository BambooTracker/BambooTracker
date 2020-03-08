#ifndef INSTRUMENT_LIST_MISC_HPP
#define INSTRUMENT_LIST_MISC_HPP

#include <unordered_map>
#include <QString>
#include <QListWidgetItem>
#include <QIcon>
#include "enum_hash.hpp"
#include "misc.hpp"

static const std::unordered_map<SoundSource, const char*> ICON_SRC = {
	{ SoundSource::FM, ":/icon/inst_fm" },
	{ SoundSource::SSG, ":/icon/inst_ssg" },
	{ SoundSource::ADPCM, ":/icon/inst_adpcm" }
};

inline QString makeInstrumentListText(int num, QString name)
{
	return QString("%1: %2").arg(num, 2, 16, QChar('0')).toUpper().arg(name);
}

inline QListWidgetItem* createInstrumentListItem(int num, SoundSource src, QString name)
{
	QListWidgetItem *item = new QListWidgetItem(QIcon(ICON_SRC.at(src)), makeInstrumentListText(num, name));
	item->setSizeHint(QSize(130, 17));
	item->setData(Qt::UserRole, num);
	return item;
}

#endif // INSTRUMENT_LIST_MISC_HPP
