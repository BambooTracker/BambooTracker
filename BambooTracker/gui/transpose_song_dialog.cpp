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

#include "transpose_song_dialog.hpp"
#include "ui_transpose_song_dialog.h"
#include <QString>

TransposeSongDialog::TransposeSongDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TransposeSongDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	for (int i = 0; i < 128; ++i) {
		auto item = new QListWidgetItem(QString("%1").arg(i, 2, 16, QChar('0')).toUpper());
		checks[i] = item;
		item->setCheckState(Qt::Unchecked);
		QObject::connect(ui->reversePushButton, &QPushButton::clicked,
						 this, [item] {
			item->setCheckState((item->checkState() == Qt::Unchecked) ? Qt::Checked : Qt::Unchecked);
		});
		QObject::connect(ui->clearPushButton, &QPushButton::clicked,
						 this, [item] { item->setCheckState(Qt::Unchecked); });
		ui->listWidget->addItem(item);
	}
}

TransposeSongDialog::~TransposeSongDialog()
{
	delete ui;
}

int TransposeSongDialog::getTransposeSeminotes() const
{
	return ui->spinBox->value();
}

std::vector<int> TransposeSongDialog::getExcludeInstruments() const
{
	std::vector<int> list;
	for (int i = 0; i < 128; ++i) {
		if (checks[i]->checkState() == Qt::Checked)
			list.push_back(i);
	}
	return list;
}
