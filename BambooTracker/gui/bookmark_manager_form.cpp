/*
 * Copyright (C) 2020 Rerrah
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

#include "bookmark_manager_form.hpp"
#include "ui_bookmark_manager_form.h"
#include <QKeyEvent>
#include <vector>
#include "song.hpp"
#include "gui/gui_utils.hpp"

BookmarkManagerForm::BookmarkManagerForm(std::weak_ptr<BambooTracker> core, bool showHex,
										 QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BookmarkManagerForm),
	bt_(core),
	curSong_(core.lock()->getCurrentSongNumber())
{
	ui->setupUi(this);
	setWindowFlags(windowFlags()
				   & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint));

	setNumberSettings(showHex);
	initList();
	ui->orderSpinBox->setDisplayIntegerBase(numBase_);
	ui->stepSpinBox->setDisplayIntegerBase(numBase_);

	insSc_ = std::make_unique<QShortcut>(Qt::Key_Insert, ui->listWidget, nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(insSc_.get(), &QShortcut::activated, this, &BookmarkManagerForm::on_createPushButton_clicked);
	delSc_ = std::make_unique<QShortcut>(Qt::Key_Delete, ui->listWidget, nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(delSc_.get(), &QShortcut::activated, this, &BookmarkManagerForm::on_removePushButton_clicked);
	mvUpSc_ = std::make_unique<QShortcut>(Qt::CTRL | Qt::Key_Up, ui->listWidget, nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(mvUpSc_.get(), &QShortcut::activated, this, &BookmarkManagerForm::on_upToolButton_clicked);
	mvDnSc_ = std::make_unique<QShortcut>(Qt::CTRL | Qt::Key_Down, ui->listWidget, nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(mvDnSc_.get(), &QShortcut::activated, this, &BookmarkManagerForm::on_downToolButton_clicked);
}

BookmarkManagerForm::~BookmarkManagerForm()
{
	delete ui;
}

void BookmarkManagerForm::initList()
{
	int size = static_cast<int>(bt_.lock()->getBookmarkSize(curSong_));
	for (int i = 0; i < size; ++i) {
		Bookmark bm = bt_.lock()->getBookmark(curSong_, i);
		addBookmark(gui_utils::utf8ToQString(bm.name), bm.order, bm.step, true);
	}
}

void BookmarkManagerForm::addBookmark(QString name, int order, int step, bool onlyUi)
{
	ui->listWidget->addItem(createText(name, order, step));

	if (!onlyUi) {
		bt_.lock()->addBookmark(curSong_, name.toUtf8().toStdString(), order, step);
	}
}

void BookmarkManagerForm::removeBookmark(int i)
{
	delete ui->listWidget->takeItem(i);
	bt_.lock()->removeBookmark(curSong_, i);
}

QString BookmarkManagerForm::createText(QString name, int order, int step)
{
	auto pos = QString("(%1,%2)").arg(order, numWidth_, numBase_, QChar('0'))
			   .arg(step, numWidth_, numBase_, QChar('0')).toUpper();
	return ((name.isEmpty() ? tr("Bookmark") : name) + " " + pos);
}

void BookmarkManagerForm::sortList(bool byPos)
{
	int row = ui->listWidget->currentRow();
	QString text = (row == -1) ? "" : ui->listWidget->currentItem()->text();

	ui->listWidget->clear();
	if (byPos) bt_.lock()->sortBookmarkByPosition(curSong_);
	else bt_.lock()->sortBookmarkByName(curSong_);
	initList();

	for (int i = 0; i < ui->listWidget->count(); ++i) {
		QListWidgetItem* item = ui->listWidget->item(i);
		if (item->text() == text) {
			ui->listWidget->setCurrentRow(i);
			break;
		}
	}

	emit modified();
}

void BookmarkManagerForm::onCurrentSongNumberChanged()
{
	curSong_ = bt_.lock()->getCurrentSongNumber();

	ui->listWidget->clear();
	initList();
}

void BookmarkManagerForm::onConfigurationChanged(bool showHex)
{
	if ((showHex && numBase_ == 16) || (!showHex && numBase_ == 10)) return;

	setNumberSettings(showHex);

	for (int i = 0; i < ui->listWidget->count(); ++i) {
		Bookmark bm = bt_.lock()->getBookmark(curSong_, i);
		ui->listWidget->item(i)->setText(createText(gui_utils::utf8ToQString(bm.name), bm.order, bm.step));
	}

	ui->orderSpinBox->setDisplayIntegerBase(numBase_);
	ui->stepSpinBox->setDisplayIntegerBase(numBase_);
}

void BookmarkManagerForm::onBookmarkToggleRequested(int order, int step)
{
	std::vector<int> idcs = bt_.lock()->findBookmarks(curSong_, order, step);
	if (idcs.empty()) {
		if (bt_.lock()->getBookmarkSize(curSong_) == 127) return;	// Maximum size
		int i = static_cast<int>(bt_.lock()->getBookmarkSize(curSong_));
		addBookmark(tr("Bookmark %1").arg(i), order, step);
	}
	else {
		for (auto&& it = idcs.rbegin(); it != idcs.rend(); ++it) {
			removeBookmark(*it);	// Remove from back to remain the position
		}
	}
	emit modified();
}

void BookmarkManagerForm::onBookmarkJumpRequested(bool toNext, int order, int step)
{
	if (!bt_.lock()->getBookmarkSize(curSong_)) return;
	Bookmark bm = toNext ? bt_.lock()->getNextBookmark(curSong_, order, step)
						 : bt_.lock()->getPreviousBookmark(curSong_, order, step);
	ui->listWidget->setCurrentRow(bt_.lock()->findBookmarks(curSong_, bm.order, bm.step).front());
	emit positionJumpRequested(bm.order, bm.step);
}

void BookmarkManagerForm::on_createPushButton_clicked()
{
	if (bt_.lock()->getBookmarkSize(curSong_) == 127) return;	// Maximum size
	addBookmark(ui->nameLineEdit->text(), ui->orderSpinBox->value(), ui->stepSpinBox->value());
	emit modified();
}

void BookmarkManagerForm::on_removePushButton_clicked()
{
	int row = ui->listWidget->currentRow();
	if (row == -1) return;

	removeBookmark(row);
	emit modified();
}

void BookmarkManagerForm::on_clearPushButton_clicked()
{
	if (!ui->listWidget->count()) return;

	ui->listWidget->clear();
	bt_.lock()->clearBookmark(curSong_);
	emit modified();
}

void BookmarkManagerForm::on_upToolButton_clicked()
{
	int row = ui->listWidget->currentRow();
	if (row < 1) return;

	bt_.lock()->swapBookmarks(curSong_, row, row - 1);
	ui->listWidget->clear();
	initList();
	ui->listWidget->setCurrentRow(row - 1);

	emit modified();
}

void BookmarkManagerForm::on_downToolButton_clicked()
{
	int row = ui->listWidget->currentRow();
	if (row == -1 || ui->listWidget->count() - 2 < row) return;

	bt_.lock()->swapBookmarks(curSong_, row, row + 1);
	ui->listWidget->clear();
	initList();
	ui->listWidget->setCurrentRow(row + 1);

	emit modified();
}

void BookmarkManagerForm::on_positionPushButton_clicked()
{
	sortList(true);
}

void BookmarkManagerForm::on_namePushButton_clicked()
{
	sortList(false);
}

void BookmarkManagerForm::on_listWidget_currentRowChanged(int currentRow)
{
	if (currentRow == -1) return;

	Bookmark bm = bt_.lock()->getBookmark(curSong_, currentRow);
	ui->nameLineEdit->setText(gui_utils::utf8ToQString(bm.name));
	ui->orderSpinBox->setValue(bm.order);
	ui->stepSpinBox->setValue(bm.step);
}

void BookmarkManagerForm::on_updatePushButton_clicked()
{
	int row = ui->listWidget->currentRow();
	if (row == -1) return;

	QString name = ui->nameLineEdit->text();
	int order = ui->orderSpinBox->value();
	int step = ui->stepSpinBox->value();
	ui->listWidget->item(row)->setText(createText(name, order, step));

	bt_.lock()->changeBookmark(curSong_, row, name.toUtf8().toStdString(), order, step);
	emit modified();
}

void BookmarkManagerForm::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
	int row = ui->listWidget->row(item);
	Bookmark bm = bt_.lock()->getBookmark(curSong_, row);
	emit positionJumpRequested(bm.order, bm.step);
}
