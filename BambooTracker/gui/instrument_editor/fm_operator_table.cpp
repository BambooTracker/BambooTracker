#include "fm_operator_table.hpp"
#include "ui_fm_operator_table.h"
#include <QString>
#include <QGraphicsScene>
#include <QPen>
#include <QBrush>

FMOperatorTable::FMOperatorTable(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::FMOperatorTable)
{
	ui->setupUi(this);
	ui->ssgegSlider->setEnabled(false);

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
	sliderMap_.emplace(Ui::FMOperatorParameter::TYPE_SSGEG, ui->ssgegSlider);

	QString name[] = { "AR", "DR", "SR", "RR", "SL", "TL", "KS", "ML", "DT", "TYPE" };
	int maxValue[] = { 31, 31, 31, 15, 15, 127, 3, 15, 7, 7 };

	int n = 0;
	for (auto& pair : sliderMap_) {
		pair.second->setText(name[n]);
		pair.second->setMaximum(maxValue[n]);
		QObject::connect(pair.second, &LabeledVerticalSlider::valueChanged,
						 this, [&](int value) {
			repaintGraph();
			emit operatorValueChanged(pair.first, value);
		});
		++n;
	}

	auto scene = new QGraphicsScene(0, 0,201, 128, ui->envelopeGraphicsView);
	ui->envelopeGraphicsView->setScene(scene);
}

FMOperatorTable::~FMOperatorTable()
{
	delete ui;
}

void FMOperatorTable::setOperatorNumber(int n)
{
	number_ = n;
	ui->groupBox->setTitle("Operator " + QString::number(n + 1));
}

int FMOperatorTable::operatorNumber() const
{
	return number_;
}

void FMOperatorTable::setValue(Ui::FMOperatorParameter param, int value)
{
	sliderMap_.at(param)->setValue(value);
}

void FMOperatorTable::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)
	resizeGraph();
	repaintGraph();
}

void FMOperatorTable::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)
	resizeGraph();
}

void FMOperatorTable::resizeGraph()
{
	ui->envelopeGraphicsView->fitInView(ui->envelopeGraphicsView->scene()->sceneRect());
}

