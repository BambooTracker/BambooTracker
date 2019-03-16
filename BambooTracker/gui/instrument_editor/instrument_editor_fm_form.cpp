#include "instrument_editor_fm_form.hpp"
#include "ui_instrument_editor_fm_form.h"
#include <stdexcept>
#include <QString>
#include <QPoint>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QRegularExpression>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QStringList>
#include <QMessageBox>
#include "gui/event_guard.hpp"
#include "misc.hpp"

InstrumentEditorFMForm::InstrumentEditorFMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorFMForm),
	instNum_(num),
	isIgnoreEvent_(false)
{
	ui->setupUi(this);

	installEventFilter(this);

	/******************** Envelope editor ********************/
	ui->envGroupBox->setContextMenuPolicy(Qt::CustomContextMenu);

	auto scene = new QGraphicsScene(ui->alGraphicsView);
	ui->alGraphicsView->setScene(scene);

	ui->alSlider->setText("AL");
	ui->alSlider->setMaximum(7);
	QObject::connect(ui->alSlider, &LabeledHorizontalSlider::valueChanged, this, [&](int value) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), FMEnvelopeParameter::AL, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
		paintAlgorithmDiagram();
		resizeAlgorithmDiagram();
	});
	ui->fbSlider->setText("FB");
	ui->fbSlider->setMaximum(7);
	QObject::connect(ui->fbSlider, &LabeledHorizontalSlider::valueChanged, this, [&](int value) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), FMEnvelopeParameter::FB, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->op1Table->setOperatorNumber(0);
	QObject::connect(ui->op1Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 0, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op1Table, &FMOperatorTable::operatorValueChanged,
					 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR1;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR1;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR1;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR1;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL1;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL1;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS1;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML1;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT1;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG1;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op1Table, &FMOperatorTable::copyEnvelopePressed,
					 this, &InstrumentEditorFMForm::copyEnvelope);
	QObject::connect(ui->op1Table, &FMOperatorTable::pasteEnvelopePressed,
					 this, &InstrumentEditorFMForm::pasteEnvelope);
	QObject::connect(ui->op1Table, &FMOperatorTable::copyOperatorPressed,
					 this, &InstrumentEditorFMForm::copyOperator);
	QObject::connect(ui->op1Table, &FMOperatorTable::pasteOperatorPressed,
					 this, &InstrumentEditorFMForm::pasteOperator);
	QObject::connect(ui->op1Table, &FMOperatorTable::pasteEnvelopeFromPressed,
					 this, &InstrumentEditorFMForm::pasteEnvelopeFrom);

	ui->op2Table->setOperatorNumber(1);
	QObject::connect(ui->op2Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 1, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op2Table, &FMOperatorTable::operatorValueChanged,
					 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR2;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR2;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR2;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR2;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL2;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL2;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS2;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML2;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT2;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG2;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op2Table, &FMOperatorTable::copyEnvelopePressed,
					 this, &InstrumentEditorFMForm::copyEnvelope);
	QObject::connect(ui->op2Table, &FMOperatorTable::pasteEnvelopePressed,
					 this, &InstrumentEditorFMForm::pasteEnvelope);
	QObject::connect(ui->op2Table, &FMOperatorTable::copyOperatorPressed,
					 this, &InstrumentEditorFMForm::copyOperator);
	QObject::connect(ui->op2Table, &FMOperatorTable::pasteOperatorPressed,
					 this, &InstrumentEditorFMForm::pasteOperator);
	QObject::connect(ui->op2Table, &FMOperatorTable::pasteEnvelopeFromPressed,
					 this, &InstrumentEditorFMForm::pasteEnvelopeFrom);

	ui->op3Table->setOperatorNumber(2);
	QObject::connect(ui->op3Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 2, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op3Table, &FMOperatorTable::operatorValueChanged,
					 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR3;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR3;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR3;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR3;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL3;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL3;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS3;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML3;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT3;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG3;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op3Table, &FMOperatorTable::copyEnvelopePressed,
					 this, &InstrumentEditorFMForm::copyEnvelope);
	QObject::connect(ui->op3Table, &FMOperatorTable::pasteEnvelopePressed,
					 this, &InstrumentEditorFMForm::pasteEnvelope);
	QObject::connect(ui->op3Table, &FMOperatorTable::copyOperatorPressed,
					 this, &InstrumentEditorFMForm::copyOperator);
	QObject::connect(ui->op3Table, &FMOperatorTable::pasteOperatorPressed,
					 this, &InstrumentEditorFMForm::pasteOperator);
	QObject::connect(ui->op3Table, &FMOperatorTable::pasteEnvelopeFromPressed,
					 this, &InstrumentEditorFMForm::pasteEnvelopeFrom);

	ui->op4Table->setOperatorNumber(3);
	QObject::connect(ui->op4Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 3, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op4Table, &FMOperatorTable::operatorValueChanged,
					 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR4;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR4;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR4;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR4;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL4;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL4;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS4;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML4;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT4;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG4;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op4Table, &FMOperatorTable::copyEnvelopePressed,
					 this, &InstrumentEditorFMForm::copyEnvelope);
	QObject::connect(ui->op4Table, &FMOperatorTable::pasteEnvelopePressed,
					 this, &InstrumentEditorFMForm::pasteEnvelope);
	QObject::connect(ui->op4Table, &FMOperatorTable::copyOperatorPressed,
					 this, &InstrumentEditorFMForm::copyOperator);
	QObject::connect(ui->op4Table, &FMOperatorTable::pasteOperatorPressed,
					 this, &InstrumentEditorFMForm::pasteOperator);
	QObject::connect(ui->op4Table, &FMOperatorTable::pasteEnvelopeFromPressed,
					 this, &InstrumentEditorFMForm::pasteEnvelopeFrom);

	/******************** LFO editor ********************/
	ui->lfoGroupBox->setContextMenuPolicy(Qt::CustomContextMenu);

	ui->lfoFreqSlider->setText("Freq");
	ui->lfoFreqSlider->setMaximum(7);
	QObject::connect(ui->lfoFreqSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int v) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::FREQ, v);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->pmsSlider->setText("PMS");
	ui->pmsSlider->setMaximum(7);
	QObject::connect(ui->pmsSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int v) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::PMS, v);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->amsSlider->setText("AMS");
	ui->amsSlider->setMaximum(3);
	QObject::connect(ui->amsSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int v) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::AMS, v);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	QObject::connect(ui->amOp1CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::AM1,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->amOp2CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::AM2,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->amOp3CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::AM3,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->amOp4CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::AM4,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->lfoStartSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int v) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::COUNT, v);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== OperatorSequence ==========//
	ui->opSeqTypeComboBox->addItem("AL", static_cast<int>(FMEnvelopeParameter::AL));
	ui->opSeqTypeComboBox->addItem("FB", static_cast<int>(FMEnvelopeParameter::FB));
	ui->opSeqTypeComboBox->addItem("AR1", static_cast<int>(FMEnvelopeParameter::AR1));
	ui->opSeqTypeComboBox->addItem("DR1", static_cast<int>(FMEnvelopeParameter::DR1));
	ui->opSeqTypeComboBox->addItem("SR1", static_cast<int>(FMEnvelopeParameter::SR1));
	ui->opSeqTypeComboBox->addItem("RR1", static_cast<int>(FMEnvelopeParameter::RR1));
	ui->opSeqTypeComboBox->addItem("SL1", static_cast<int>(FMEnvelopeParameter::SL1));
	ui->opSeqTypeComboBox->addItem("TL1", static_cast<int>(FMEnvelopeParameter::TL1));
	ui->opSeqTypeComboBox->addItem("KS1", static_cast<int>(FMEnvelopeParameter::KS1));
	ui->opSeqTypeComboBox->addItem("ML1", static_cast<int>(FMEnvelopeParameter::ML1));
	ui->opSeqTypeComboBox->addItem("DT1", static_cast<int>(FMEnvelopeParameter::DT1));
	ui->opSeqTypeComboBox->addItem("AR2", static_cast<int>(FMEnvelopeParameter::AR2));
	ui->opSeqTypeComboBox->addItem("DR2", static_cast<int>(FMEnvelopeParameter::DR2));
	ui->opSeqTypeComboBox->addItem("SR2", static_cast<int>(FMEnvelopeParameter::SR2));
	ui->opSeqTypeComboBox->addItem("RR2", static_cast<int>(FMEnvelopeParameter::RR2));
	ui->opSeqTypeComboBox->addItem("SL2", static_cast<int>(FMEnvelopeParameter::SL2));
	ui->opSeqTypeComboBox->addItem("TL2", static_cast<int>(FMEnvelopeParameter::TL2));
	ui->opSeqTypeComboBox->addItem("KS2", static_cast<int>(FMEnvelopeParameter::KS2));
	ui->opSeqTypeComboBox->addItem("ML2", static_cast<int>(FMEnvelopeParameter::ML2));
	ui->opSeqTypeComboBox->addItem("DT2", static_cast<int>(FMEnvelopeParameter::DT2));
	ui->opSeqTypeComboBox->addItem("AR3", static_cast<int>(FMEnvelopeParameter::AR3));
	ui->opSeqTypeComboBox->addItem("DR3", static_cast<int>(FMEnvelopeParameter::DR3));
	ui->opSeqTypeComboBox->addItem("SR3", static_cast<int>(FMEnvelopeParameter::SR3));
	ui->opSeqTypeComboBox->addItem("RR3", static_cast<int>(FMEnvelopeParameter::RR3));
	ui->opSeqTypeComboBox->addItem("SL3", static_cast<int>(FMEnvelopeParameter::SL3));
	ui->opSeqTypeComboBox->addItem("TL3", static_cast<int>(FMEnvelopeParameter::TL3));
	ui->opSeqTypeComboBox->addItem("KS3", static_cast<int>(FMEnvelopeParameter::KS3));
	ui->opSeqTypeComboBox->addItem("ML3", static_cast<int>(FMEnvelopeParameter::ML3));
	ui->opSeqTypeComboBox->addItem("DT3", static_cast<int>(FMEnvelopeParameter::DT3));
	ui->opSeqTypeComboBox->addItem("AR4", static_cast<int>(FMEnvelopeParameter::AR4));
	ui->opSeqTypeComboBox->addItem("DR4", static_cast<int>(FMEnvelopeParameter::DR4));
	ui->opSeqTypeComboBox->addItem("SR4", static_cast<int>(FMEnvelopeParameter::SR4));
	ui->opSeqTypeComboBox->addItem("RR4", static_cast<int>(FMEnvelopeParameter::RR4));
	ui->opSeqTypeComboBox->addItem("SL4", static_cast<int>(FMEnvelopeParameter::SL4));
	ui->opSeqTypeComboBox->addItem("TL4", static_cast<int>(FMEnvelopeParameter::TL4));
	ui->opSeqTypeComboBox->addItem("KS4", static_cast<int>(FMEnvelopeParameter::KS4));
	ui->opSeqTypeComboBox->addItem("ML4", static_cast<int>(FMEnvelopeParameter::ML4));
	ui->opSeqTypeComboBox->addItem("DT4", static_cast<int>(FMEnvelopeParameter::DT4));

	ui->opSeqEditor->setDefaultRow(0);
	ui->opSeqEditor->setLabelDiaplayMode(true);
	setOperatorSequenceEditor();

	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->addOperatorSequenceFMSequenceCommand(
						param, ui->opSeqNumSpinBox->value(), row, ui->opSeqEditor->getSequenceDataAt(col));
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->removeOperatorSequenceFMSequenceCommand(param, ui->opSeqNumSpinBox->value());
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->setOperatorSequenceFMSequenceCommand(
						param, ui->opSeqNumSpinBox->value(), col, row, ui->opSeqEditor->getSequenceDataAt(col));
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->setOperatorSequenceFMLoops(
						param, ui->opSeqNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setOperatorSequenceFMRelease(param, ui->opSeqNumSpinBox->value(), t, point);
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->opSeqTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onOperatorSequenceTypeChanged);

	//========== Arpeggio ==========//
	ui->arpEditor->setMaximumDisplayedRowCount(15);
	ui->arpEditor->setDefaultRow(48);
	ui->arpEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 96; ++i) {
		int d = i - 48;
		auto text = QString::number(d);
		if (d > 0) text = "+" + text;
		ui->arpEditor->AddRow(text);
	}
	ui->arpEditor->setUpperRow(55);
	ui->arpEditor->setMMLDisplay0As(-48);

	ui->arpTypeComboBox->addItem(tr("Absolute"), 0);
	ui->arpTypeComboBox->addItem(tr("Fix"), 1);
	ui->arpTypeComboBox->addItem(tr("Relative"), 2);

	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioFMSequenceCommand(
						ui->arpNumSpinBox->value(), row, ui->arpEditor->getSequenceDataAt(col));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioFMSequenceCommand(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioFMSequenceCommand(
						ui->arpNumSpinBox->value(), col, row, ui->arpEditor->getSequenceDataAt(col));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioFMLoops(
						ui->arpNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setArpeggioFMRelease(ui->arpNumSpinBox->value(), t, point);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->arpTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onArpeggioTypeChanged);

	//========== Pitch ==========//
	ui->ptEditor->setMaximumDisplayedRowCount(15);
	ui->ptEditor->setDefaultRow(127);
	ui->ptEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 255; ++i) {
		int d = i - 127;
		auto text = QString::number(d);
		if (d > 0) text = "+" + text;
		ui->ptEditor->AddRow(text);
	}
	ui->ptEditor->setUpperRow(134);
	ui->ptEditor->setMMLDisplay0As(-127);

	ui->ptTypeComboBox->addItem("Absolute", 0);
	ui->ptTypeComboBox->addItem("Relative", 2);

	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPitchFMSequenceCommand(
						ui->ptNumSpinBox->value(), row, ui->ptEditor->getSequenceDataAt(col));
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchFMSequenceCommand(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchFMSequenceCommand(
						ui->ptNumSpinBox->value(), col, row, ui->ptEditor->getSequenceDataAt(col));
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchFMLoops(
						ui->ptNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setPitchFMRelease(ui->ptNumSpinBox->value(), t, point);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->ptTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onPitchTypeChanged);
}

