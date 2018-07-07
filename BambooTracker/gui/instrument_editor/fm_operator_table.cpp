#include "fm_operator_table.hpp"
#include "ui_fm_operator_table.h"
#include <QString>

FMOperatorTable::FMOperatorTable(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::FMOperatorTable)
{
	ui->setupUi(this);

	sliderMap_.emplace(Ui::FMOperatorParameter::AR, ui->arSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::DR, ui->drSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::SR, ui->srSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::RR, ui->rrSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::SL, ui->slSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::TL, ui->tlSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::KS, ui->ksSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::ML, ui->mlSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::DT, ui->dtSlider);
	sliderMap_.emplace(Ui::FMOperatorParameter::AM, ui->amSlider);

	QString name[] = { "AR", "DR", "SR", "RR", "SL", "TL", "KS", "ML", "DT", "AM" };
	int maxValue[] = { 31, 31, 31, 15, 15, 127, 3, 15, 7, 1 };

	int n = 0;
	for (auto& pair : sliderMap_) {
		pair.second->setText(name[n]);
		pair.second->setMaximum(maxValue[n]);
		QObject::connect(pair.second, &LabeledSlider::valueChanged,
						 this, [&](int value) { emit operatorValueChanged(pair.first, value); });
		++n;
	}
}

FMOperatorTable::~FMOperatorTable()
{
	delete ui;
}

void FMOperatorTable::setOperatorNumber(int n)
{
	number_ = n;
}

int FMOperatorTable::operatorNumber() const
{
	return number_;
}

void FMOperatorTable::setValue(Ui::FMOperatorParameter param, int value)
{
	sliderMap_.at(param)->setValue(value);
}
