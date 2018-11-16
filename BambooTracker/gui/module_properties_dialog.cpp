#include "module_properties_dialog.hpp"
#include "ui_module_properties_dialog.h"
#include <vector>
#include <utility>

ModulePropertiesDialog::ModulePropertiesDialog(std::weak_ptr<BambooTracker> core, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ModulePropertiesDialog),
	bt_(core)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	ui->songTreeWidget->setColumnCount(2);
	ui->songTreeWidget->setHeaderLabels({ "Number", "Title", "Song type" });
	ui->songTreeWidget->header()->resizeSection(0, 52);
	size_t songCnt = core.lock()->getSongCount();
	for (size_t i = 0; i < songCnt; ++i) {
		auto title = core.lock()->getSongTitle(i);
		insertSong(i, QString::fromUtf8(title.c_str(), title.length()), core.lock()->getSongStyle(i).type, i);
	}

	ui->insertTypeComboBox->addItem("Standard", static_cast<int>(SongType::STD));
	// ui->insertTypeComboBox->addItem("FM3ch extension", static_cast<int>(SongType::FMEX));
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
	case SongType::STD:		item->setText(2, "Standard");		break;
	case SongType::FMEX:	item->setText(2, "FM3ch expanded");	break;
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

void ModulePropertiesDialog::on_buttonBox_accepted()
{
	auto* tree = ui->songTreeWidget;
	std::vector<int> newSongNums;

	for (int i = 0; i < tree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = tree->topLevelItem(i);
		if (item->data(0, Qt::UserRole).toInt() == -1) {	// Add new song
			int n = bt_.lock()->getSongCount();
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
