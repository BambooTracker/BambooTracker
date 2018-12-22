#include "instrument_selection_dialog.hpp"
#include "ui_instrument_selection_dialog.h"
#include "bank.hpp"

InstrumentSelectionDialog::InstrumentSelectionDialog(const AbstractBank &bank, const QString &text, QWidget *parent)
	: QDialog(parent), bank_(bank), ui_(new Ui::InstrumentSelectionDialog) {
	ui_->setupUi(this);
	ui_->label->setText(text);
	setupContents();
}

InstrumentSelectionDialog::~InstrumentSelectionDialog() {
}

void InstrumentSelectionDialog::setupContents()
{
	QListWidget *lw = ui_->listWidget;
	lw->setSelectionMode(QListWidget::MultiSelection);

	size_t instCount = bank_.getNumInstruments();
	for (size_t i = 0; i < instCount; ++i) {
		QString id = QString::fromStdString(bank_.getInstrumentIdentifier(i));
		QString name = QString::fromStdString(bank_.getInstrumentName(i));

		QListWidgetItem *item = new QListWidgetItem(QString("%1 %2").arg(id).arg(name));
		item->setData(Qt::UserRole, (qulonglong)i);
		lw->addItem(item);
	}
}

QVector<size_t> InstrumentSelectionDialog::currentInstrumentSelection() const
{
	QListWidget *lw = ui_->listWidget;
	QList<QListWidgetItem *> items = lw->selectedItems();

	QVector<size_t> selection;
	selection.reserve(items.size());

	for (QListWidgetItem *item : items) {
		size_t index = (size_t)item->data(Qt::UserRole).toULongLong();
		selection.push_back(index);
	}

	return selection;
}
