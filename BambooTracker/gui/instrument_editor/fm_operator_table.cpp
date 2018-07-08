#include "fm_operator_table.hpp"
#include "ui_fm_operator_table.h"
#include <QString>
#include <QLineSeries>
#include <QValueAxis>
#include <QColor>

FMOperatorTable::FMOperatorTable(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::FMOperatorTable)
{
	ui->setupUi(this);

	// Init sliders
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

	// Init ADSR graph
	auto&& chart = ui->adsrChartView->chart();
	chart->legend()->hide();
	chart->layout()->setContentsMargins(0, 0, 0, 0);
	chart->setBackgroundBrush(Qt::transparent);

	rewriteGraph();
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

void FMOperatorTable::rewriteGraph()
{
	auto&& chart = ui->adsrChartView->chart();

	chart->removeAllSeries();

	auto* series = new QtCharts::QLineSeries();
	series->append(0,0);
	series->append(1,1);
	series->append(5,0);
	chart->addSeries(series);

	auto* xAxis = new QtCharts::QValueAxis();
	xAxis->setRange(0,100);
	chart->setAxisX(xAxis, series);
	auto* yAxis = new QtCharts::QValueAxis();
	yAxis->setRange(0,127);
	chart->setAxisY(yAxis, series);
}