InstrumentEditorFMForm::~InstrumentEditorFMForm()
{
	delete ui;
}

int InstrumentEditorFMForm::getInstrumentNumber() const
{
	return instNum_;
}

void InstrumentEditorFMForm::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
	updateInstrumentParameters();
}

void InstrumentEditorFMForm::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;

	std::vector<QString> names;
	for (auto pair : config.lock()->getFMEnvelopeTextMap()) {
		names.push_back(QString::fromUtf8(pair.first.c_str(), pair.first.length()));
	}
	ui->op1Table->setEnvelopeSetNames(names);
	ui->op2Table->setEnvelopeSetNames(names);
	ui->op3Table->setEnvelopeSetNames(names);
	ui->op4Table->setEnvelopeSetNames(names);
}

void InstrumentEditorFMForm::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;

	ui->op1Table->setColorPalette(palette);
	ui->op2Table->setColorPalette(palette);
	ui->op3Table->setColorPalette(palette);
	ui->op4Table->setColorPalette(palette);

	ui->opSeqEditor->setColorPalette(palette);
	ui->arpEditor->setColorPalette(palette);
	ui->ptEditor->setColorPalette(palette);
}

ReleaseType InstrumentEditorFMForm::convertReleaseTypeForData(VisualizedInstrumentMacroEditor::ReleaseType type)
{
	switch (type) {
	case VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE:
		return ReleaseType::NO_RELEASE;
	case VisualizedInstrumentMacroEditor::ReleaseType::FIX:
		return ReleaseType::FIX;
	case VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE:
		return ReleaseType::ABSOLUTE;
	case VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE:
		return ReleaseType::RELATIVE;
	default:
		throw std::invalid_argument("Unexpected ReleaseType.");
	}
}

