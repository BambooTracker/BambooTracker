#ifndef SHORTCUT_UTIL_HPP
#define SHORTCUT_UTIL_HPP

#include <string>
#include <QString>
#include <QKeySequence>

inline QKeySequence strToKeySeq(std::string str)
{
	return QKeySequence(QString::fromUtf8(str.c_str(), static_cast<int>(str.length())));
}

#endif // SHORTCUT_UTIL_HPP
