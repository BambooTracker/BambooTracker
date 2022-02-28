/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "labeled_horizontal_slider.hpp"
#include "ui_labeled_horizontal_slider.h"
#include <QWheelEvent>
#include "slider_style.hpp"

LabeledHorizontalSlider::LabeledHorizontalSlider(QWidget *parent) : LabeledHorizontalSlider("", "", "", parent) {}

LabeledHorizontalSlider::LabeledHorizontalSlider(QString text, QString prefix, QString suffix, QWidget *parent)
	: QFrame(parent),
	  ui(new Ui::LabeledHorizontalSlider)
{
	ui->setupUi(this);

	rate_ = 1.0;
	precision_ = 0;
	isSigned_ = false;
	ui->textLabel->setText(text);
	prefix_ = prefix;
	suffix_ = suffix;
	updateValueLabel();
	ui->slider->setStyle(SliderStyle::instance());
	ui->slider->installEventFilter(this);
}

LabeledHorizontalSlider::~LabeledHorizontalSlider()
{
	delete ui;
}

int LabeledHorizontalSlider::value() const
{
	return ui->slider->value();
}

void LabeledHorizontalSlider::setValue(int value)
{
	ui->slider->setValue(value);
}

int LabeledHorizontalSlider::maximum() const
{
	return ui->slider->maximum();
}

void LabeledHorizontalSlider::setMaximum(int value)
{
	ui->slider->setMaximum(value);
}

int LabeledHorizontalSlider::minimum() const
{
	return ui->slider->minimum();
}

void LabeledHorizontalSlider::setMinimum(int value)
{
	ui->slider->setMinimum(value);
}

void LabeledHorizontalSlider::setValueRate(double rate, int precision)
{
	rate_ = rate;
	precision_ = precision;
	updateValueLabel();
}

void LabeledHorizontalSlider::setSign(bool enabled)
{
	isSigned_ = enabled;
	updateValueLabel();
}

void LabeledHorizontalSlider::setTickPosition(QSlider::TickPosition position)
{
	ui->slider->setTickPosition(position);
}

void LabeledHorizontalSlider::setTickInterval(int ti)
{
	ui->slider->setTickInterval(ti);
}

QString LabeledHorizontalSlider::text() const
{
	return ui->textLabel->text();
}

void LabeledHorizontalSlider::setText(QString text)
{
	ui->textLabel->setText(text);
}

QString LabeledHorizontalSlider::suffix() const
{
	return suffix_;
}

void LabeledHorizontalSlider::setSuffix(QString suffix)
{
	suffix_ = suffix;
	updateValueLabel();
}

QString LabeledHorizontalSlider::prefix() const
{
	return prefix_;
}

void LabeledHorizontalSlider::setprefix(QString prefix)
{
	prefix_ = prefix;
	updateValueLabel();
}

bool LabeledHorizontalSlider::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui->slider) {
		if (event->type() == QEvent::Wheel) {
			auto e = dynamic_cast<QWheelEvent*>(event);
			if (e->angleDelta().y() > 0) ui->slider->setValue(ui->slider->value() + 1);
			else if (e->angleDelta().y() < 0) ui->slider->setValue(ui->slider->value() - 1);
			return true;
		}
	}

	return QFrame::eventFilter(watched, event);
}

void LabeledHorizontalSlider::on_slider_valueChanged(int value)
{
	updateValueLabel();
	emit valueChanged(value);
}

void LabeledHorizontalSlider::updateValueLabel()
{
	QString sign = (isSigned_ && ui->slider->value() > -1) ? "+" : "";
	ui->valueLabel->setText(
				prefix_ + sign + QString::number(ui->slider->value() * rate_, 'f', precision_) + suffix_);
}