void FMOperatorTable::repaintGraph()
{
	auto scene = ui->envelopeGraphicsView->scene();
	double envHeight = ui->ssgegCheckBox->isChecked()? (scene->height() - 40) : scene->height();
	int linew = 2;
	QBrush lineColor = Qt::red;
	double startx = 0;

	double tly, tlx;
	if (ui->arSlider->value()) {
		tly = (127 - ui->tlSlider->value()) / 127.0 * envHeight;
	}
	else {
		tly = 0;
	}
	tlx = 50 * (31 - ui->arSlider->value()) / 31 * tly / envHeight;

	double sly, slx;
	if (ui->drSlider->value()) {
		sly = (15 - ui->slSlider->value()) / 15.0 * tly;
		slx = 100 / envHeight * (31 - ui->drSlider->value()) / 31 * (tly - sly);
	}
	else {
		sly = tly;
		if (ui->slSlider->value()) {
			slx = 100 * sly / envHeight;
		}
		else {
			slx = 0;
		}
	}
	slx += tlx;
	tly = envHeight - tly;

	double rry, rrx;
	if (!ui->drSlider->value() && ui->slSlider->value()) {
		rry = sly;
		rrx = 0;
	}
	else {
		if (ui->srSlider->value()) {
			rry = 0.5 * sly;
			rrx = 100 / envHeight * (31 - ui->srSlider->value()) / 31 * (sly - rry);
		}
		else {
			rry = sly;
			rrx = 100;
		}
	}
	rrx += slx;
	sly = envHeight - sly;

	double endy, endx;
	if (ui->rrSlider->value()) {
		endy = 0;
		endx = (100 * rry / envHeight) * (15 - ui->rrSlider->value()) / 15 + rrx;
	}
	else {
		endy = rry;
		endx = 200;
	}
	rry = envHeight - rry;
	endy = envHeight - endy;

	scene->clear();

	scene->addLine(startx, envHeight, tlx, tly, QPen(lineColor, linew));
	scene->addLine(tlx, tly, slx, sly, QPen(lineColor, linew));
	scene->addLine(slx, sly, rrx, rry, QPen(lineColor, linew));
	scene->addLine(rrx, rry, endx, endy, QPen(lineColor, linew));

	scene->addEllipse(tlx-1, tly, 4, 4, QPen(), Qt::white);
	scene->addEllipse(slx-1, sly, 4, 4, QPen(), Qt::white);
	scene->addEllipse(rrx-1, rry, 4, 4, QPen(), Qt::white);

	if (ui->ssgegCheckBox->isChecked()) {
		double seph = scene->height() - 39;
		scene->addLine(0, seph, 200, seph, QPen(Qt::gray));
		double toph = seph + 2;
		double both = scene->height();
		lineColor = Qt::blue;
		switch (ui->ssgegSlider->value()) {
		case 0:
		{
			for (int i = 0; i < 5; ++i) {
				scene->addLine(40 * i, both, 40 * i, toph, QPen(lineColor, linew));
				scene->addLine(40 * i, toph, 40 * (i + 1), both, QPen(lineColor, linew));
			}
		}
			break;
		case 1:
		{
			scene->addLine(0, both, 0, toph, QPen(lineColor, linew));
			scene->addLine(0, toph, 40, both, QPen(lineColor, linew));
			scene->addLine(40, both, 200, both, QPen(lineColor, linew));
		}
			break;
		case 2:
		{
			scene->addLine(0, both, 0, toph, QPen(lineColor, linew));
			scene->addLine(0, toph, 40, both, QPen(lineColor, linew));
			for (int i = 0; i < 2; ++i) {
				scene->addLine(40 + 80 * i, both, 80 + 80 * i, toph, QPen(lineColor, linew));
				scene->addLine(80 + 80 * i, toph, 40 + 80 * (i + 1), both, QPen(lineColor, linew));
			}
		}
			break;
		case 3:
		{
			scene->addLine(0, both, 0, toph, QPen(lineColor, linew));
			scene->addLine(0, toph, 40, both, QPen(lineColor, linew));
			scene->addLine(40, both, 40, toph, QPen(lineColor, linew));
			scene->addLine(40, toph, 200, toph, QPen(lineColor, linew));
		}
			break;
		case 4:
		{
			for (int i = 0; i < 5; ++i) {
				scene->addLine(40 * i, both, 40 * (i + 1), toph, QPen(lineColor, linew));
				scene->addLine(40 * (i + 1), toph, 40 * (i + 1), both, QPen(lineColor, linew));
			}
		}
			break;
		case 5:
		{
			scene->addLine(0, both, 40, toph, QPen(lineColor, linew));
			scene->addLine(40, toph, 200, toph, QPen(lineColor, linew));
		}
			break;
		case 6:
		{
			for (int i = 0; i < 2; ++i) {
				scene->addLine(80 * i, both, 40 + 80 * i, toph, QPen(lineColor, linew));
				scene->addLine(40 + 80 * i, toph, 80 * (i + 1), both, QPen(lineColor, linew));
			}
			scene->addLine(160, both, 200, toph, QPen(lineColor, linew));
		}
			break;
		case 7:
		{
			scene->addLine(0, both, 40, toph, QPen(lineColor, linew));
			scene->addLine(40, toph, 40, both, QPen(lineColor, linew));
			scene->addLine(40, both, 200, both, QPen(lineColor, linew));
		}
			break;
		}
	}
}

void FMOperatorTable::on_ssgegCheckBox_stateChanged(int arg1)
{
	Q_UNUSED(arg1)
	if (ui->ssgegCheckBox->isChecked()) {
		ui->ssgegSlider->setEnabled(true);
		ui->arSlider->setValue(31);
		ui->arSlider->setEnabled(false);
	}
	else {
		ui->ssgegSlider->setEnabled(false);
		ui->arSlider->setEnabled(true);
	}
	repaintGraph();
}
