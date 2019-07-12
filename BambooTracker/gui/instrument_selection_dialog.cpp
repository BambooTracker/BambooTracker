#include "instrument_selection_dialog.hpp"
#include "ui_instrument_selection_dialog.h"
#include "bank.hpp"

InstrumentSelectionDialog::InstrumentSelectionDialog(const AbstractBank &bank, const QString &text, QWidget *parent)
	: QDialog(parent), bank_(bank), ui_(new Ui::InstrumentSelectionDialog)
{
	ui_->setupUi(this);
	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	ui_->label->setText(text);
	setupContents();
}

InstrumentSelectionDialog::~InstrumentSelectionDialog()
{
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
		item->setData(Qt::UserRole, static_cast<qulonglong>(i));
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
		size_t index = static_cast<size_t>(item->data(Qt::UserRole).toULongLong());
		selection.push_back(index);
	}

	return selection;
}

void InstrumentSelectionDialog::on_searchLineEdit_textChanged(const QString &search)
{
	QListWidget *lw = ui_->listWidget;
	unsigned count = static_cast<unsigned>(lw->count());

	for (unsigned row = 0; row < count; ++row) {
		QListWidgetItem *item = lw->item(static_cast<int>(row));
		bool accept = search.isEmpty() || item->text().contains(search, Qt::CaseInsensitive);
		item->setHidden(!accept);
	}
}
