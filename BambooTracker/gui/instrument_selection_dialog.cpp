/*
 * Copyright (C) 2018-2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "instrument_selection_dialog.hpp"
#include "ui_instrument_selection_dialog.h"
#include <stdexcept>
#include <QKeyEvent>
#include "bank.hpp"
#include "gui/jam_layout.hpp"

InstrumentSelectionDialog::InstrumentSelectionDialog(const AbstractBank &bank, const QString &text,
													 std::weak_ptr<Configuration> config, QWidget *parent)
	: QDialog(parent), bank_(bank), config_(config), ui_(new Ui::InstrumentSelectionDialog)
{
	ui_->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui_->label->setText(text);
	setupContents();

	ui_->listWidget->installEventFilter(this);
}

InstrumentSelectionDialog::~InstrumentSelectionDialog()
{
}

void InstrumentSelectionDialog::setupContents()
{
	QListWidget* lw = ui_->listWidget;
	lw->setSelectionMode(QListWidget::MultiSelection);

	size_t instCount = bank_.getNumInstruments();
	for (size_t i = 0; i < instCount; ++i) {
		QString id = QString::fromStdString(bank_.getInstrumentIdentifier(i));
		QString name = QString::fromStdString(bank_.getInstrumentName(i));

		QListWidgetItem* item = new QListWidgetItem(QString("%1 %2").arg(id, name));
		item->setData(Qt::UserRole, static_cast<qulonglong>(i));
		lw->addItem(item);
	}
}

QVector<size_t> InstrumentSelectionDialog::currentInstrumentSelection() const
{
	QListWidget *lw = ui_->listWidget;
	const QList<QListWidgetItem *> items = lw->selectedItems();

	QVector<size_t> selection;
	selection.reserve(items.size());

	for (const QListWidgetItem* item : items) {
		size_t index = static_cast<size_t>(item->data(Qt::UserRole).toULongLong());
		selection.push_back(index);
	}

	return selection;
}

void InstrumentSelectionDialog::onJamKeyOnByMidi(int key)
{
	auto item = ui_->listWidget->currentItem();
	if (item) emit jamKeyOnMidiEvent(static_cast<size_t>(item->data(Qt::UserRole).toULongLong()), key);
}

void InstrumentSelectionDialog::onJamKeyOffByMidi(int key)
{
	auto item = ui_->listWidget->currentItem();
	if (item) emit jamKeyOffMidiEvent(key);
}

bool InstrumentSelectionDialog::eventFilter(QObject* watched, QEvent* event)
{
	Q_UNUSED(watched)	// Only list widget

	auto item = ui_->listWidget->currentItem();
	if (!item) return false;
	size_t n = static_cast<size_t>(item->data(Qt::UserRole).toULongLong());

	if (event->type() == QEvent::KeyPress) {
		auto ke = reinterpret_cast<QKeyEvent*>(event);
		if (!ke->isAutoRepeat()) {
			Qt::Key qtKey = static_cast<Qt::Key>(ke->key());
			try {
				JamKey jk = getJamKeyFromLayoutMapping(qtKey, config_);
				emit jamKeyOnEvent(n, jk);
			} catch (std::invalid_argument&) {}
		}
	}
	else if (event->type() == QEvent::KeyRelease) {
		auto ke = reinterpret_cast<QKeyEvent*>(event);
		if (!ke->isAutoRepeat()) {
			Qt::Key qtKey = static_cast<Qt::Key>(ke->key());
			try {
				JamKey jk = getJamKeyFromLayoutMapping(qtKey, config_);
				emit jamKeyOffEvent(jk);
			} catch (std::invalid_argument&) {}
		}
	}

	return false;
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
