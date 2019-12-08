#include "fm_operator_table.hpp"
#include "ui_fm_operator_table.h"
#include <QString>
#include <QPen>
#include <QMenu>
#include <QClipboard>
#include <QPointF>
#include <QPainter>
#include "gui/event_guard.hpp"

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

	int n = 0;
	for (auto& pair : sliderMap_) {
		if (pair.second == ui->dtSlider) {
			QObject::connect(pair.second, &LabeledVerticalSlider::valueChanged,
							 this, [&](int value) {
				if (isDTNegative_) {
					switch (value) {
					case -1:	value = 5;	break;
					case -2:	value = 6;	break;
					case -3:	value = 7;	break;
					default:				break;
					}
				}
				repaintGraph();
				if (!isIgnoreEvent_) emit operatorValueChanged(pair.first, value);
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

	envmap_ = std::make_unique<QPixmap>();

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
	ui->groupBox->setTitle(tr("Operator ") + QString::number(n + 1));
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
		if (param == Ui::FMOperatorParameter::DT) {
			if (isDTNegative_) {
				switch (value) {
				case 4:	value = 0;	break;
				case 5:	value = -1;	break;
				case 6:	value = -2;	break;
				case 7:	value = -3;	break;
				default:		break;
				}
			}
		}
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
		int v = ui->dtSlider->value();
		switch (v) {
		case 4:	v = 0;	break;
		case 5:	v = -1;	break;
		case 6:	v = -2;	break;
		case 7:	v = -3;	break;
		default:		break;
		}
		ui->dtSlider->setMinimum(-3);
		ui->dtSlider->setMaximum(3);
		ui->dtSlider->setSign(true);
		ui->dtSlider->setValue(v);
	}
	else {
		int v = ui->dtSlider->value();
		switch (v) {
		case -1:	v = 5;	break;
		case -2:	v = 6;	break;
		case -3:	v = 7;	break;
		default:			break;
		}
		ui->dtSlider->setMinimum(0);
		ui->dtSlider->setMaximum(7);
		ui->dtSlider->setSign(false);
		ui->dtSlider->setValue(v);
	}
}

QString FMOperatorTable::toString() const
{
	auto str = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10")
			   .arg(QString::number(ui->arSlider->value()))
			   .arg(QString::number(ui->drSlider->value()))
			   .arg(QString::number(ui->srSlider->value()))
			   .arg(QString::number(ui->rrSlider->value()))
			   .arg(QString::number(ui->slSlider->value()))
			   .arg(QString::number(ui->tlSlider->value()))
			   .arg(QString::number(ui->ksSlider->value()))
			   .arg(QString::number(ui->mlSlider->value()))
			   .arg(QString::number(ui->dtSlider->value()))
			   .arg(ui->ssgegCheckBox->isChecked()
					? QString::number(ui->ssgegSlider->value())
					: "-1");
	return str;
}

bool FMOperatorTable::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == ui->envFrame) {
		if (event->type() == QEvent::Paint) {
			QPainter painter(ui->envFrame);
			painter.eraseRect(ui->envFrame->rect());
			painter.drawPixmap(ui->envFrame->rect(), *envmap_.get(), envmap_->rect());
		}
	}
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
	envmap_ = std::make_unique<QPixmap>(ui->envFrame->size());
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
			p4 = { 200, p3.y() };
		}
		else if (ui->rrSlider->value() == 31) {
			p4.setX(p3.x());
		}
		else {
			double rr = 127. / (marginAr * (15 - ui->rrSlider->value()) / 14.);
			p4.setX(p3.x() + p3.y() / rr);
		}
	}

	double envHeight = (ui->ssgegCheckBox->isChecked() ? 87 : 127);
	double envHrate = envHeight / 127.;
	double xr = envmap_->width() / 200.;
	double yr = envmap_->height() / 127.;
	p0 = { p0.x() * xr, (127. - p0.y()) * envHrate * yr };
	p1 = { p1.x() * xr, (127. - p1.y()) * envHrate * yr };
	p2 = { p2.x() * xr, (127. - p2.y()) * envHrate * yr };
	p3 = { p3.x() * xr, (127. - p3.y()) * envHrate * yr };
	p4 = { p4.x() * xr, (127. - p4.y()) * envHrate * yr };

	envmap_->fill(palette_->instFMEnvBackColor);
	QPainter painter(envmap_.get());

	painter.setPen(QPen(palette_->instFMEnvGridColor, 1));
	painter.drawLine(p1.x(), 0, p1.x(), envHeight * yr);
	painter.drawLine(p2.x(), 0, p2.x(), envHeight * yr);
	painter.drawLine(p3.x(), 0, p3.x(), envHeight * yr);
	painter.setPen(QPen(palette_->instFMEnvLine1Color, 2));
	painter.drawLines({ QLineF(p0, p1), QLineF(p1, p2), QLineF(p2, p3) });
	painter.setPen(QPen(palette_->instFMEnvLine2Color, 2));
	painter.drawLine(QLineF(p3, p4));

	if (ui->ssgegCheckBox->isChecked()) {
		double seph = 88;
		painter.setPen(QPen(palette_->instFMEnvBorderColor, 1));
		painter.drawLine(0, seph * yr, 200 * xr, seph * yr);
		double toph = seph + 2;
		double both = 127;
		painter.setPen(QPen(palette_->instFMEnvLine3Color, 2));
		switch (ui->ssgegSlider->value()) {
		case 0:
		{
			for (int i = 0; i < 5; ++i) {
				painter.drawLine(40 * i * xr, both * yr, 40 * i * xr, toph * yr);
				painter.drawLine(40 * i * xr, toph * yr, 40 * (i + 1) * xr, both * yr);
			}
		}
			break;
		case 1:
		{
			painter.drawLine(0, both * yr, 0, toph * yr);
			painter.drawLine(0, toph * yr, 40 * xr, both * yr);
			painter.drawLine(40 * xr, both * yr, 200 * xr, both * yr);
		}
			break;
		case 2:
		{
			painter.drawLine(0, both * yr, 0, toph * yr);
			painter.drawLine(0, toph * yr, 40 * xr, both * yr);
			for (int i = 0; i < 2; ++i) {
				painter.drawLine((40 + 80 * i) * xr, both * yr, (80 + 80 * i) * xr, toph * yr);
				painter.drawLine((80 + 80 * i) * xr, toph * yr, (40 + 80 * (i + 1)) * xr, both * yr);
			}
		}
			break;
		case 3:
		{
			painter.drawLine(0, both * yr, 0, toph * yr);
			painter.drawLine(0, toph * yr, 40 * xr, both * yr);
			painter.drawLine(40 * xr, both * yr, 40 * xr, toph * yr);
			painter.drawLine(40 * xr, toph * yr, 200 * xr, toph * yr);
		}
			break;
		case 4:
		{
			for (int i = 0; i < 5; ++i) {
				painter.drawLine(40 * i * xr, both * yr, 40 * (i + 1) * xr, toph * yr);
				painter.drawLine(40 * (i + 1) * xr, toph * yr, 40 * (i + 1) * xr, both * yr);
			}
		}
			break;
		case 5:
		{
			painter.drawLine(0, both * yr, 40 * xr, toph * yr);
			painter.drawLine(40 * xr, toph * yr, 200 * xr, toph * yr);
		}
			break;
		case 6:
		{
			for (int i = 0; i < 2; ++i) {
				painter.drawLine(80 * i * xr, both * yr, (40 + 80 * i) * xr, toph * yr);
				painter.drawLine((40 + 80 * i) * xr, toph * yr, 80 * (i + 1) * xr, both * yr);
			}
			painter.drawLine(160 * xr, both * yr, 200 * xr, toph * yr);
		}
			break;
		case 7:
		{
			painter.drawLine(0, both * yr, 40 * xr, toph * yr);
			painter.drawLine(40 * xr, toph * yr, 40 * xr, both * yr);
			painter.drawLine(40 * xr, both * yr, 200 * xr, both * yr);
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
		ui->arSlider->setValue(31);
		ui->arSlider->setEnabled(false);
		if (!isIgnoreEvent_) emit operatorValueChanged(Ui::FMOperatorParameter::SSGEG, ui->ssgegSlider->value());
	}
	else {
		ui->ssgegSlider->setEnabled(false);
		ui->arSlider->setEnabled(true);
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
