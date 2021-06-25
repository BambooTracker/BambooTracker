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

#include "go_to_dialog.hpp"
#include "ui_go_to_dialog.h"
#include <QString>
#include "gui/gui_utils.hpp"

GoToDialog::GoToDialog(std::weak_ptr<BambooTracker> bt, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GoToDialog),
	bt_(bt),
	song_(bt.lock()->getCurrentSongNumber())
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);


	ui->orderSpinBox->setMaximum(bt.lock()->getOrderSize(song_) - 1);
	ui->orderSpinBox->setValue(bt.lock()->getCurrentOrderNumber());
	ui->stepSpinBox->setMaximum(bt.lock()->getPatternSizeFromOrderNumber(song_, ui->orderSpinBox->value()));
	ui->stepSpinBox->setValue(bt.lock()->getCurrentStepNumber());
	auto style = bt.lock()->getSongStyle(bt.lock()->getCurrentSongNumber());
	for (auto& attrib : style.trackAttribs) {
		ui->trackComboBox->addItem(
					gui_utils::getTrackName(style.type, attrib.source, attrib.channelInSource), attrib.number);
		if (bt.lock()->getCurrentTrackAttribute().number == attrib.number)
			ui->trackComboBox->setCurrentIndex(ui->trackComboBox->count() - 1);
	}
}

GoToDialog::~GoToDialog()
{
	delete ui;
}

int GoToDialog::getOrder() const
{
	return ui->orderSpinBox->value();
}

int GoToDialog::getStep() const
{
	return ui->stepSpinBox->value();
}

int GoToDialog::getTrack() const
{
	return ui->trackComboBox->currentData().toInt();
}

void GoToDialog::on_orderSpinBox_valueChanged(int arg1)
{
	ui->stepSpinBox->setMaximum(bt_.lock()->getPatternSizeFromOrderNumber(song_, arg1));
}
