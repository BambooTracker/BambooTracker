#include "groove_settings_dialog.hpp"
#include "ui_groove_settings_dialog.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>

GrooveSettingsDialog::GrooveSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GrooveSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

GrooveSettingsDialog::~GrooveSettingsDialog()
{
	delete ui;
}

void GrooveSettingsDialog::setGrooveSquences(std::vector<std::vector<int>> seqs)
{
	seqs_ = seqs;

	for (size_t i = 0; i < seqs_.size(); ++i) {
		QString text = QString::number(i) + ": ";
		for (auto& g : seqs_[i]) {
			text = text + QString::number(g) + " ";
		}
		ui->listWidget->addItem(text);
	}

	ui->removeButton->setEnabled(ui->listWidget->count() > 1);
}

std::vector<std::vector<int>> GrooveSettingsDialog::getGrooveSequences()
{
	return seqs_;
}

void GrooveSettingsDialog::on_addButton_clicked()
{
	ui->listWidget->addItem(QString::number(seqs_.size()) + ": 6 6 ");
	seqs_.push_back({ 6, 6});
	ui->removeButton->setEnabled(true);
}
#include <QDebug>
void GrooveSettingsDialog::on_removeButton_clicked()
{
	int row = ui->listWidget->currentRow();
	if (row == -1) return;

	seqs_.erase(seqs_.begin() + row);
	delete ui->listWidget->takeItem(row);
	for (int i = row; i < ui->listWidget->count(); ++i) {
		QString text = QString::number(i) + ": ";
		for (auto& g : seqs_[i]) {
			text = text + QString::number(g) + " ";
		}
		ui->listWidget->item(i)->setText(text);
	}

	on_listWidget_currentRowChanged(ui->listWidget->currentRow());

	if (ui->listWidget->count() == 1)
		ui->removeButton->setEnabled(false);
}

void GrooveSettingsDialog::on_lineEdit_editingFinished()
{
	int row = ui->listWidget->currentRow();
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

	text = "";
	for (auto& g : seq) {
		text = text + QString::number(g) + " ";
	}
	ui->lineEdit->setText(text);
	ui->listWidget->item(row)->setText(QString::number(row) + ": " + text);
	seqs_.at(row) = std::move(seq);
}

void GrooveSettingsDialog::on_listWidget_currentRowChanged(int currentRow)
{
	if (currentRow > -1) {
		QString text;
		for (auto& g : seqs_[currentRow]) {
			text = text + QString::number(g) + " ";
		}
		ui->lineEdit->setText(text);
	}
}
