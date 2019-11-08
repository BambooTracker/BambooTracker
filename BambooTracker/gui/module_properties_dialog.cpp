#include "module_properties_dialog.hpp"
#include "ui_module_properties_dialog.h"
#include <vector>
#include <utility>

ModulePropertiesDialog::ModulePropertiesDialog(std::weak_ptr<BambooTracker> core, double configFmMixer,
											   double configSsgMixer, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ModulePropertiesDialog),
	  bt_(core),
	  configFmMixer_(configFmMixer),
	  configSsgMixer_(configSsgMixer)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

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

	ui->songTreeWidget->setColumnCount(3);
	ui->songTreeWidget->setHeaderLabels({ tr("Number"), tr("Title"), tr("Song type") });
	ui->songTreeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	int songCnt = static_cast<int>(core.lock()->getSongCount());
	for (int i = 0; i < songCnt; ++i) {
		auto title = core.lock()->getSongTitle(i);
		insertSong(i, QString::fromUtf8(title.c_str(), static_cast<int>(title.length())),
				   core.lock()->getSongStyle(i).type, i);
	}

	ui->insertTypeComboBox->addItem(tr("Standard"), static_cast<int>(SongType::Standard));
	ui->insertTypeComboBox->addItem(tr("FM3ch expanded"), static_cast<int>(SongType::FM3chExpanded));
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
	switch (type) {
	case SongType::Standard:		item->setText(2, tr("Standard"));		break;
	case SongType::FM3chExpanded:	item->setText(2, tr("FM3ch expanded"));	break;
	}
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

	QString fmSign = (fm > -1) ? "+" : "";
	ui->customMixerFMLevelLabel->setText(fmSign + QString::number(fmMixer_, 'f', 1) + "dB");
	QString ssgSign = (ssg > -1) ? "+" : "";
	ui->customMixerSSGLevelLabel->setText(ssgSign + QString::number(ssgMixer_, 'f', 1) + "dB");
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
			   ui->insertTitleLineEdit->text(),
			   static_cast<SongType>(ui->insertTypeComboBox->currentData(Qt::UserRole).toInt()));
	ui->songTreeWidget->setCurrentItem(ui->songTreeWidget->topLevelItem(row));
}

void ModulePropertiesDialog::on_songTreeWidget_itemSelectionChanged()
{
	ui->editTitleLineEdit->setText(ui->songTreeWidget->currentItem()->text(1));
	checkButtonsEnabled();
}

void ModulePropertiesDialog::on_editTitleLineEdit_textEdited(const QString &arg1)
{
	if (ui->songTreeWidget->currentItem() != nullptr)
		ui->songTreeWidget->currentItem()->setText(1, arg1);
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
		if (item->data(0, Qt::UserRole).toInt() == -1) {	// Add new song
			int n = static_cast<int>(bt_.lock()->getSongCount());
			bt_.lock()->addSong(static_cast<SongType>(item->data(2, Qt::UserRole).toInt()),
								item->text(1).toUtf8().toStdString());
			newSongNums.push_back(n);
		}
		else {	// Update song meta data
			int n = item->data(0, Qt::UserRole).toInt();
			bt_.lock()->setSongTitle(n, item->text(1).toUtf8().toStdString());
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
