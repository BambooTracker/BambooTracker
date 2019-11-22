#ifndef EFFECTDESCRIPTION_HPP
#define EFFECTDESCRIPTION_HPP

#include <unordered_map>
#include <QObject>
#include <QString>
#include "effect.hpp"
#include "enum_hash.hpp"

class EffectDescription : public QObject
{
	Q_OBJECT

public:
	static QString getEffectFormat(const EffectType type);
	static QString getEffectDescription(const EffectType type);
	static QString getEffectFormatAndDetailString(const EffectType type);

private:
	EffectDescription();

	struct EffectDetail
	{
		const QString format, desc;

		QString mergedString() const { return format + " - " + desc; }
	};

	static const std::unordered_map<EffectType, EffectDetail> details_;
};

#endif // EFFECTDESCRIPTION_HPP
