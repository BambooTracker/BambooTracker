#pragma once

#include <QDialog>
#include <QVector>
#include <memory>

class AbstractBank;

namespace Ui {
	class InstrumentSelectionDialog;
};

class InstrumentSelectionDialog : public QDialog
{
public:
	InstrumentSelectionDialog(const AbstractBank &bank, const QString &text, QWidget *parent = nullptr);
	~InstrumentSelectionDialog();

	QVector<size_t> currentInstrumentSelection() const;

private:
	const AbstractBank &bank_;
	std::unique_ptr<Ui::InstrumentSelectionDialog> ui_;

	void setupContents();
};