VisualizedInstrumentMacroEditor::ReleaseType InstrumentEditorFMForm::convertReleaseTypeForUI(ReleaseType type)
{
	switch (type) {
	case ReleaseType::NO_RELEASE:
		return VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE;
	case ReleaseType::FIX:
		return VisualizedInstrumentMacroEditor::ReleaseType::FIX;
	case ReleaseType::ABSOLUTE:
		return VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE;
	case ReleaseType::RELATIVE:
		return VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE;
	default:
		throw std::invalid_argument("Unexpected ReleaseType.");
	}
}

void InstrumentEditorFMForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());
	auto name = QString::fromUtf8(instFM->getName().c_str(), instFM->getName().length());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentEnvelopeParameters();
	setInstrumentLFOParameters();
	setInstrumentOperatorSequenceParameters();
	setInstrumentArpeggioParameters();
	setInstrumentPitchParameters();

	ui->envResetCheckBox->setChecked(instFM->getEnvelopeResetEnabled());
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorFMForm::keyPressEvent(QKeyEvent *event)
{
	// For jam key on

	// Check keys
	QString seq = QKeySequence(event->modifiers() | event->key()).toString();
	if (seq == QKeySequence(QString::fromUtf8(config_.lock()->getOctaveUpKey().c_str(),
											  config_.lock()->getOctaveUpKey().length())).toString()) {
		emit octaveChanged(true);
		return;
	}
	else if (seq == QKeySequence(QString::fromUtf8(config_.lock()->getOctaveDownKey().c_str(),
												   config_.lock()->getOctaveDownKey().length())).toString()) {
		emit octaveChanged(false);
		return;
	}

	// General keys
	switch (event->key()) {
	//case Qt::Key_Return:	emit playStatusChanged(0);	break;
	case Qt::Key_F5:		emit playStatusChanged(1);	break;
	case Qt::Key_F6:		emit playStatusChanged(2);	break;
	case Qt::Key_F7:		emit playStatusChanged(3);	break;
	case Qt::Key_F8:		emit playStatusChanged(-1);	break;
	case Qt::Key_Escape:	close();					break;
	default:
		if (!event->isAutoRepeat()) {
			// Musical keyboard
			switch (event->key()) {
			case Qt::Key_Z:
			case Qt::Key_S:
			case Qt::Key_X:
			case Qt::Key_D:
			case Qt::Key_C:
			case Qt::Key_V:
			case Qt::Key_G:
			case Qt::Key_B:
			case Qt::Key_H:
			case Qt::Key_N:
			case Qt::Key_J:
			case Qt::Key_M:
			case Qt::Key_Comma:
			case Qt::Key_L:
			case Qt::Key_Period:
			case Qt::Key_Q:
			case Qt::Key_2:
			case Qt::Key_W:
			case Qt::Key_3:
			case Qt::Key_E:
			case Qt::Key_R:
			case Qt::Key_5:
			case Qt::Key_T:
			case Qt::Key_6:
			case Qt::Key_Y:
			case Qt::Key_7:
			case Qt::Key_U:
			case Qt::Key_I:
			case Qt::Key_9:
			case Qt::Key_O:
				emit jamKeyOnEvent(event);
				break;
			default: break;
			}
		}
		break;
	}
}

// MUST DIRECT CONNECTION
void InstrumentEditorFMForm::keyReleaseEvent(QKeyEvent *event)
{
	// For jam key off
	if (!event->isAutoRepeat()) {
		switch (event->key()) {
		case Qt::Key_Z:
		case Qt::Key_S:
		case Qt::Key_X:
		case Qt::Key_D:
		case Qt::Key_C:
		case Qt::Key_V:
		case Qt::Key_G:
		case Qt::Key_B:
		case Qt::Key_H:
		case Qt::Key_N:
		case Qt::Key_J:
		case Qt::Key_M:
		case Qt::Key_Comma:
		case Qt::Key_L:
		case Qt::Key_Period:
		case Qt::Key_Q:
		case Qt::Key_2:
		case Qt::Key_W:
		case Qt::Key_3:
		case Qt::Key_E:
		case Qt::Key_R:
		case Qt::Key_5:
		case Qt::Key_T:
		case Qt::Key_6:
		case Qt::Key_Y:
		case Qt::Key_7:
		case Qt::Key_U:
		case Qt::Key_I:
		case Qt::Key_9:
		case Qt::Key_O:
			emit jamKeyOffEvent(event);
			break;
		default: break;
		}
	}
}

void InstrumentEditorFMForm::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)

	paintAlgorithmDiagram();
	resizeAlgorithmDiagram();
}

void InstrumentEditorFMForm::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)

	resizeAlgorithmDiagram();
}

//========== Envelope ==========//
void InstrumentEditorFMForm::setInstrumentEnvelopeParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->envNumSpinBox->setValue(instFM->getEnvelopeNumber());
	onEnvelopeNumberChanged();
	ui->alSlider->setValue(instFM->getEnvelopeParameter(FMEnvelopeParameter::AL));
	ui->fbSlider->setValue(instFM->getEnvelopeParameter(FMEnvelopeParameter::FB));
	ui->op1Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG1));
	ui->op2Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG2));
	ui->op3Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG3));
	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG4));
}

