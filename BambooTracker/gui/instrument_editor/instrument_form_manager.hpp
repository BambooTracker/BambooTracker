#ifndef INSTRUMENT_FORM_MANAGER_HPP
#define INSTRUMENT_FORM_MANAGER_HPP

#include <QObject>
#include <QWidget>
#include <QString>
#include <memory>
#include <map>
#include "misc.hpp"

class InstrumentFormManager : public QObject
{
	Q_OBJECT

public:
	InstrumentFormManager();

	const std::unique_ptr<QWidget>& getForm(int n) const;
	void remove(int n);
	void add(int n, std::unique_ptr<QWidget> form, QString instName, SoundSource instSrc);

	void showForm(int n);
	void closeAll();

	QString getFormInstrumentName(int n) const;
	void setFormInstrumentName(int n, QString name);
	SoundSource getFormInstrumentSoundSource(int n) const;

public slots:
	void onInstrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentFMEnvelopeNumberChanged();
	void onInstrumentFMLFOParameterChanged(int lfoNum, int fromInstNum);
	void onInstrumentFMLFONumberChanged();
	void onInstrumentSSGWaveFormParameterChanged(int wfNum, int fromInstNum);
	void onInstrumentSSGWaveFormNumberChanged();
	void onInstrumentSSGEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentSSGEnvelopeNumberChanged();

private:
	std::map<int, std::unique_ptr<QWidget>> map_;
};


#endif // INSTRUMENT_FORM_MANAGER_HPP
