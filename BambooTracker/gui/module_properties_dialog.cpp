/*
 * Copyright (C) 2018-2020 Rerrah
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

#include "module_properties_dialog.hpp"
#include "ui_module_properties_dialog.h"
#include <vector>
#include <utility>
#include <unordered_map>
#include "gui/gui_utils.hpp"
#include "enum_hash.hpp"

namespace
{
const std::unordered_map<SongType, QString> SONG_TYPE_TEXT = {
	{ SongType::Standard, QT_TRANSLATE_NOOP("SongType", "Standard") },
	{ SongType::FM3chExpanded, QT_TRANSLATE_NOOP("SongType", "FM3ch expanded") }
};
}

ModulePropertiesDialog::ModulePropertiesDialog(std::weak_ptr<BambooTracker> core, double configFmMixer,
											   double configSsgMixer, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ModulePropertiesDialog),
	  bt_(core),
	  configFmMixer_(configFmMixer),
	  configSsgMixer_(configSsgMixer)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	int tickFreq = static_cast<int>(core.lock()->getModuleTickFrequency());
	ui->customTickFreqSpinBox->setValue(tickFreq);
	switch (tickFreq) {
	case 60:	ui->ntscRadioButton->setChecked(true);				break;
	case 50:	ui->palRadioButton->setChecked(true);				break;
	default:	ui->customTickFreqRadioButton->setChecked(true);	break;
	}

	MixerType mixType = core.lock()->getModuleMixerType();
	if (mixType == MixerType::UNSPECIFIED) {
		ui->mixerGroupBox->setChecked(false);
	}
	else {
		ui->mixerGroupBox->setChecked(true);
		ui->mixerTypeComboBox->setCurrentIndex(static_cast<int>(mixType) - 1);
	}
	setCustomMixerLevels(core.lock()->getModuleCustomMixerFMLevel(), core.lock()->getModuleCustomMixerSSGLevel());

	ui->songTreeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	int songCnt = static_cast<int>(core.lock()->getSongCount());
	for (int i = 0; i < songCnt; ++i) {
		auto title = core.lock()->getSongTitle(i);
		insertSong(i, gui_utils::utf8ToQString(title), core.lock()->getSongStyle(i).type, i);
	}

	ui->sngTypeComboBox->addItem(SONG_TYPE_TEXT.at(SongType::Standard), static_cast<int>(SongType::Standard));
	ui->sngTypeComboBox->addItem(SONG_TYPE_TEXT.at(SongType::FM3chExpanded), static_cast<int>(SongType::FM3chExpanded));
}

ModulePropertiesDialog::~ModulePropertiesDialog()
{
	delete ui;
}

void ModulePropertiesDialog::insertSong(int row, QString title, SongType type, int prevNum)
{
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, QString::number(row));
	item->setData(0, Qt::UserRole, prevNum);
	item->setText(1, title);
	item->setText(2, SONG_TYPE_TEXT.at(type));
	item->setData(2, Qt::UserRole, static_cast<int>(type));
	ui->songTreeWidget->insertTopLevelItem(row, item);

	for (int i = row + 1; i < ui->songTreeWidget->topLevelItemCount(); ++i) {
		ui->songTreeWidget->topLevelItem(i)->setText(0, QString::number(i));
	}

	checkButtonsEnabled();
}

void ModulePropertiesDialog::checkButtonsEnabled()
{
	if (ui->songTreeWidget->currentItem() != nullptr
			&& ui->songTreeWidget->topLevelItemCount() > 1) {
		ui->upToolButton->setEnabled(true);
		ui->downToolButton->setEnabled(true);
		ui->removePushButton->setEnabled(true);
	}
	else {
		ui->upToolButton->setEnabled(false);
		ui->downToolButton->setEnabled(false);
		ui->removePushButton->setEnabled(false);
	}
}

void ModulePropertiesDialog::swapset(int aboveRow, int belowRow)
{
	auto* tree = ui->songTreeWidget;
	QTreeWidgetItem* below = tree->takeTopLevelItem(belowRow);
	if (tree->topLevelItemCount() > 2) {
		QTreeWidgetItem* above = tree->takeTopLevelItem(aboveRow);
		tree->insertTopLevelItem(aboveRow, below);
		tree->insertTopLevelItem(belowRow, above);
	}
	else {
		tree->insertTopLevelItem(aboveRow, below);
	}

	for (int i = aboveRow; i < ui->songTreeWidget->topLevelItemCount(); ++i) {
		ui->songTreeWidget->topLevelItem(i)->setText(0, QString::number(i));
	}
}

void ModulePropertiesDialog::setCustomMixerLevels(double fm, double ssg)
{
	fmMixer_ = fm;
	ssgMixer_ = ssg;

	ui->customMixerFMLevelLabel->setText(QString::asprintf("%+.1fdB", fmMixer_));
	ui->customMixerSSGLevelLabel->setText(QString::asprintf("%+.1fdB", ssgMixer_));
}

/******************************/
void ModulePropertiesDialog::on_upToolButton_clicked()
{
	int curRow = ui->songTreeWidget->currentIndex().row();
	if (!curRow) return;

	swapset(curRow - 1, curRow);
	ui->songTreeWidget->setCurrentItem(ui->songTreeWidget->topLevelItem(curRow - 1));
}