void InstrumentEditorFMForm::setInstrumentEnvelopeParameters(QString data)
{
	QRegularExpression re("^(?<fb>\\d+),(?<al>\\d+),\\s*"
						  "(?<ar1>\\d+),(?<dr1>\\d+),(?<sr1>\\d+),(?<rr1>\\d+),(?<sl1>\\d+),"
						  "(?<tl1>\\d+),(?<ks1>\\d+),(?<ml1>\\d+),(?<dt1>\\d+),(?<ssgeg1>-?\\d+),\\s*"
						  "(?<ar2>\\d+),(?<dr2>\\d+),(?<sr2>\\d+),(?<rr2>\\d+),(?<sl2>\\d+),"
						  "(?<tl2>\\d+),(?<ks2>\\d+),(?<ml2>\\d+),(?<dt2>\\d+),(?<ssgeg2>-?\\d+),\\s*"
						  "(?<ar3>\\d+),(?<dr3>\\d+),(?<sr3>\\d+),(?<rr3>\\d+),(?<sl3>\\d+),"
						  "(?<tl3>\\d+),(?<ks3>\\d+),(?<ml3>\\d+),(?<dt3>\\d+),(?<ssgeg3>-?\\d+),\\s*"
						  "(?<ar4>\\d+),(?<dr4>\\d+),(?<sr4>\\d+),(?<rr4>\\d+),(?<sl4>\\d+),"
						  "(?<tl4>\\d+),(?<ks4>\\d+),(?<ml4>\\d+),(?<dt4>\\d+),(?<ssgeg4>-?\\d+),?\\s*");
	QRegularExpressionMatch match = re.match(data);

	if (match.hasMatch()) {
		ui->fbSlider->setValue(match.captured("fb").toInt());
		ui->alSlider->setValue(match.captured("al").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg1").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg2").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg3").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg4").toInt());
	}
}

void InstrumentEditorFMForm::setInstrumentEnvelopeParameters(QString envType, QString data)
{
	data.replace(QRegularExpression(R"(\D+)"), ",");
	if (data.startsWith(",")) data.remove(0, 1);
	if (data.endsWith(",")) data.remove(data.length() - 1, 1);
	QStringList digits = data.split(",");

	std::vector<FMEnvelopeTextType> set = config_.lock()->getFMEnvelopeTextMap().at(envType.toUtf8().toStdString());
	if (static_cast<int>(set.size()) > digits.size()) {
		QMessageBox::critical(this, tr("Error"), tr("Did not match the clipboard text format with %1.").arg(envType));
		return;
	}

	for (int i = 0; i < digits.size(); ++i) {
		int d = digits[i].toInt();
		switch (set[i]) {
		case FMEnvelopeTextType::Skip:	break;
		case FMEnvelopeTextType::AL:	ui->alSlider->setValue(d);	break;
		case FMEnvelopeTextType::FB:	ui->fbSlider->setValue(d);	break;
		case FMEnvelopeTextType::AR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::AR, d);	break;
		case FMEnvelopeTextType::DR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::DR, d);	break;
		case FMEnvelopeTextType::SR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::SR, d);	break;
		case FMEnvelopeTextType::RR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::RR, d);	break;
		case FMEnvelopeTextType::SL1:	ui->op1Table->setValue(Ui::FMOperatorParameter::SL, d);	break;
		case FMEnvelopeTextType::TL1:	ui->op1Table->setValue(Ui::FMOperatorParameter::TL, d);	break;
		case FMEnvelopeTextType::KS1:	ui->op1Table->setValue(Ui::FMOperatorParameter::KS, d);	break;
		case FMEnvelopeTextType::ML1:	ui->op1Table->setValue(Ui::FMOperatorParameter::ML, d);	break;
		case FMEnvelopeTextType::DT1:	ui->op1Table->setValue(Ui::FMOperatorParameter::DT, d);	break;
		case FMEnvelopeTextType::AR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::AR, d);	break;
		case FMEnvelopeTextType::DR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::DR, d);	break;
		case FMEnvelopeTextType::SR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::SR, d);	break;
		case FMEnvelopeTextType::RR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::RR, d);	break;
		case FMEnvelopeTextType::SL2:	ui->op2Table->setValue(Ui::FMOperatorParameter::SL, d);	break;
		case FMEnvelopeTextType::TL2:	ui->op2Table->setValue(Ui::FMOperatorParameter::TL, d);	break;
		case FMEnvelopeTextType::KS2:	ui->op2Table->setValue(Ui::FMOperatorParameter::KS, d);	break;
		case FMEnvelopeTextType::ML2:	ui->op2Table->setValue(Ui::FMOperatorParameter::ML, d);	break;
		case FMEnvelopeTextType::DT2:	ui->op2Table->setValue(Ui::FMOperatorParameter::DT, d);	break;
		case FMEnvelopeTextType::AR3:	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, d);	break;
		case FMEnvelopeTextType::DR3:	ui->op3Table->setValue(Ui::FMOperatorParameter::DR, d);	break;
		case FMEnvelopeTextType::SR3:	ui->op3Table->setValue(Ui::FMOperatorParameter::SR, d);	break;
		case FMEnvelopeTextType::RR3:	ui->op3Table->setValue(Ui::FMOperatorParameter::RR, d);	break;
		case FMEnvelopeTextType::SL3:	ui->op3Table->setValue(Ui::FMOperatorParameter::SL, d);	break;
		case FMEnvelopeTextType::TL3:	ui->op3Table->setValue(Ui::FMOperatorParameter::TL, d);	break;
		case FMEnvelopeTextType::KS3:	ui->op3Table->setValue(Ui::FMOperatorParameter::KS, d);	break;
		case FMEnvelopeTextType::ML3:	ui->op3Table->setValue(Ui::FMOperatorParameter::ML, d);	break;
		case FMEnvelopeTextType::DT3:	ui->op3Table->setValue(Ui::FMOperatorParameter::DT, d);	break;
		case FMEnvelopeTextType::AR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, d);	break;
		case FMEnvelopeTextType::DR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::DR, d);	break;
		case FMEnvelopeTextType::SR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::SR, d);	break;
		case FMEnvelopeTextType::RR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::RR, d);	break;
		case FMEnvelopeTextType::SL4:	ui->op4Table->setValue(Ui::FMOperatorParameter::SL, d);	break;
		case FMEnvelopeTextType::TL4:	ui->op4Table->setValue(Ui::FMOperatorParameter::TL, d);	break;
		case FMEnvelopeTextType::KS4:	ui->op4Table->setValue(Ui::FMOperatorParameter::KS, d);	break;
		case FMEnvelopeTextType::ML4:	ui->op4Table->setValue(Ui::FMOperatorParameter::ML, d);	break;
		case FMEnvelopeTextType::DT4:	ui->op4Table->setValue(Ui::FMOperatorParameter::DT, d);	break;
		}
	}
}

