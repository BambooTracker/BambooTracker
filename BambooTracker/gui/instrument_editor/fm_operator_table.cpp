/*
 * Copyright (C) 2018-2020 Rerrah
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

#include "fm_operator_table.hpp"
#include "ui_fm_operator_table.h"
#include <QString>
#include <QPen>
#include <QMenu>
#include <QClipboard>
#include "gui/event_guard.hpp"

const int FMOperatorTable::DT_SIGN_TBL_[8] = { 0, 1, 2, 3, 0, -1, -2, -3 };
const int FMOperatorTable::DT_UNSIGN_TBL_[7] = { 7, 6, 5, 0, 1, 2, 3 };

FMOperatorTable::FMOperatorTable(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::FMOperatorTable),
	isDTNegative_(false),
	isIgnoreEvent_(false)
{
	ui->setupUi(this);

	ui->groupBox->setContextMenuPolicy(Qt::CustomContextMenu);

	// Init sliders
	ui->arSlider->setText("AR");
	ui->arSlider->setMaximum(31);
	ui->drSlider->setText("DR");
	ui->drSlider->setMaximum(31);
	ui->srSlider->setText("SR");
	ui->srSlider->setMaximum(31);
	ui->rrSlider->setText("RR");
	ui->rrSlider->setMaximum(15);
	ui->slSlider->setText("SL");
	ui->slSlider->setMaximum(15);
	ui->tlSlider->setText("TL");
	ui->tlSlider->setMaximum(127);
	ui->ksSlider->setText("KS");
	ui->ksSlider->setMaximum(3);
	ui->mlSlider->setText("ML");
	ui->mlSlider->setMaximum(15);
	ui->dtSlider->setText("DT");
	ui->dtSlider->setMaximum(7);

	sliderMap_ = {
		{ Ui::FMOperatorParameter::AR, ui->arSlider },
		{ Ui::FMOperatorParameter::DR, ui->drSlider },
		{ Ui::FMOperatorParameter::SR, ui->srSlider },
		{ Ui::FMOperatorParameter::RR, ui->rrSlider },
		{ Ui::FMOperatorParameter::SL, ui->slSlider },
		{ Ui::FMOperatorParameter::TL, ui->tlSlider },
		{ Ui::FMOperatorParameter::KS, ui->ksSlider },
		{ Ui::FMOperatorParameter::ML, ui->mlSlider },
		{ Ui::FMOperatorParameter::DT, ui->dtSlider }
	};

	for (auto& pair : sliderMap_) {
		if (pair.second == ui->dtSlider) {
			QObject::connect(pair.second, &LabeledVerticalSlider::valueChanged,
							 this, [&](int value) {
				repaintGraph();
				if (!isIgnoreEvent_) {
					if (isDTNegative_) value = DT_UNSIGN_TBL_[value + 3];
					emit operatorValueChanged(pair.first, value);
				}
			});
		}
		else {
			QObject::connect(pair.second, &LabeledVerticalSlider::valueChanged,
							 this, [&](int value) {
				repaintGraph();
				if (!isIgnoreEvent_) emit operatorValueChanged(pair.first, value);
			});
		}
	}

	ui->ssgegSlider->setEnabled(false);
	ui->ssgegSlider->setText(tr("Type"));
	ui->ssgegSlider->setMaximum(7);
	QObject::connect(ui->ssgegSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int value) {
		repaintGraph();
		if (!isIgnoreEvent_) emit operatorValueChanged(Ui::FMOperatorParameter::SSGEG, value);
	});

	ui->envFrame->installEventFilter(this);
}

FMOperatorTable::~FMOperatorTable()
{
	delete ui;
}

void FMOperatorTable::setEnvelopeSetNames(std::vector<QString> list)
{
	envelopeTypes_ = list;
}

void FMOperatorTable::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
}

void FMOperatorTable::setOperatorNumber(int n)
{
	number_ = n;
	ui->groupBox->setTitle(tr("Operator %1").arg(n + 1));
}

int FMOperatorTable::operatorNumber() const
{
	return number_;
}

void FMOperatorTable::setValue(Ui::FMOperatorParameter param, int value)
{
	if (param == Ui::FMOperatorParameter::SSGEG) {
		if (value == -1) {
			ui->ssgegCheckBox->setChecked(false);
		}
		else {
			ui->ssgegCheckBox->setChecked(true);
			ui->ssgegSlider->setValue(value);
		}
	}
	else {
		if (param == Ui::FMOperatorParameter::DT && isDTNegative_) value = DT_SIGN_TBL_[value];
		sliderMap_.at(param)->setValue(value);
	}
}

void FMOperatorTable::setGroupEnabled(bool enabled)
{
	Ui::EventGuard eg(isIgnoreEvent_);

	ui->groupBox->setChecked(enabled);
}

void FMOperatorTable::setDTDisplayType(bool useNegative)
{
	if (isDTNegative_ == useNegative) return;

	Ui::EventGuard eg(isIgnoreEvent_);
	isDTNegative_ = useNegative;
	if (useNegative) {
		int dt = DT_SIGN_TBL_[ui->dtSlider->value()];
		ui->dtSlider->setMinimum(-3);
		ui->dtSlider->setMaximum(3);
		ui->dtSlider->setSign(true);
		ui->dtSlider->setValue(dt);
	}
	else {
		int dt = DT_UNSIGN_TBL_[ui->dtSlider->value() + 3];
		ui->dtSlider->setMinimum(0);
		ui->dtSlider->setMaximum(7);
		ui->dtSlider->setSign(false);
		ui->dtSlider->setValue(dt);
	}
}

QString FMOperatorTable::toString() const
{
	int dt = ui->dtSlider->value();
	if (isDTNegative_) dt = DT_UNSIGN_TBL_[dt + 3];

	auto str = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10")
			   .arg(ui->arSlider->value())
			   .arg(ui->drSlider->value())
			   .arg(ui->srSlider->value())
			   .arg(ui->rrSlider->value())
			   .arg(ui->slSlider->value())
			   .arg(ui->tlSlider->value())
			   .arg(ui->ksSlider->value())
			   .arg(ui->mlSlider->value())
			   .arg(dt)
			   .arg(ui->ssgegCheckBox->isChecked() ? ui->ssgegSlider->value() : -1);
	return str;
}

bool FMOperatorTable::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == ui->envFrame) {
		if (event->type() == QEvent::Paint) {
			QPainter painter(ui->envFrame);
			painter.eraseRect(ui->envFrame->rect());
			painter.drawPixmap(ui->envFrame->rect(), envmap_, envmap_.rect());
		}
	}

	return false;
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
	repaintGraph();
}

void FMOperatorTable::resizeGraph()
{
	envmap_ = QPixmap(ui->envFrame->size());
	xr_ = (envmap_.width() - (ENV_LINE_W_ + 1) * 2.) / ENV_W_;
	yr_ = (envmap_.height() - (ENV_LINE_W_ + 1) * 2.) / ENV_H_;
}

void FMOperatorTable::repaintGraph()
{
	if (!palette_) return;

	QPointF p0, p1, p2, p3, p4;
	const double marginHorizon = 100;
	const double marginAr = 50;
	const double marginDr = 150;
	const double marginSr = 400;

	if (ui->arSlider->value() && ui->tlSlider->value() < 127) {
		p1.setY(127 - ui->tlSlider->value());
		if (ui->arSlider->value() < 31) {
			double ar = 127. / (marginAr * (31 - ui->arSlider->value()) / 30.);
			p1.setX(p1.y() / ar);
		}

		if (ui->drSlider->value()) {
			p2.setY(p1.y() * (1. - ui->slSlider->value() / 15.));
			if (ui->drSlider->value() == 31) {
				p2.setX(p1.x());
				if (ui->slSlider->value() == 15) {
					p3 = p2;
				}
				else {
					if (ui->srSlider->value()) {
						p3.setY(p2.y() / 2.);
						if (ui->srSlider->value() == 31) {
							p3.setX(p2.x());
						}
						else {
							double sr = 127. / (marginSr * (31 - ui->srSlider->value()) / 30.);
							p3.setX(p2.x() + p3.y() / 2. / sr);
						}
					}
					else {
						p3 = { p2.x() + ((ui->slSlider->value() == 15) ? 0 : marginHorizon), p2.y() };
					}
				}
			}
			else {
				double dr = 127. / (marginDr * (31 - ui->drSlider->value()) / 30.);
				p2.setX(p1.x() + (p1.y() - p2.y()) / dr);
				if (ui->srSlider->value()) {
					p3.setY(p2.y() / 2.);
					if (ui->srSlider->value() == 31) {
						p3.setX(p2.x());
					}
					else {
						double sr = 127. / (marginSr * (31 - ui->srSlider->value()) / 30.);
						p3.setX(p2.x() + p3.y() / 2. / sr);
					}
				}
				else {
					p3 = { p2.x() + ((ui->slSlider->value() == 15) ? 0 : marginHorizon), p2.y() };
				}
			}
		}
		else {
			if (ui->slSlider->value()) {
				p2 = { p1.x() + marginHorizon, p1.y() };
				p3 = p2;
			}
			else {
				p2 = p1;
				if (ui->srSlider->value()) {
					p3.setY(p2.y() / 2.);
					if (ui->srSlider->value() == 31) {
						p3.setX(p2.x());
					}
					else {
						double sr = 127. / (marginSr * (31 - ui->srSlider->value()) / 30.);
						p3.setX(p2.x() + p3.y() / 2. / sr);
					}
				}
				else {
					p3 = { p2.x() + marginHorizon, p2.y() };
				}
			}
		}

		if (!ui->rrSlider->value()) {
			p4 = { ENV_W_, p3.y() };
		}
		else if (ui->rrSlider->value() == 31) {
			p4.setX(p3.x());
		}
		else {
			double rr = 127. / (marginAr * (15 - ui->rrSlider->value()) / 14.);
			p4.setX(p3.x() + p3.y() / rr);
		}
	}

	double envHeight = ui->ssgegCheckBox->isChecked() ? (ENV_H_ - SSGEG_H_) : ENV_H_;
	double envHrate = envHeight / 127.;
	p0.setY((127. - p0.y()) * envHrate);
	p1.setY((127. - p1.y()) * envHrate);
	p2.setY((127. - p2.y()) * envHrate);
	p3.setY((127. - p3.y()) * envHrate);
	p4.setY((127. - p4.y()) * envHrate);

	envmap_.fill(palette_->instFMEnvBackColor);
	QPainter painter(&envmap_);

	painter.setPen(QPen(palette_->instFMEnvGridColor, ENV_LINE_T_));
	drawLine(painter, p1.x(), 0, p1.x(), envHeight);
	drawLine(painter, p2.x(), 0, p2.x(), envHeight);
	drawLine(painter, p3.x(), 0, p3.x(), envHeight);

	painter.setPen(QPen(palette_->instFMEnvLine1Color, ENV_LINE_W_));
	drawLines(painter, { p0, p1, p2, p3 });
	painter.setPen(QPen(palette_->instFMEnvLine2Color, ENV_LINE_W_));
	drawLine(painter, p3, p4);

	if (ui->ssgegCheckBox->isChecked()) {
		const double seph = envHeight + 1;
		painter.setPen(QPen(palette_->instFMEnvBorderColor, ENV_LINE_T_));
		drawLine(painter, 0, seph, ENV_W_, seph);

		const double toph = seph + 2;
		const double both = ENV_H_;
		const double horsec = ENV_W_ / 5.;
		const double dhorsec = horsec * 2;
		painter.setPen(QPen(palette_->instFMEnvLine3Color, ENV_LINE_W_));
		switch (ui->ssgegSlider->value()) {
		case 0:
		{
			for (int i = 0; i < 5; ++i) {
				drawLine(painter, horsec * i, both, horsec * i, toph);
				drawLine(painter, horsec * i, toph, horsec * (i + 1), both);
			}
		}
			break;
		case 1:
		{
			drawLine(painter, 0, both, 0, toph);
			drawLine(painter, 0, toph, horsec, both);
			drawLine(painter, horsec, both, ENV_W_, both);
		}
			break;
		case 2:
		{
			drawLine(painter, 0, both, 0, toph);
			drawLine(painter, 0, toph, horsec, both);
			for (int i = 0; i < 2; ++i) {
				drawLine(painter, horsec + dhorsec * i, both, dhorsec + dhorsec * i, toph);
				drawLine(painter, dhorsec + dhorsec * i, toph, horsec + dhorsec * (i + 1), both);
			}
		}
			break;
		case 3:
		{
			drawLine(painter, 0, both, 0, toph);
			drawLine(painter, 0, toph, horsec, both);
			drawLine(painter, horsec, both, horsec, toph);
			drawLine(painter, horsec, toph, ENV_W_, toph);
		}
			break;
		case 4:
		{
			for (int i = 0; i < 5; ++i) {
				drawLine(painter, horsec * i, both, horsec * (i + 1), toph);
				drawLine(painter, horsec * (i + 1), toph, horsec * (i + 1), both);
			}
		}
			break;
		case 5:
		{
			drawLine(painter, 0, both, horsec, toph);
			drawLine(painter, horsec, toph, ENV_W_, toph);
		}
			break;
		case 6:
		{
			for (int i = 0; i < 2; ++i) {
				drawLine(painter, dhorsec * i, both, horsec + dhorsec * i, toph);
				drawLine(painter, horsec + dhorsec * i, toph, dhorsec * (i + 1), both);
			}
			drawLine(painter, dhorsec * 2, both, ENV_W_, toph);
		}
			break;
		case 7:
		{
			drawLine(painter, 0, both, horsec, toph);
			drawLine(painter, horsec, toph, horsec, both);
			drawLine(painter, horsec, both, ENV_W_, both);
		}
			break;
		}
	}

	ui->envFrame->repaint();
}

void FMOperatorTable::on_ssgegCheckBox_stateChanged(int arg1)
{
	Q_UNUSED(arg1)
	if (ui->ssgegCheckBox->isChecked()) {
		ui->ssgegSlider->setEnabled(true);
		if (!isIgnoreEvent_) emit operatorValueChanged(Ui::FMOperatorParameter::SSGEG, ui->ssgegSlider->value());
	}
	else {
		ui->ssgegSlider->setEnabled(false);
		if (!isIgnoreEvent_) emit operatorValueChanged(Ui::FMOperatorParameter::SSGEG, -1);
	}
	repaintGraph();
}

void FMOperatorTable::on_groupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) emit operatorEnableChanged(arg1);
}

void FMOperatorTable::on_groupBox_customContextMenuRequested(const QPoint &pos)
{
	QPoint globalPos = ui->groupBox->mapToGlobal(pos);
	QMenu menu;
	// Leave Before Qt5.7.0 style due to windows xp
	QAction* copyEnv = menu.addAction(tr("Copy envelope"));
	QObject::connect(copyEnv, &QAction::triggered, this, [&] { emit copyEnvelopePressed(); });
	QAction* pasteEnv = menu.addAction(tr("Paste envelope"));
	QObject::connect(pasteEnv, &QAction::triggered, this, [&] { emit pasteEnvelopePressed(); });
	QMenu* pasteFrom = menu.addMenu(tr("Paste envelope From"));
	for (size_t i = 0; i < envelopeTypes_.size(); ++i) {
		QAction* act = pasteFrom->addAction(envelopeTypes_[i]);
		act->setData(static_cast<int>(i));
	}
	QObject::connect(pasteFrom, &QMenu::triggered,
					 this, [&](QAction* action) { emit pasteEnvelopeFromPressed(action->data().toInt()); });
	menu.addSeparator();
	QAction* copyOp = menu.addAction(tr("Copy operator"));
	QObject::connect(copyOp, &QAction::triggered, this, [&] { emit copyOperatorPressed(number_); });
	QAction* pasteOp = menu.addAction(tr("Paste operator"));
	QObject::connect(pasteOp, &QAction::triggered, this, [&] { emit pasteOperatorPressed(number_); });

	QClipboard* clipboard = QApplication::clipboard();
	pasteEnv->setEnabled(clipboard->text().startsWith("FM_ENVELOPE:"));
	pasteOp->setEnabled(clipboard->text().startsWith("FM_OPERATOR:"));

	menu.exec(globalPos);
}
