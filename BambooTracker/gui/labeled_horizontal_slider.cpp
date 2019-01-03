#include "labeled_horizontal_slider.hpp"
#include "ui_labeled_horizontal_slider.h"
#include <QWheelEvent>
#include "slider_style.hpp"

LabeledHorizontalSlider::LabeledHorizontalSlider(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::LabeledHorizontalSlider)
{
	init("", "", "");
}

LabeledHorizontalSlider::LabeledHorizontalSlider(QString text, QString prefix, QString suffix, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::LabeledHorizontalSlider)
{
	init(text, prefix, suffix);
}

void LabeledHorizontalSlider::init(QString text, QString prefix, QString suffix)
{
	ui->setupUi(this);

	rate_ = 1.0;
	precision_ = 0;
	isSigned_ = false;
	ui->textLabel->setText(text);
	prefix_ = prefix;
	suffix_ = suffix;
	updateValueLabel();
	ui->slider->setStyle(new SliderStyle());
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