void InstrumentEditorFMForm::setInstrumentOperatorParameters(int opNum, QString data)
{
	QRegularExpression re("^(?<ar>\\d+),(?<dr>\\d+),(?<sr>\\d+),(?<rr>\\d+),(?<sl>\\d+),"
						  "(?<tl>\\d+),(?<ks>\\d+),(?<ml>\\d+),(?<dt>\\d+),(?<ssgeg>-?\\d+)");
	QRegularExpressionMatch match = re.match(data);

	if (match.hasMatch()) {
		switch (opNum) {
		case 0:
			ui->op1Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt").toInt());
			ui->op1Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg").toInt());
			break;
		case 1:
			ui->op2Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt").toInt());
			ui->op2Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg").toInt());
			break;
		case 2:
			ui->op3Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt").toInt());
			ui->op3Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg").toInt());
			break;
		case 3:
			ui->op4Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt").toInt());
			ui->op4Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg").toInt());
			break;
		}
	}
}

void InstrumentEditorFMForm::paintAlgorithmDiagram()
{
	if (!palette_) return;

	ui->alGraphicsView->setBackgroundBrush(QBrush(palette_->instFMAlBackColor));
	QGraphicsScene* scene = ui->alGraphicsView->scene();	// 200 * 70
	scene->clear();

	QPen pen(palette_->instFMAlForeColor);
	QBrush brush(palette_->instFMAlForeColor);
	auto one = new QGraphicsSimpleTextItem();
	one->setBrush(brush);
	one->setText("1");
	auto two = new QGraphicsSimpleTextItem();
	two->setBrush(brush);
	two->setText("2");
	auto three = new QGraphicsSimpleTextItem();
	three->setBrush(brush);
	three->setText("3");
	auto four = new QGraphicsSimpleTextItem();
	four->setBrush(brush);
	four->setText("4");
	switch (ui->alSlider->value()) {
	case 0:
	{
		scene->addRect(8, -4, 16, 8, pen, brush);
		scene->addRect(40, -4, 16, 8, pen, brush);
		scene->addRect(72, -4, 16, 8, pen, brush);
		scene->addRect(104, -4, 16, 8, pen, brush);
		scene->addLine(0, 0, 128, 0, pen);
		scene->addLine(0, -8, 32, -8, pen);
		scene->addLine(0, 0, 0, -8, pen);
		scene->addLine(32, 0, 32, -8, pen);
		one->setPos(14, 8);
		two->setPos(46, 8);
		three->setPos(78, 8);
		four->setPos(110, 8);
		break;
	}
	case 1:
	{
		scene->addRect(8, -12, 16, 8, pen, brush);
		scene->addRect(8, 4, 16, 8, pen, brush);
		scene->addRect(40, -4, 16, 8, pen, brush);
		scene->addRect(72, -4, 16, 8, pen, brush);
		scene->addLine(0, -8, 35, -8, pen);
		scene->addLine(24, 8, 35, 8, pen);
		scene->addLine(0, -16, 29, -16, pen);
		scene->addLine(0, -8, 0, -16, pen);
		scene->addLine(29, -8, 29, -16, pen);
		scene->addLine(35, -8, 35, 8, pen);
		scene->addLine(35, 0, 96, 0, pen);
		one->setPos(-8, -12);
		two->setPos(-8, 4);
		three->setPos(46, 8);
		four->setPos(78, 8);
		break;
	}
	case 2:
	{
		scene->addRect(40, -12, 16, 8, pen, brush);
		scene->addRect(8, 4, 16, 8, pen, brush);
		scene->addRect(40, 4, 16, 8, pen, brush);
		scene->addRect(72, -4, 16, 8, pen, brush);
		scene->addLine(32, -8, 67, -8, pen);
		scene->addLine(32, -8, 32, -16, pen);
		scene->addLine(61, -8, 61, -16, pen);
		scene->addLine(32, -16, 61, -16, pen);
		scene->addLine(24, 8, 67, 8, pen);
		scene->addLine(67, -8, 67, 8, pen);
		scene->addLine(67, 0, 96, 0, pen);
		one->setPos(24, -12);
		two->setPos(14, 16);
		three->setPos(46, 16);
		four->setPos(78, 8);
		break;
	}
	case 3:
	{
		scene->addRect(8, -12, 16, 8, pen, brush);
		scene->addRect(40, -12, 16, 8, pen, brush);
		scene->addRect(40, 4, 16, 8, pen, brush);
		scene->addRect(72, -4, 16, 8, pen, brush);
		scene->addLine(0, -8, 64, -8, pen);
		scene->addLine(0, -8, 0, -16, pen);
		scene->addLine(32, -8, 32, -16, pen);
		scene->addLine(0, -16, 32, -16, pen);
		scene->addLine(56, 8, 64, 8, pen);
		scene->addLine(64, -8, 64, 8, pen);
		scene->addLine(64, 0, 96, 0, pen);
		one->setPos(-8, -12);
		two->setPos(46, -24);
		three->setPos(46, 16);
		four->setPos(78, 8);
		break;
	}
	case 4:
	{
		scene->addRect(8, -12, 16, 8, pen, brush);
		scene->addRect(40, -12, 16, 8, pen, brush);
		scene->addRect(8, 4, 16, 8, pen, brush);
		scene->addRect(40, 4, 16, 8, pen, brush);
		scene->addLine(0, -8, 64, -8, pen);
		scene->addLine(0, -8, 0, -16, pen);
		scene->addLine(32, -8, 32, -16, pen);
		scene->addLine(0, -16, 32, -16, pen);
		scene->addLine(24, 8, 64, 8, pen);
		scene->addLine(64, -8, 64, 8, pen);
		scene->addLine(64, 0, 72, 0, pen);
		one->setPos(-8, -12);
		two->setPos(46, -24);
		three->setPos(14, 16);
		four->setPos(46, 16);
		break;
	}
	case 5:
	{
		scene->addRect(8, -4, 16, 8, pen, brush);
		scene->addRect(40, -20, 16, 8, pen, brush);
		scene->addRect(40, -4, 16, 8, pen, brush);
		scene->addRect(40, 12, 16, 8, pen, brush);
		scene->addLine(0, 0, 72, 0, pen);
		scene->addLine(0, 0, 0, -8, pen);
		scene->addLine(29, 0, 29, -8, pen);
		scene->addLine(0, -8, 29, -8, pen);
		scene->addLine(35, -16, 35, 16, pen);
		scene->addLine(64, -16, 64, 16, pen);
		scene->addLine(35, -16, 64, -16, pen);
		scene->addLine(35, 16, 64, 16, pen);
		one->setPos(14, 8);
		two->setPos(76, -20);
		three->setPos(76, -4);
		four->setPos(76, 12);
		break;
	}
	case 6:
	{
		scene->addRect(8, -20, 16, 8, pen, brush);
		scene->addRect(40, -20, 16, 8, pen, brush);
		scene->addRect(40, -4, 16, 8, pen, brush);
		scene->addRect(40, 12, 16, 8, pen, brush);
		scene->addLine(0, -16, 64, -16, pen);
		scene->addLine(0, -16, 0, -24, pen);
		scene->addLine(32, -16, 32, -24, pen);
		scene->addLine(0, -24, 32, -24, pen);
		scene->addLine(56, 0, 72, 0, pen);
		scene->addLine(56, 16, 64, 16, pen);
		scene->addLine(64, -16, 64, 16, pen);
		one->setPos(14, -8);
		two->setPos(76, -20);
		three->setPos(76, -4);
		four->setPos(76, 12);
		break;
	}
	case 7:
	{
		scene->addRect(-28, 8, 8, 16, pen, brush);
		scene->addRect(-12, 8, 8, 16, pen, brush);
		scene->addRect(4, 8, 8, 16, pen, brush);
		scene->addRect(20, 8, 8, 16, pen, brush);
		scene->addLine(-24, 0, -24, 35, pen);
		scene->addLine(-24, 0, -32, 0, pen);
		scene->addLine(-24, 29, -32, 29, pen);
		scene->addLine(-32, 0, -32, 29, pen);
		scene->addLine(-8, 24, -8, 35, pen);
		scene->addLine(8, 24, 8, 35, pen);
		scene->addLine(24, 24, 24, 35, pen);
		scene->addLine(-24, 35, 24, 35, pen);
		scene->addLine(0, 35, 0, 40, pen);
		one->setPos(-26, -12);
		two->setPos(-10, -12);
		three->setPos(6, -12);
		four->setPos(22, -12);
		break;
	}
	}
	scene->addItem(one);
	scene->addItem(two);
	scene->addItem(three);
	scene->addItem(four);

	scene->setSceneRect(scene->itemsBoundingRect());
}