void ModulePropertiesDialog::on_downToolButton_clicked()
{
	int curRow = ui->songTreeWidget->currentIndex().row();
	if (curRow == ui->songTreeWidget->topLevelItemCount() - 1) return;

	swapset(curRow, curRow + 1);
	ui->songTreeWidget->setCurrentItem(ui->songTreeWidget->topLevelItem(curRow + 1));
}

void ModulePropertiesDialog::on_removePushButton_clicked()
{
	int row = ui->songTreeWidget->currentIndex().row();
	auto del = ui->songTreeWidget->takeTopLevelItem(row);
	delete del;
	for (int i = row; i < ui->songTreeWidget->topLevelItemCount(); ++ i) {
		ui->songTreeWidget->topLevelItem(i)->setText(0, QString::number(i));
	}

	checkButtonsEnabled();
}

void ModulePropertiesDialog::on_insertPushButton_clicked()
{
	int row = ui->songTreeWidget->currentIndex().row();
	if (row == -1) row = ui->songTreeWidget->topLevelItemCount();

	insertSong(row,
			   ui->sngTitleLineEdit->text(),
			   static_cast<SongType>(ui->sngTypeComboBox->currentData(Qt::UserRole).toInt()));
	ui->songTreeWidget->setCurrentItem(ui->songTreeWidget->topLevelItem(row));
}

void ModulePropertiesDialog::on_songTreeWidget_itemSelectionChanged()
{
	auto item = ui->songTreeWidget->currentItem();
	ui->sngTitleLineEdit->setText(item->text(1));
	int type = item->data(2, Qt::UserRole).toInt();
	for (int i = 0; i < ui->sngTypeComboBox->count(); ++i) {
		if (ui->sngTypeComboBox->itemData(i, Qt::UserRole).toInt() == type) {
			ui->sngTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	checkButtonsEnabled();
}

void ModulePropertiesDialog::onAccepted()
{
	// Set tick frequency
	unsigned int tickFreq;
	if (ui->ntscRadioButton->isChecked()) tickFreq = 60;
	else if (ui->palRadioButton->isChecked()) tickFreq = 50;
	else tickFreq = static_cast<unsigned int>(ui->customTickFreqSpinBox->value());
	bt_.lock()->setModuleTickFrequency(tickFreq);

	// Set mixer
	if (ui->mixerGroupBox->isChecked()) {
		auto mixType = static_cast<MixerType>(ui->mixerTypeComboBox->currentIndex() + 1);
		bt_.lock()->setModuleMixerType(mixType);
		if (mixType == MixerType::CUSTOM) {
			bt_.lock()->setModuleCustomMixerFMLevel(fmMixer_);
			bt_.lock()->setModuleCustomMixerSSGLevel(ssgMixer_);
		}
	}
	else {
		bt_.lock()->setModuleMixerType(MixerType::UNSPECIFIED);
	}

	auto* tree = ui->songTreeWidget;
	std::vector<int> newSongNums;

	for (int i = 0; i < tree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = tree->topLevelItem(i);
		SongType type = static_cast<SongType>(item->data(2, Qt::UserRole).toInt());
		std::string title = item->text(1).toUtf8().toStdString();
		if (item->data(0, Qt::UserRole).toInt() == -1) {	// Add new song
			int n = static_cast<int>(bt_.lock()->getSongCount());
			bt_.lock()->addSong(type, title);
			newSongNums.push_back(n);
		}
		else {	// Update song data
			int n = item->data(0, Qt::UserRole).toInt();
			bt_.lock()->setSongTitle(n, title);
			if (bt_.lock()->getSongStyle(n).type != type)
				bt_.lock()->changeSongType(n, type);
			newSongNums.push_back(n);
		}
	}

	// Sort songs
	bt_.lock()->sortSongs(std::move(newSongNums));
}

void ModulePropertiesDialog::on_mixerTypeComboBox_currentIndexChanged(int index)
{
	ui->mixerCustomGroupBox->setEnabled(index == 0);
}

void ModulePropertiesDialog::on_customMixerSetPushButton_clicked()
{
	setCustomMixerLevels(configFmMixer_, configSsgMixer_);
}

void ModulePropertiesDialog::on_updateButton_clicked()
{
	if (auto item = ui->songTreeWidget->currentItem()) {
		item->setText(1, ui->sngTitleLineEdit->text());
		auto typeInt = ui->sngTypeComboBox->currentData(Qt::UserRole).toInt();
		item->setText(2, SONG_TYPE_TEXT.at(static_cast<SongType>(typeInt)));
		item->setData(2, Qt::UserRole, typeInt);
	}
}
