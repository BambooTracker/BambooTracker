/*
 * Copyright (C) 2018-2019 Rerrah
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

#include "groove_settings_dialog.hpp"
#include "ui_groove_settings_dialog.h"
#include <algorithm>
#include <numeric>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QApplication>
#include <QClipboard>
#include "utils.hpp"

GrooveSettingsDialog::GrooveSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GrooveSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	ui->grooveListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

GrooveSettingsDialog::~GrooveSettingsDialog()
{
	delete ui;
}

void GrooveSettingsDialog::setGrooveSquences(std::vector<std::vector<int>> seqs)
{
	seqs_ = seqs;

	for (size_t i = 0; i < seqs_.size(); ++i) {
		auto text = QString("%1: ").arg(i, 2, 16, QChar('0')).toUpper();
		for (auto& g : seqs_[i]) {
			text = text + QString::number(g) + " ";
		}
		ui->grooveListWidget->addItem(text);
	}

	ui->grooveListWidget->setCurrentRow(0);
	ui->removeButton->setEnabled(ui->grooveListWidget->count() > 1);
}

std::vector<std::vector<int>> GrooveSettingsDialog::getGrooveSequences()
{
	return seqs_;
}

void GrooveSettingsDialog::keyPressEvent(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		// Prevent dialog from closing when it finished line edit
		break;
	default:
		QDialog::keyPressEvent(event);
		break;
	}
}

void GrooveSettingsDialog::on_addButton_clicked()
{
	ui->grooveListWidget->addItem(QString("%1: 6 6").arg(seqs_.size(), 2, 16, QChar('0')).toUpper());
	seqs_.push_back({ 6, 6});
	ui->removeButton->setEnabled(true);
	ui->grooveListWidget->setCurrentRow(ui->grooveListWidget->count() - 1);
	if (ui->grooveListWidget->count() == 128) ui->addButton->setEnabled(false);
}

void GrooveSettingsDialog::on_removeButton_clicked()
{
	int row = ui->grooveListWidget->currentRow();
	if (row == -1) return;

	seqs_.erase(seqs_.begin() + row);
	delete ui->grooveListWidget->takeItem(row);
	for (int i = row; i < ui->grooveListWidget->count(); ++i) {
		QString text = QString::number(i) + ": ";
		for (auto& g : seqs_[static_cast<size_t>(i)]) {
			text = text + QString::number(g) + " ";
		}
		ui->grooveListWidget->item(i)->setText(text);
	}

	on_grooveListWidget_currentRowChanged(ui->grooveListWidget->currentRow());

	ui->addButton->setEnabled(true);
	if (ui->grooveListWidget->count() == 1)
		ui->removeButton->setEnabled(false);
}

void GrooveSettingsDialog::on_lineEdit_editingFinished()
{
	int row = ui->grooveListWidget->currentRow();
	if (row == -1) return;

	std::vector<int> seq;
	QString text = ui->lineEdit->text();
	while (!text.isEmpty()) {
		QRegularExpressionMatch m = QRegularExpression("^(\\d+)").match(text);
		if (m.hasMatch()) {
			seq.push_back(m.captured(1).toInt());
			text.remove(QRegularExpression("^\\d+"));
			continue;
		}

		m = QRegularExpression("^ +").match(text);
		if (m.hasMatch()) {
			text.remove(QRegularExpression("^ +"));
			continue;
		}

		return;
	}

	if (seq.empty()) return;

	seqs_.at(static_cast<size_t>(row)) = std::move(seq);
	changeSequence(row);
}

void GrooveSettingsDialog::on_grooveListWidget_currentRowChanged(int currentRow)
{
	if (currentRow > -1) updateSequence(static_cast<size_t>(currentRow));
}

void GrooveSettingsDialog::changeSequence(int seqNum)
{
	QString text = updateSequence(static_cast<size_t>(seqNum));
	ui->grooveListWidget->item(seqNum)->setText(QString("%1: ").arg(seqNum, 2, 16, QChar('0')).toUpper() + text);
}

QString GrooveSettingsDialog::updateSequence(size_t seqNum)
{
	ui->seqListWidget->clear();
	QString text;
	auto& seq = seqs_.at(seqNum);
	for (size_t i = 0; i < seq.size(); ++i) {
		ui->seqListWidget->addItem(QString("%1: %2").arg(i, 2, 16, QChar('0')).toUpper().arg(seq[i]));
		text = text + QString::number(seq[i]) + " ";
	}
	ui->seqListWidget->setCurrentRow(0);
	ui->lineEdit->setText(text);
	double speed = std::accumulate(seq.begin(), seq.end(), 0) / static_cast<double>(seq.size());
	ui->speedLabel->setText(tr("Speed: %1").arg(QString::number(speed, 'f', 3)));
	return text;
}

void GrooveSettingsDialog::on_upToolButton_clicked()
{
	int curRow = ui->seqListWidget->currentRow();
	if (!curRow) return;

	swapSequenceItem(static_cast<size_t>(ui->grooveListWidget->currentRow()), curRow - 1, curRow);
	ui->seqListWidget->setCurrentRow(curRow - 1);
}

void GrooveSettingsDialog::on_downToolButton_clicked()
{
	int curRow = ui->seqListWidget->currentRow();
	if (curRow == ui->seqListWidget->count() - 1) return;

	swapSequenceItem(static_cast<size_t>(ui->grooveListWidget->currentRow()), curRow, curRow + 1);
	ui->seqListWidget->setCurrentRow(curRow + 1);
}

void GrooveSettingsDialog::swapSequenceItem(size_t seqNum, int index1, int index2)
{
	std::iter_swap(seqs_.at(seqNum).begin() + index1, seqs_.at(seqNum).begin() + index2);
	changeSequence(static_cast<int>(seqNum));
}

void GrooveSettingsDialog::on_expandPushButton_clicked()
{
	size_t id = static_cast<size_t>(ui->grooveListWidget->currentRow());
	auto& ref = seqs_[id];
	if (utils::find(ref, 1) != ref.end()) return;

	std::vector<int> seq;
	for (auto v : ref) {
		int tmp = v / 2;
		seq.push_back(v - tmp);
		seq.push_back(tmp);
	}
	seqs_.at(id) = std::move(seq);
	changeSequence(static_cast<int>(id));
}

void GrooveSettingsDialog::on_shrinkPushButton_clicked()
{
	size_t id = static_cast<size_t>(ui->grooveListWidget->currentRow());
	auto& ref = seqs_[id];
	if (ref.size() % 2) return;

	std::vector<int> seq;
	for (auto it = ref.begin(); it != ref.end(); it += 2) seq.push_back(*it + *(it + 1));
	seqs_.at(id) = std::move(seq);
	changeSequence(static_cast<int>(id));
}

void GrooveSettingsDialog::on_genPushButton_clicked()
{
	int num = ui->numeratorSpinBox->value();
	int denom = ui->denominatorSpinBox->value();
	if (num < denom) return;

	std::vector<int> seq(static_cast<size_t>(denom));
	for (int i = 0; i < num * denom; i += num)
		seq.at(static_cast<size_t>(denom - i / num - 1)) = (i + num) / denom - i / denom;
	seqs_.at(static_cast<size_t>(ui->grooveListWidget->currentRow())) = std::move(seq);
	changeSequence(ui->grooveListWidget->currentRow());
}

void GrooveSettingsDialog::on_padPushButton_clicked()
{
	int pad = ui->padSpinBox->value();
	size_t id = static_cast<size_t>(ui->grooveListWidget->currentRow());
	auto& ref = seqs_[id];
	if (std::any_of(ref.begin(), ref.end(), [pad](int x) {return (x <= pad); })) return;

	std::vector<int> seq;
	for (auto v : ref) {
		seq.push_back(v - pad);
		seq.push_back(pad);
	}
	seqs_.at(id) = std::move(seq);
	changeSequence(static_cast<int>(id));
}

void GrooveSettingsDialog::on_copyPushButton_clicked()
{
	auto& seq = seqs_[static_cast<size_t>(ui->grooveListWidget->currentRow())];
	auto text = QString("PATTERN_COPY:3,2,%1,").arg(seq.size());
	for (auto v : seq) text += QString("0F,%1,").arg(v);

	QApplication::clipboard()->setText(text);
}
