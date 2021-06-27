/*
 * Copyright (C) 2021 Rerrah
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

#include "key_signature_manager_form.hpp"
#include "ui_key_signature_manager_form.h"
#include "gui/note_name_manager.hpp"

KeySignatureManagerForm::KeySignatureManagerForm(std::weak_ptr<BambooTracker> core, bool showHex,
												 QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::KeySignatureManagerForm),
	  bt_(core),
	  curSong_(core.lock()->getCurrentSongNumber())
{
	ui->setupUi(this);
	setWindowFlags((windowFlags()
				   & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint))
				   | Qt::WindowStaysOnTopHint);

	for (int i = KeySignature::FISRT; i <= KeySignature::LAST; ++i) {
		ui->keyComboBox->addItem(NoteNameManager::getManager().getKeyName(static_cast<KeySignature::Type>(i)), i);
	}

	setNumberSettings(showHex);
	initList();
	ui->orderSpinBox->setDisplayIntegerBase(numBase_);
	ui->stepSpinBox->setDisplayIntegerBase(numBase_);

	insSc_ = std::make_unique<QShortcut>(Qt::Key_Insert, ui->listWidget, nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(insSc_.get(), &QShortcut::activated, this, &KeySignatureManagerForm::on_createPushButton_clicked);
	delSc_ = std::make_unique<QShortcut>(Qt::Key_Delete, ui->listWidget, nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(delSc_.get(), &QShortcut::activated, this, &KeySignatureManagerForm::on_removePushButton_clicked);
}

KeySignatureManagerForm::~KeySignatureManagerForm()
{
	delete ui;
}

void KeySignatureManagerForm::initList()
{
	int size = static_cast<int>(bt_.lock()->getKeySignatureSize(curSong_));
	for (int i = 0; i < size; ++i) {
		KeySignature ks = bt_.lock()->getKeySignature(curSong_, i);
		addKeySignature(ks.type, ks.order, ks.step, true);
	}
}

void KeySignatureManagerForm::addKeySignature(KeySignature::Type key, int order, int step, bool onlyUi)
{
	if (onlyUi) {
		ui->listWidget->addItem(createText(key, order, step));
	}
	else {
		bt_.lock()->addKeySignature(curSong_, key, order, step);
		ui->listWidget->clear();
		initList();
	}
}

void KeySignatureManagerForm::removeKeySignature(int i)
{
	if (!i) return;
	delete ui->listWidget->takeItem(i);
	bt_.lock()->removeKeySignature(curSong_, i);
}

QString KeySignatureManagerForm::createText(KeySignature::Type key, int order, int step)
{
	auto pos = QString("%1,%2").arg(order, numWidth_, numBase_, QChar('0'))
			   .arg(step, numWidth_, numBase_, QChar('0')).toUpper();
	return (pos + ": " + NoteNameManager::getManager().getKeyName(key));
}

void KeySignatureManagerForm::onCurrentSongNumberChanged()
{
	curSong_ = bt_.lock()->getCurrentSongNumber();

	ui->listWidget->clear();
	initList();
}

void KeySignatureManagerForm::onConfigurationChanged(bool showHex)
{
	for (int i = KeySignature::FISRT; i <= KeySignature::LAST; ++i) {
		ui->keyComboBox->setItemText(i, NoteNameManager::getManager().getKeyName(static_cast<KeySignature::Type>(i)));
	}

	setNumberSettings(showHex);

	for (int i = 0; i < ui->listWidget->count(); ++i) {
		KeySignature ks = bt_.lock()->getKeySignature(curSong_, i);
		ui->listWidget->item(i)->setText(createText(ks.type, ks.order, ks.step));
	}

	ui->orderSpinBox->setDisplayIntegerBase(numBase_);
	ui->stepSpinBox->setDisplayIntegerBase(numBase_);
}

void KeySignatureManagerForm::on_createPushButton_clicked()
{
	if (bt_.lock()->getKeySignatureSize(curSong_) == 127) return;	// Maximum size
	addKeySignature(static_cast<KeySignature::Type>(ui->keyComboBox->currentData().toInt()),
					ui->orderSpinBox->value(), ui->stepSpinBox->value());
	emit modified();
}

void KeySignatureManagerForm::on_updatePushButton_clicked()
{
	int row = ui->listWidget->currentRow();
	if (row == -1) return;

	auto key = static_cast<KeySignature::Type>(ui->keyComboBox->currentData().toInt());
	int order = ui->orderSpinBox->value();
	int step = ui->stepSpinBox->value();
	if (!row && (order || step)) bt_.lock()->addKeySignature(curSong_, key, order, step);
	else bt_.lock()->changeKeySignature(curSong_, row, key, order, step);

	ui->listWidget->clear();
	initList();

	emit modified();
}

void KeySignatureManagerForm::on_removePushButton_clicked()
{
	int row = ui->listWidget->currentRow();
	if (row < 2) return;

	removeKeySignature(row);
	emit modified();
}

void KeySignatureManagerForm::on_clearAllPushButton_clicked()
{
	if (!ui->listWidget->count()) return;

	ui->listWidget->clear();
	bt_.lock()->clearKeySignature(curSong_);
	initList();

	emit modified();
}

void KeySignatureManagerForm::on_listWidget_currentRowChanged(int currentRow)
{
	if (currentRow == -1) return;

	KeySignature ks = bt_.lock()->getKeySignature(curSong_, currentRow);
	for (int i = 0; i < ui->keyComboBox->count(); ++i) {
		if (static_cast<KeySignature::Type>(ui->keyComboBox->itemData(i).toInt()) == ks.type) {
			ui->keyComboBox->setCurrentIndex(i);
			break;
		}
	}
	ui->orderSpinBox->setValue(ks.order);
	ui->stepSpinBox->setValue(ks.step);
}

void KeySignatureManagerForm::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
	int row = ui->listWidget->row(item);
	KeySignature ks = bt_.lock()->getKeySignature(curSong_, row);
	emit positionJumpRequested(ks.order, ks.step);
}
