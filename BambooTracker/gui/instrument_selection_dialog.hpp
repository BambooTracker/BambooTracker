#pragma once

#include <QDialog>
#include <QVector>
#include <memory>
#include "configuration.hpp"

class AbstractBank;

namespace Ui {
	class InstrumentSelectionDialog;
};

class InstrumentSelectionDialog : public QDialog
{
	Q_OBJECT

public:
	InstrumentSelectionDialog(const AbstractBank &bank, const QString &text,
							  std::weak_ptr<Configuration> config, QWidget *parent = nullptr);
	~InstrumentSelectionDialog() override;

	QVector<size_t> currentInstrumentSelection() const;

public slots:
	void onJamKeyOnByMidi(int key);
	void onJamKeyOffByMidi(int key);

signals:
	void jamKeyOnEvent(size_t id, JamKey key);
	void jamKeyOnMidiEvent(size_t id, int key);
	void jamKeyOffEvent(JamKey key);
	void jamKeyOffMidiEvent(int key);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	const AbstractBank &bank_;
	std::weak_ptr<Configuration> config_;
	std::unique_ptr<Ui::InstrumentSelectionDialog> ui_;

	void setupContents();

private slots:
	void on_searchLineEdit_textChanged(const QString &search);
};