void InstrumentEditorFMForm::resizeAlgorithmDiagram()
{
	ui->alGraphicsView->fitInView(ui->alGraphicsView->scene()->itemsBoundingRect(),
								  Qt::AspectRatioMode::KeepAspectRatio);
}

/********** Slots **********/
void InstrumentEditorFMForm::copyEnvelope()
{
	QApplication::clipboard()->setText(QString("FM_ENVELOPE:%1,%2,\n%3,\n%4,\n%5,\n%6,")
									   .arg(QString::number(ui->fbSlider->value()))
									   .arg(QString::number(ui->alSlider->value()))
									   .arg(ui->op1Table->toString())
									   .arg(ui->op2Table->toString())
									   .arg(ui->op3Table->toString())
									   .arg(ui->op4Table->toString()));
}

void InstrumentEditorFMForm::pasteEnvelope()
{
	QString data = QApplication::clipboard()->text().remove("FM_ENVELOPE:");
	setInstrumentEnvelopeParameters(data);
}

void InstrumentEditorFMForm::pasteEnvelopeFrom(QString type)
{
	setInstrumentEnvelopeParameters(type, QApplication::clipboard()->text());
}

void InstrumentEditorFMForm::copyOperator(int opNum)
{
	QString text;
	switch (opNum) {
	case 0:	text = ui->op1Table->toString();	break;
	case 1:	text = ui->op2Table->toString();	break;
	case 2:	text = ui->op3Table->toString();	break;
	case 3:	text = ui->op4Table->toString();	break;
	}

	QApplication::clipboard()->setText(QString("FM_OPERATOR:") + text);
}

void InstrumentEditorFMForm::pasteOperator(int opNum)
{
	QString data = QApplication::clipboard()->text().remove("FM_OPERATOR:");
	setInstrumentOperatorParameters(opNum, data);
}

void InstrumentEditorFMForm::on_envNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMEnvelope(instNum_, arg1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

void InstrumentEditorFMForm::on_envGroupBox_customContextMenuRequested(const QPoint &pos)
{
	QPoint globalPos = ui->envGroupBox->mapToGlobal(pos);
	QMenu menu;
	// Leave Before Qt5.7.0 style due to windows xp
	QAction* copy = menu.addAction(tr("Copy envelope"));
	QObject::connect(copy, &QAction::triggered, this, &InstrumentEditorFMForm::copyEnvelope);
	QAction* paste = menu.addAction(tr("Paste envelope"));
	QObject::connect(paste, &QAction::triggered, this, &InstrumentEditorFMForm::pasteEnvelope);
	paste->setEnabled(QApplication::clipboard()->text().startsWith("FM_ENVELOPE:"));
	QMenu* pasteFrom = menu.addMenu(tr("Paste envelope From"));
	for (auto pair : config_.lock()->getFMEnvelopeTextMap())
		pasteFrom->addAction(QString::fromUtf8(pair.first.c_str(), pair.first.length()));
	QObject::connect(pasteFrom, &QMenu::triggered,
					 this, [&](QAction* action) { pasteEnvelopeFrom(action->text()); });

	menu.exec(globalPos);
}

void InstrumentEditorFMForm::onEnvelopeParameterChanged(int envNum)
{
	if (ui->envNumSpinBox->value() == envNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentEnvelopeParameters();
	}
}

void InstrumentEditorFMForm::onEnvelopeNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getEnvelopeFMUsers(ui->envNumSpinBox->value());
	for (auto& n : users) {
		str += (QString("%1").arg(n, 2, 16, QChar('0')).toUpper() + ",");
	}
	str.chop(1);
	ui->envUsersLineEdit->setText(str);
}

//========== LFO ==========//
void InstrumentEditorFMForm::setInstrumentLFOParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->lfoNumSpinBox->setValue(instFM->getLFONumber());
	ui->lfoFreqSlider->setValue(instFM->getLFOParameter(FMLFOParameter::FREQ));
	ui->pmsSlider->setValue(instFM->getLFOParameter(FMLFOParameter::PMS));
	ui->amsSlider->setValue(instFM->getLFOParameter(FMLFOParameter::AMS));
	ui->amOp1CheckBox->setChecked(instFM->getLFOParameter(FMLFOParameter::AM1));
	ui->amOp2CheckBox->setChecked(instFM->getLFOParameter(FMLFOParameter::AM2));
	ui->amOp3CheckBox->setChecked(instFM->getLFOParameter(FMLFOParameter::AM3));
	ui->amOp4CheckBox->setChecked(instFM->getLFOParameter(FMLFOParameter::AM4));
	ui->lfoStartSpinBox->setValue(instFM->getLFOParameter(FMLFOParameter::COUNT));
	if (instFM->getLFOEnabled()) {
		ui->lfoGroupBox->setChecked(true);
		onLFONumberChanged();
	}
	else {
		ui->lfoGroupBox->setChecked(false);
	}
}

void InstrumentEditorFMForm::setInstrumentLFOParameters(QString data)
{
	QRegularExpression re("^(?<freq>\\d+),(?<pms>\\d+),(?<ams>\\d+),(?<cnt>\\d+),"
						  "(?<am1>\\d+),(?<am2>\\d+),(?<am3>\\d+),(?<am4>\\d+)");
	QRegularExpressionMatch match = re.match(data);

	if (match.hasMatch()) {
		ui->lfoFreqSlider->setValue(match.captured("freq").toInt());
		ui->pmsSlider->setValue(match.captured("pms").toInt());
		ui->amsSlider->setValue(match.captured("ams").toInt());
		ui->lfoStartSpinBox->setValue(match.captured("cnt").toInt());
		ui->amOp1CheckBox->setChecked(match.captured("am1").toInt() == 1);
		ui->amOp2CheckBox->setChecked(match.captured("am2").toInt() == 1);
		ui->amOp3CheckBox->setChecked(match.captured("am3").toInt() == 1);
		ui->amOp4CheckBox->setChecked(match.captured("am4").toInt() == 1);
	}
}

