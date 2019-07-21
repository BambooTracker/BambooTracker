#include "fm_operator_table.hpp"
#include "ui_fm_operator_table.h"
#include <QString>
#include <QGraphicsScene>
#include <QPen>
#include <QBrush>
#include <QMenu>
#include <QClipboard>
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

	QString name[] = { "AR", "DR", "SR", "RR", "SL", "TL", "KS", "ML", "DT"};
	int maxValue[] = { 31, 31, 31, 15, 15, 127, 3, 15, 7};

	int n = 0;
	for (auto& pair : sliderMap_) {
		pair.second->setText(name[n]);
		pair.second->setMaximum(maxValue[n]);
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
		++n;
	}

	ui->ssgegSlider->setEnabled(false);
	ui->ssgegSlider->setText(tr("Type"));
	ui->ssgegSlider->setMaximum(7);
	QObject::connect(ui->ssgegSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int value) {
		repaintGraph();
		if (!isIgnoreEvent_) emit operatorValueChanged(Ui::FMOperatorParameter::SSGEG, value);
	});

	// Init graph
	auto scene = new QGraphicsScene(0, 0, 201, 128, ui->envelopeGraphicsView);
	ui->envelopeGraphicsView->setScene(scene);
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
	if (!palette_) return;

	auto scene = ui->envelopeGraphicsView->scene();
	double envHeight = ui->ssgegCheckBox->isChecked() ? (scene->height() - 40) : scene->height();
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
	scene->addRect(0, 0, scene->width(), scene->height(),
				   QPen(palette_->instFMEnvBackColor), QBrush(palette_->instFMEnvBackColor));

	auto linePen = QPen(QBrush(palette_->instFMEnvLine1Color), 2);
	scene->addLine(startx, envHeight, tlx, tly, linePen);
	scene->addLine(tlx, tly, slx, sly, linePen);
	scene->addLine(slx, sly, rrx, rry, linePen);
	scene->addLine(rrx, rry, endx, endy, linePen);

	auto cclPen = QPen(palette_->instFMEnvCirclePenColor);
	auto cclBrush = QBrush(palette_->instFMEnvCircleBrushColor);
	scene->addEllipse(tlx-1, tly, 4, 4, cclPen, cclBrush);
	scene->addEllipse(slx-1, sly, 4, 4, cclPen, cclBrush);
	scene->addEllipse(rrx-1, rry, 4, 4, cclPen, cclBrush);

	if (ui->ssgegCheckBox->isChecked()) {
		double seph = scene->height() - 39;
		scene->addLine(0, seph, 200, seph, QPen(palette_->instFMEnvBorderColor));
		double toph = seph + 2;
		double both = scene->height();
		linePen.setBrush(palette_->instFMEnvLine2Color);
		switch (ui->ssgegSlider->value()) {
		case 0:
		{
			for (int i = 0; i < 5; ++i) {
				scene->addLine(40 * i, both, 40 * i, toph, linePen);
				scene->addLine(40 * i, toph, 40 * (i + 1), both, linePen);
			}
		}
			break;
		case 1:
		{
			scene->addLine(0, both, 0, toph, linePen);
			scene->addLine(0, toph, 40, both, linePen);
			scene->addLine(40, both, 200, both, linePen);
		}
			break;
		case 2:
		{
			scene->addLine(0, both, 0, toph, linePen);
			scene->addLine(0, toph, 40, both, linePen);
			for (int i = 0; i < 2; ++i) {
				scene->addLine(40 + 80 * i, both, 80 + 80 * i, toph, linePen);
				scene->addLine(80 + 80 * i, toph, 40 + 80 * (i + 1), both, linePen);
			}
		}
			break;
		case 3:
		{
			scene->addLine(0, both, 0, toph, linePen);
			scene->addLine(0, toph, 40, both, linePen);
			scene->addLine(40, both, 40, toph, linePen);
			scene->addLine(40, toph, 200, toph, linePen);
		}
			break;
		case 4:
		{
			for (int i = 0; i < 5; ++i) {
				scene->addLine(40 * i, both, 40 * (i + 1), toph, linePen);
				scene->addLine(40 * (i + 1), toph, 40 * (i + 1), both, linePen);
			}
		}
			break;
		case 5:
		{
			scene->addLine(0, both, 40, toph, linePen);
			scene->addLine(40, toph, 200, toph, linePen);
		}
			break;
		case 6:
		{
			for (int i = 0; i < 2; ++i) {
				scene->addLine(80 * i, both, 40 + 80 * i, toph, linePen);
				scene->addLine(40 + 80 * i, toph, 80 * (i + 1), both, linePen);
			}
			scene->addLine(160, both, 200, toph, linePen);
		}
			break;
		case 7:
		{
			scene->addLine(0, both, 40, toph, linePen);
			scene->addLine(40, toph, 40, both, linePen);
			scene->addLine(40, both, 200, both, linePen);
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
