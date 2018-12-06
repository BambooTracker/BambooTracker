#include "labeled_vertical_slider.hpp"
#include "ui_labeled_vertical_slider.h"
#include <QWheelEvent>
#include "slider_style.hpp"

LabeledVerticalSlider::LabeledVerticalSlider(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::LabeledVerticalSlider)
{
	init("");
}

LabeledVerticalSlider::LabeledVerticalSlider(QString text, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::LabeledVerticalSlider)
{
	init(text);
}

void LabeledVerticalSlider::init(QString text)
{
	ui->setupUi(this);
	ui->textLabel->setText(text);
	ui->valueLabel->setText(QString::number(ui->slider->value()));
	ui->slider->setStyle(new SliderStyle());
	ui->slider->installEventFilter(this);
}

LabeledVerticalSlider::~LabeledVerticalSlider()
{
	delete ui;
}

int LabeledVerticalSlider::value() const
{
	return ui->slider->value();
}

void LabeledVerticalSlider::setValue(int value)
{
	ui->slider->setValue(value);
}

int LabeledVerticalSlider::maximum() const
{
	return ui->slider->maximum();
}

void LabeledVerticalSlider::setMaximum(int value)
{
	ui->slider->setMaximum(value);
}

int LabeledVerticalSlider::minimum() const
{
	return ui->slider->minimum();
}

void LabeledVerticalSlider::setMinimum(int value)
{
	ui->slider->setMinimum(value);
}

QString LabeledVerticalSlider::text() const
{
	return ui->textLabel->text();
}

void LabeledVerticalSlider::setText(QString text)
{
	ui->textLabel->setText(text);
}

bool LabeledVerticalSlider::eventFilter(QObject* watched, QEvent* event)
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

void LabeledVerticalSlider::on_slider_valueChanged(int value)
{
	ui->valueLabel->setText(QString::number(value));
	emit valueChanged(value);
}
