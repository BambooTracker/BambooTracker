/*
 * Copyright (C) 2019-2021 Rerrah
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

#include "fm_envelope_set_edit_dialog.hpp"
#include "ui_fm_envelope_set_edit_dialog.h"
#include <QTreeWidgetItem>
#include "configuration.hpp"

FMEnvelopeSetEditDialog::FMEnvelopeSetEditDialog(std::vector<FMEnvelopeTextType> set, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FMEnvelopeSetEditDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	for (size_t i = 0; i < set.size(); ++i) {
		insertRow(static_cast<int>(i), set.at(i));
	}
}

FMEnvelopeSetEditDialog::~FMEnvelopeSetEditDialog()
{
	delete ui;
}

std::vector<FMEnvelopeTextType> FMEnvelopeSetEditDialog::getSet()
{
	std::vector<FMEnvelopeTextType> set;
	for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
		set.push_back(static_cast<FMEnvelopeTextType>(
						  qobject_cast<QComboBox*>(ui->treeWidget->itemWidget(
													   ui->treeWidget->topLevelItem(i), 1))->currentData().toInt()));
	}
	return set;
}

void FMEnvelopeSetEditDialog::swapset(int aboveRow, int belowRow)
{
	auto* tree = ui->treeWidget;
	QComboBox* belowBox = makeCombobox();
	belowBox->setCurrentIndex(qobject_cast<QComboBox*>(tree->itemWidget(tree->topLevelItem(belowRow), 1))->currentIndex());
	QTreeWidgetItem* below = tree->takeTopLevelItem(belowRow);
	if (tree->topLevelItemCount() > 2) {
		QComboBox* aboveBox = makeCombobox();
		aboveBox->setCurrentIndex(qobject_cast<QComboBox*>(tree->itemWidget(tree->topLevelItem(aboveRow), 1))->currentIndex());
		QTreeWidgetItem* above = tree->takeTopLevelItem(aboveRow);
		tree->insertTopLevelItem(aboveRow, below);
		tree->insertTopLevelItem(belowRow, above);
		tree->setItemWidget(below, 1, belowBox);
		tree->setItemWidget(above, 1, aboveBox);
	}
	else {
		tree->insertTopLevelItem(aboveRow, below);
		tree->setItemWidget(below, 1, belowBox);
	}

	if (!aboveRow || !belowRow) alignTreeOn1stItemChanged();	// Dummy set and delete to align

	for (int i = aboveRow; i < ui->treeWidget->topLevelItemCount(); ++i) {
		ui->treeWidget->topLevelItem(i)->setText(0, QString::number(i));
	}
}

void FMEnvelopeSetEditDialog::insertRow(int row, FMEnvelopeTextType type)
{
	if (row == -1) row = 0;
	auto item = new QTreeWidgetItem();
	item->setText(0, QString::number(row));
	QComboBox* box = makeCombobox();
	for (int i = 0; i < box->count(); ++i) {
		if (static_cast<FMEnvelopeTextType>(box->itemData(i).toInt()) == type) {
			box->setCurrentIndex(i);
			break;
		}
	}
	ui->treeWidget->insertTopLevelItem(row, item);
	ui->treeWidget->setItemWidget(item, 1, box);

	if (!row) alignTreeOn1stItemChanged();	// Dummy set and delete to align

	for (int i = row + 1; i < ui->treeWidget->topLevelItemCount(); ++i) {
		ui->treeWidget->topLevelItem(i)->setText(0, QString::number(i));
	}
}

QComboBox* FMEnvelopeSetEditDialog::makeCombobox()
{
	auto box = new QComboBox();
	box->addItem(tr("Skip"), static_cast<int>(FMEnvelopeTextType::Skip));
	box->addItem("AL", static_cast<int>(FMEnvelopeTextType::AL));
	box->addItem("FB", static_cast<int>(FMEnvelopeTextType::FB));
	box->addItem("AR1", static_cast<int>(FMEnvelopeTextType::AR1));
	box->addItem("DR1", static_cast<int>(FMEnvelopeTextType::DR1));
	box->addItem("SR1", static_cast<int>(FMEnvelopeTextType::SR1));
	box->addItem("RR1", static_cast<int>(FMEnvelopeTextType::RR1));
	box->addItem("SL1", static_cast<int>(FMEnvelopeTextType::SL1));
	box->addItem("TL1", static_cast<int>(FMEnvelopeTextType::TL1));
	box->addItem("KS1", static_cast<int>(FMEnvelopeTextType::KS1));
	box->addItem("ML1", static_cast<int>(FMEnvelopeTextType::ML1));
	box->addItem("DT1", static_cast<int>(FMEnvelopeTextType::DT1));
	box->addItem("AR2", static_cast<int>(FMEnvelopeTextType::AR2));
	box->addItem("DR2", static_cast<int>(FMEnvelopeTextType::DR2));
	box->addItem("SR2", static_cast<int>(FMEnvelopeTextType::SR2));
	box->addItem("RR2", static_cast<int>(FMEnvelopeTextType::RR2));
	box->addItem("SL2", static_cast<int>(FMEnvelopeTextType::SL2));
	box->addItem("TL2", static_cast<int>(FMEnvelopeTextType::TL2));
	box->addItem("KS2", static_cast<int>(FMEnvelopeTextType::KS2));
	box->addItem("ML2", static_cast<int>(FMEnvelopeTextType::ML2));
	box->addItem("DT2", static_cast<int>(FMEnvelopeTextType::DT2));
	box->addItem("AR3", static_cast<int>(FMEnvelopeTextType::AR3));
	box->addItem("DR3", static_cast<int>(FMEnvelopeTextType::DR3));
	box->addItem("SR3", static_cast<int>(FMEnvelopeTextType::SR3));
	box->addItem("RR3", static_cast<int>(FMEnvelopeTextType::RR3));
	box->addItem("SL3", static_cast<int>(FMEnvelopeTextType::SL3));
	box->addItem("TL3", static_cast<int>(FMEnvelopeTextType::TL3));
	box->addItem("KS3", static_cast<int>(FMEnvelopeTextType::KS3));
	box->addItem("ML3", static_cast<int>(FMEnvelopeTextType::ML3));
	box->addItem("DT3", static_cast<int>(FMEnvelopeTextType::DT3));
	box->addItem("AR4", static_cast<int>(FMEnvelopeTextType::AR4));
	box->addItem("DR4", static_cast<int>(FMEnvelopeTextType::DR4));
	box->addItem("SR4", static_cast<int>(FMEnvelopeTextType::SR4));
	box->addItem("RR4", static_cast<int>(FMEnvelopeTextType::RR4));
	box->addItem("SL4", static_cast<int>(FMEnvelopeTextType::SL4));
	box->addItem("TL4", static_cast<int>(FMEnvelopeTextType::TL4));
	box->addItem("KS4", static_cast<int>(FMEnvelopeTextType::KS4));
	box->addItem("ML4", static_cast<int>(FMEnvelopeTextType::ML4));
	box->addItem("DT4", static_cast<int>(FMEnvelopeTextType::DT4));
	return box;
}

/// Dummy set and delete to align
void FMEnvelopeSetEditDialog::alignTreeOn1stItemChanged()
{
	auto tmp = new QTreeWidgetItem();
	ui->treeWidget->insertTopLevelItem(1, tmp);
	delete ui->treeWidget->takeTopLevelItem(1);
}

void FMEnvelopeSetEditDialog::on_upToolButton_clicked()
{
	int curRow = ui->treeWidget->currentIndex().row();
	if (!curRow) return;

	swapset(curRow - 1, curRow);
	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(curRow - 1));
}

void FMEnvelopeSetEditDialog::on_downToolButton_clicked()
{
	int curRow = ui->treeWidget->currentIndex().row();
	if (curRow == ui->treeWidget->topLevelItemCount() - 1) return;

	swapset(curRow, curRow + 1);
	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(curRow + 1));
}

void FMEnvelopeSetEditDialog::on_addPushButton_clicked()
{
	int row = ui->treeWidget->currentIndex().row();
	insertRow(row, FMEnvelopeTextType::Skip);

	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem((row == -1) ? 0 : row));
	ui->upToolButton->setEnabled(true);
	ui->downToolButton->setEnabled(true);
	ui->removePushButton->setEnabled(true);
}

void FMEnvelopeSetEditDialog::on_removePushButton_clicked()
{
	int row = ui->treeWidget->currentIndex().row();
	delete ui->treeWidget->takeTopLevelItem(row);

	for (int i = row; i < ui->treeWidget->topLevelItemCount(); ++i) {
		ui->treeWidget->topLevelItem(i)->setText(0, QString::number(i));
	}

	if (!ui->treeWidget->topLevelItemCount()) {
		ui->upToolButton->setEnabled(false);
		ui->downToolButton->setEnabled(false);
		ui->removePushButton->setEnabled(false);
	}
}

void FMEnvelopeSetEditDialog::on_treeWidget_itemSelectionChanged()
{
	if (ui->treeWidget->currentIndex().row() == -1) {
		ui->upToolButton->setEnabled(false);
		ui->downToolButton->setEnabled(false);
		ui->removePushButton->setEnabled(false);
	}
	else {
		ui->upToolButton->setEnabled(true);
		ui->downToolButton->setEnabled(true);
		ui->removePushButton->setEnabled(true);
	}
}
