#include "labeled_slider.hpp"
#include "ui_labeled_slider.h"

LabeledSlider::LabeledSlider(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::LabeledSlider)
{
	init("");
}

LabeledSlider::LabeledSlider(QString text, QWidget *parent) :
	QFrame(parent),
	ui(new Ui::LabeledSlider)
{
	init(text);
}

void LabeledSlider::init(QString text)
{
	ui->setupUi(this);
	ui->textLabel->setText(text);
	ui->valueLabel->setText(QString::number(ui->slider->value()));
}

LabeledSlider::~LabeledSlider()
{
	delete ui;
}

int LabeledSlider::value() const
{
	return ui->slider->value();
}

void LabeledSlider::setValue(int value)
{
	ui->slider->setValue(value);
}

int LabeledSlider::maximum() const
{
	return ui->slider->maximum();
}

void LabeledSlider::setMaximum(int value)
{
	ui->slider->setMaximum(value);
}

int LabeledSlider::minimum() const
{
	return ui->slider->minimum();
}

void LabeledSlider::setMinimum(int value)
{
	ui->slider->setMinimum(value);
}

QString LabeledSlider::text() const
{
	return ui->textLabel->text();
}

void LabeledSlider::setText(QString text)
{
	ui->textLabel->setText(text);
}

void LabeledSlider::on_slider_valueChanged(int value)
{
	ui->valueLabel->setText(QString::number(value));

	emit valueChanged(value);
}