QString InstrumentEditorFMForm::toLFOString() const
{
	auto str = QString("%1,%2,%3,%4,%5,%6,%7,%8")
			   .arg(QString::number(ui->lfoFreqSlider->value()))
			   .arg(QString::number(ui->pmsSlider->value()))
			   .arg(QString::number(ui->amsSlider->value()))
			   .arg(QString::number(ui->lfoStartSpinBox->value()))
			   .arg(QString::number(ui->amOp1CheckBox->isChecked() ? 1 : 0))
			   .arg(QString::number(ui->amOp2CheckBox->isChecked() ? 1 : 0))
			   .arg(QString::number(ui->amOp3CheckBox->isChecked() ? 1 : 0))
			   .arg(QString::number(ui->amOp4CheckBox->isChecked() ? 1 : 0));
	return str;
}

/********** Slots **********/
void InstrumentEditorFMForm::onLFOParameterChanged(int lfoNum)
{
	if (ui->lfoNumSpinBox->value() == lfoNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentLFOParameters();
	}
}

void InstrumentEditorFMForm::onLFONumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getLFOFMUsers(ui->lfoNumSpinBox->value());
	for (auto& n : users) {
		str += (QString("%1").arg(n, 2, 16, QChar('0')).toUpper() + ",");
	}
	str.chop(1);
	ui->lfoUsersLineEdit->setText(str);
}

void InstrumentEditorFMForm::on_lfoGroupBox_customContextMenuRequested(const QPoint &pos)
{
	QClipboard* clipboard = QApplication::clipboard();
	QPoint globalPos = ui->lfoGroupBox->mapToGlobal(pos);

	QMenu menu;
	// Leave Before Qt5.7.0 style due to windows xp
	QAction* copy = menu.addAction(tr("Copy LFO parameters"));
	QObject::connect(copy, &QAction::triggered, this, [&, clipboard]() {
		clipboard->setText("FM_LFO:" + toLFOString());
	});
	QAction* paste = menu.addAction(tr("Paste LFO parameters"));
	QObject::connect(paste, &QAction::triggered, this, [&, clipboard]() {
		QString data = clipboard->text().remove("FM_LFO:");
		setInstrumentLFOParameters(data);
	});
	if (!ui->lfoGroupBox->isChecked()) {
		copy->setEnabled(false);
		paste->setEnabled(false);
	}
	else if (!clipboard->text().startsWith("FM_LFO:")) {
		paste->setEnabled(false);
	}

	menu.exec(globalPos);
}

void InstrumentEditorFMForm::on_lfoNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMLFO(instNum_, arg1);
		setInstrumentLFOParameters();
		emit lfoNumberChanged();
		emit modified();
	}

	onLFONumberChanged();
}

void InstrumentEditorFMForm::on_lfoGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMLFOEnabled(instNum_, arg1);
		setInstrumentLFOParameters();
		emit lfoNumberChanged();
		emit modified();
	}

	onLFONumberChanged();
}

//--- OperatorSequence
FMEnvelopeParameter InstrumentEditorFMForm::getOperatorSequenceParameter() const
{
	return static_cast<FMEnvelopeParameter>(ui->opSeqTypeComboBox->currentData(Qt::UserRole).toInt());
}

void InstrumentEditorFMForm::setInstrumentOperatorSequenceParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	FMEnvelopeParameter param = getOperatorSequenceParameter();

	ui->opSeqNumSpinBox->setValue(instFM->getOperatorSequenceNumber(param));
	ui->opSeqEditor->clearData();
	setOperatorSequenceEditor();
	for (auto& com : instFM->getOperatorSequenceSequence(param)) {
		ui->opSeqEditor->addSequenceCommand(com.type);
	}
	for (auto& l : instFM->getOperatorSequenceLoops(param)) {
		ui->opSeqEditor->addLoop(l.begin, l.end, l.times);
	}
	ui->opSeqEditor->setRelease(convertReleaseTypeForUI(instFM->getOperatorSequenceRelease(param).type),
								instFM->getOperatorSequenceRelease(param).begin);
	if (instFM->getOperatorSequenceEnabled(param)) {
		ui->opSeqEditGroupBox->setChecked(true);
		onOperatorSequenceNumberChanged();
	}
	else {
		ui->opSeqEditGroupBox->setChecked(false);
	}
}

void InstrumentEditorFMForm::setOperatorSequenceEditor()
{
	ui->opSeqEditor->clearRow();

	FMEnvelopeParameter param = getOperatorSequenceParameter();
	switch (param) {
	case FMEnvelopeParameter::AL:
	case FMEnvelopeParameter::FB:
	case FMEnvelopeParameter::DT1:
	case FMEnvelopeParameter::DT2:
	case FMEnvelopeParameter::DT3:
	case FMEnvelopeParameter::DT4:
		ui->opSeqEditor->setMaximumDisplayedRowCount(8);
		for (int i = 0; i < 8; ++i) {
			ui->opSeqEditor->AddRow(QString::number(i));
		}
		ui->opSeqEditor->setUpperRow(7);
		break;
	case FMEnvelopeParameter::AR1:
	case FMEnvelopeParameter::AR2:
	case FMEnvelopeParameter::AR3:
	case FMEnvelopeParameter::AR4:
	case FMEnvelopeParameter::DR1:
	case FMEnvelopeParameter::DR2:
	case FMEnvelopeParameter::DR3:
	case FMEnvelopeParameter::DR4:
	case FMEnvelopeParameter::SR1:
	case FMEnvelopeParameter::SR2:
	case FMEnvelopeParameter::SR3:
	case FMEnvelopeParameter::SR4:
		ui->opSeqEditor->setMaximumDisplayedRowCount(16);
		for (int i = 0; i < 32; ++i) {
			ui->opSeqEditor->AddRow(QString::number(i));
		}
		ui->opSeqEditor->setUpperRow(15);
		break;
	case FMEnvelopeParameter::RR1:
	case FMEnvelopeParameter::RR2:
	case FMEnvelopeParameter::RR3:
	case FMEnvelopeParameter::RR4:
	case FMEnvelopeParameter::SL1:
	case FMEnvelopeParameter::SL2:
	case FMEnvelopeParameter::SL3:
	case FMEnvelopeParameter::SL4:
	case FMEnvelopeParameter::ML1:
	case FMEnvelopeParameter::ML2:
	case FMEnvelopeParameter::ML3:
	case FMEnvelopeParameter::ML4:
		ui->opSeqEditor->setMaximumDisplayedRowCount(16);
		for (int i = 0; i < 16; ++i) {
			ui->opSeqEditor->AddRow(QString::number(i));
		}
		ui->opSeqEditor->setUpperRow(15);
		break;
	case FMEnvelopeParameter::KS1:
	case FMEnvelopeParameter::KS2:
	case FMEnvelopeParameter::KS3:
	case FMEnvelopeParameter::KS4:
		ui->opSeqEditor->setMaximumDisplayedRowCount(4);
		for (int i = 0; i < 4; ++i) {
			ui->opSeqEditor->AddRow(QString::number(i));
		}
		ui->opSeqEditor->setUpperRow(3);
		break;
	case FMEnvelopeParameter::TL1:
	case FMEnvelopeParameter::TL2:
	case FMEnvelopeParameter::TL3:
	case FMEnvelopeParameter::TL4:
		ui->opSeqEditor->setMaximumDisplayedRowCount(16);
		for (int i = 0; i < 128; ++i) {
			ui->opSeqEditor->AddRow(QString::number(i));
		}
		ui->opSeqEditor->setUpperRow(15);
		break;
	default:
		break;
	}
}

/********** Slots **********/
void InstrumentEditorFMForm::onOperatorSequenceNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getOperatorSequenceFMUsers(getOperatorSequenceParameter(), ui->opSeqNumSpinBox->value());
	for (auto& n : users) {
		str += (QString("%1").arg(n, 2, 16, QChar('0')).toUpper() + ",");
	}
	str.chop(1);

	ui->opSeqUsersLineEdit->setText(str);
}

void InstrumentEditorFMForm::onOperatorSequenceParameterChanged(FMEnvelopeParameter param, int tnNum)
{
	if (param == getOperatorSequenceParameter() && ui->opSeqNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentOperatorSequenceParameters();
	}
}

void InstrumentEditorFMForm::onOperatorSequenceTypeChanged(int type)
{
	Q_UNUSED(type)

	if (!isIgnoreEvent_) {
		setInstrumentOperatorSequenceParameters();
	}
}

void InstrumentEditorFMForm::on_opSeqEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMOperatorSequenceEnabled(instNum_, getOperatorSequenceParameter(), arg1);
		setInstrumentOperatorSequenceParameters();
		emit operatorSequenceNumberChanged();
		emit modified();
	}

	onOperatorSequenceNumberChanged();
}

void InstrumentEditorFMForm::on_opSeqNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMOperatorSequence(instNum_, getOperatorSequenceParameter(), arg1);
		setInstrumentOperatorSequenceParameters();
		emit operatorSequenceNumberChanged();
		emit modified();
	}

	onOperatorSequenceNumberChanged();
}

//--- Arpeggio
void InstrumentEditorFMForm::setInstrumentArpeggioParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->arpNumSpinBox->setValue(instFM->getArpeggioNumber());
	ui->arpEditor->clearData();
	for (auto& com : instFM->getArpeggioSequence()) {
		ui->arpEditor->addSequenceCommand(com.type);
	}
	for (auto& l : instFM->getArpeggioLoops()) {
		ui->arpEditor->addLoop(l.begin, l.end, l.times);
	}
	ui->arpEditor->setRelease(convertReleaseTypeForUI(instFM->getArpeggioRelease().type),
							  instFM->getArpeggioRelease().begin);
	ui->arpTypeComboBox->setCurrentIndex(instFM->getArpeggioType());
	if (instFM->getArpeggioEnabled()) {
		ui->arpEditGroupBox->setChecked(true);
		onArpeggioNumberChanged();
	}
	else {
		ui->arpEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorFMForm::onArpeggioNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getArpeggioFMUsers(ui->arpNumSpinBox->value());
	for (auto& n : users) {
		str += (QString("%1").arg(n, 2, 16, QChar('0')).toUpper() + ",");
	}
	str.chop(1);

	ui->arpUsersLineEdit->setText(str);
}

void InstrumentEditorFMForm::onArpeggioParameterChanged(int tnNum)
{
	if (ui->arpNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentArpeggioParameters();
	}
}

void InstrumentEditorFMForm::onArpeggioTypeChanged(int index)
{
	Q_UNUSED(index)

	if (!isIgnoreEvent_) {
		bt_.lock()->setArpeggioFMType(ui->arpNumSpinBox->value(),
									  ui->arpTypeComboBox->currentData(Qt::UserRole).toInt());
		emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
		emit modified();
	}

	// Update labels
	if (index == 1) {
		QString tn[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 12; ++j) {
				ui->arpEditor->setLabel(i * 12 + j, tn[j] + QString::number(i));
			}
		}
	}
	else {
		for (int i = 0; i < 96; ++i) {
			int d = i - 48;
			auto text = QString::number(d);
			if (d > 0) text = "+" + text;
			ui->arpEditor->setLabel(i, text);
		}
	}
}

void InstrumentEditorFMForm::on_arpEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMArpeggioEnabled(instNum_, arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

void InstrumentEditorFMForm::on_arpNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMArpeggio(instNum_, arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

//--- Pitch
void InstrumentEditorFMForm::setInstrumentPitchParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->ptNumSpinBox->setValue(instFM->getPitchNumber());
	ui->ptEditor->clearData();
	for (auto& com : instFM->getPitchSequence()) {
		ui->ptEditor->addSequenceCommand(com.type);
	}
	for (auto& l : instFM->getPitchLoops()) {
		ui->ptEditor->addLoop(l.begin, l.end, l.times);
	}
	ui->ptEditor->setRelease(convertReleaseTypeForUI(instFM->getPitchRelease().type),
							 instFM->getPitchRelease().begin);
	for (int i = 0; i < ui->ptTypeComboBox->count(); ++i) {
		if (ui->ptTypeComboBox->itemData(i, Qt::UserRole).toInt() == instFM->getPitchType()) {
			ui->ptTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instFM->getPitchEnabled()) {
		ui->ptEditGroupBox->setChecked(true);
		onPitchNumberChanged();
	}
	else {
		ui->ptEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorFMForm::onPitchNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getPitchFMUsers(ui->ptNumSpinBox->value());
	for (auto& n : users) {
		str += (QString("%1").arg(n, 2, 16, QChar('0')).toUpper() + ",");
	}
	str.chop(1);

	ui->ptUsersLineEdit->setText(str);
}

void InstrumentEditorFMForm::onPitchParameterChanged(int tnNum)
{
	if (ui->ptNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentPitchParameters();
	}
}

void InstrumentEditorFMForm::onPitchTypeChanged(int index)
{
	Q_UNUSED(index)

	if (!isIgnoreEvent_) {
		bt_.lock()->setPitchFMType(ui->ptNumSpinBox->value(),
								   ui->ptTypeComboBox->currentData(Qt::UserRole).toInt());
		emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
		emit modified();
	}
}

void InstrumentEditorFMForm::on_ptEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMPitchEnabled(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}

void InstrumentEditorFMForm::on_ptNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMPitch(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}

//========== Else ==========//
/********** Slots **********/
void InstrumentEditorFMForm::on_envResetCheckBox_stateChanged(int arg1)
{
	if (arg1 == Qt::Checked) {
		bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, true);
	}
	else {
		bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, false);
	}
	emit modified();
}
