/*
 * Copyright (C) 2018-2021 Rerrah
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
#include <set>
#include "gui/event_guard.hpp"
#include "gui/jam_layout.hpp"
#include "gui/instrument_editor/instrument_editor_utils.hpp"
#include "gui/gui_utils.hpp"

InstrumentEditorFMForm::InstrumentEditorFMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorFMForm),
	instNum_(num),
	isIgnoreEvent_(false)
{
	ui->setupUi(this);

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
			default:	throw std::invalid_argument("Unexpected FMOperatorParameter.");
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
			default:	throw std::invalid_argument("Unexpected FMOperatorParameter.");
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
			default:	throw std::invalid_argument("Unexpected FMOperatorParameter.");
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
			default:	throw std::invalid_argument("Unexpected FMOperatorParameter.");
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

	ui->lfoFreqSlider->setText(tr("Freq"));
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
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParameter::Count, v);
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

	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->addOperatorSequenceFMSequenceData(param, ui->opSeqNumSpinBox->value(), row);
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->removeOperatorSequenceFMSequenceData(param, ui->opSeqNumSpinBox->value());
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->setOperatorSequenceFMSequenceData(param, ui->opSeqNumSpinBox->value(), col, row);
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->addOperatorSequenceFMLoop(param, ui->opSeqNumSpinBox->value(), loop);
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->removeOperatorSequenceFMLoop(
						param, ui->opSeqNumSpinBox->value(), begin, end);
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->clearOperatorSequenceFMLoops(param, ui->opSeqNumSpinBox->value());
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->changeOperatorSequenceFMLoop(
						param, ui->opSeqNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->opSeqEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param = getOperatorSequenceParameter();
			bt_.lock()->setOperatorSequenceFMRelease(param, ui->opSeqNumSpinBox->value(), release);
			emit operatorSequenceParameterChanged(param, ui->opSeqNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->opSeqTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onOperatorSequenceTypeChanged);

	//========== Arpeggio ==========//
	ui->arpOpComboBox->addItem(tr("All"), static_cast<int>(FMOperatorType::All));
	ui->arpOpComboBox->addItem("Op1", static_cast<int>(FMOperatorType::Op1));
	ui->arpOpComboBox->addItem("Op2", static_cast<int>(FMOperatorType::Op2));
	ui->arpOpComboBox->addItem("Op3", static_cast<int>(FMOperatorType::Op3));
	ui->arpOpComboBox->addItem("Op4", static_cast<int>(FMOperatorType::Op4));

	ui->arpTypeComboBox->addItem(tr("Absolute"), static_cast<int>(SequenceType::AbsoluteSequence));
	ui->arpTypeComboBox->addItem(tr("Fixed"), static_cast<int>(SequenceType::FixedSequence));
	ui->arpTypeComboBox->addItem(tr("Relative"), static_cast<int>(SequenceType::RelativeSequence));

	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioFMSequenceData(ui->arpNumSpinBox->value(), row);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioFMSequenceData(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioFMSequenceData(ui->arpNumSpinBox->value(), col, row);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioFMLoop(ui->arpNumSpinBox->value(), loop);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioFMLoop(ui->arpNumSpinBox->value(), begin, end);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearArpeggioFMLoops(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changeArpeggioFMLoop(ui->arpNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioFMRelease(ui->arpNumSpinBox->value(), release);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->arpTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onArpeggioTypeChanged);
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->arpOpComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onArpeggioOperatorChanged);

	//========== Pitch ==========//
	ui->ptOpComboBox->addItem(tr("All"), static_cast<int>(FMOperatorType::All));
	ui->ptOpComboBox->addItem("Op1", static_cast<int>(FMOperatorType::Op1));
	ui->ptOpComboBox->addItem("Op2", static_cast<int>(FMOperatorType::Op2));
	ui->ptOpComboBox->addItem("Op3", static_cast<int>(FMOperatorType::Op3));
	ui->ptOpComboBox->addItem("Op4", static_cast<int>(FMOperatorType::Op4));

	ui->ptEditor->setMaximumDisplayedRowCount(15);
	ui->ptEditor->setDefaultRow(127);
	ui->ptEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 255; ++i) {
		ui->ptEditor->AddRow(QString::asprintf("%+d", i - 127), false);
	}
	ui->ptEditor->autoFitLabelWidth();
	ui->ptEditor->setUpperRow(134);
	ui->ptEditor->setMMLDisplay0As(-127);

	ui->ptTypeComboBox->addItem(tr("Absolute"), static_cast<int>(SequenceType::AbsoluteSequence));
	ui->ptTypeComboBox->addItem(tr("Relative"), static_cast<int>(SequenceType::RelativeSequence));

	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPitchFMSequenceData(ui->ptNumSpinBox->value(), row);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchFMSequenceData(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchFMSequenceData(ui->ptNumSpinBox->value(), col, row);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPitchFMLoop(ui->ptNumSpinBox->value(), loop);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchFMLoop(ui->ptNumSpinBox->value(), begin, end);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearPitchFMLoops(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changePitchFMLoop(ui->ptNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchFMRelease(ui->ptNumSpinBox->value(), release);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->ptTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onPitchTypeChanged);
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->ptOpComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorFMForm::onPitchOperatorChanged);

	//========== Pan ==========//
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPanFMSequenceData(ui->panNumSpinBox->value(), row);
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePanFMSequenceData(ui->panNumSpinBox->value());
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPanFMSequenceData(ui->panNumSpinBox->value(), col, row);
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPanFMLoop(ui->panNumSpinBox->value(), loop);
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePanFMLoop(ui->panNumSpinBox->value(), begin, end);
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearPanFMLoops(ui->panNumSpinBox->value());
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changePanFMLoop(ui->panNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->panEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPanFMRelease(ui->panNumSpinBox->value(), release);
			emit panParameterChanged(ui->panNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Others ==========//
	QObject::connect(ui->envResetCheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, FMOperatorType::All, (state == Qt::Checked));
			emit modified();
		}
	});
	QObject::connect(ui->envResetOp1CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, FMOperatorType::Op1, (state == Qt::Checked));
			emit modified();
		}
	});
	QObject::connect(ui->envResetOp2CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, FMOperatorType::Op2, (state == Qt::Checked));
			emit modified();
		}
	});
	QObject::connect(ui->envResetOp3CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, FMOperatorType::Op3, (state == Qt::Checked));
			emit modified();
		}
	});
	QObject::connect(ui->envResetOp4CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, FMOperatorType::Op4, (state == Qt::Checked));
			emit modified();
		}
	});
}

InstrumentEditorFMForm::~InstrumentEditorFMForm()
{
	delete ui;
}

void InstrumentEditorFMForm::setInstrumentNumber(int num)
{
	instNum_ = num;
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
	for (const auto& texts : config.lock()->getFMEnvelopeTexts()) {
		names.push_back(gui_utils::utf8ToQString(texts.name));
	}
	ui->op1Table->setEnvelopeSetNames(names);
	ui->op2Table->setEnvelopeSetNames(names);
	ui->op3Table->setEnvelopeSetNames(names);
	ui->op4Table->setEnvelopeSetNames(names);

	updateConfigurationForDisplay();
}

void InstrumentEditorFMForm::updateConfigurationForDisplay()
{
	ui->op1Table->setDTDisplayType(config_.lock()->getShowFMDetuneAsSigned());
	ui->op2Table->setDTDisplayType(config_.lock()->getShowFMDetuneAsSigned());
	ui->op3Table->setDTDisplayType(config_.lock()->getShowFMDetuneAsSigned());
	ui->op4Table->setDTDisplayType(config_.lock()->getShowFMDetuneAsSigned());
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
	ui->panEditor->setColorPalette(palette);
}

void InstrumentEditorFMForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	auto name = gui_utils::utf8ToQString(bt_.lock()->getInstrument(instNum_)->getName());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentEnvelopeParameters();
	setInstrumentLFOParameters();
	setInstrumentOperatorSequenceParameters();
	setInstrumentArpeggioParameters();
	setInstrumentPitchParameters();
	setInstrumentPanParameters();

	setInstrumentEnvelopeResetParameters();
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorFMForm::keyPressEvent(QKeyEvent *event)
{
	// General keys
	switch (event->key()) {
	case Qt::Key_Escape:
		close();
		break;
	default:
		// For jam key on
		if (!event->isAutoRepeat()) {
			// Musical keyboard
			Qt::Key qtKey = static_cast<Qt::Key>(event->key());
			try {
				JamKey jk = getJamKeyFromLayoutMapping(qtKey, config_);
				emit jamKeyOnEvent(jk);
			} catch (std::invalid_argument&) {}
		}
		break;
	}
}

// MUST DIRECT CONNECTION
void InstrumentEditorFMForm::keyReleaseEvent(QKeyEvent *event)
{
	// For jam key off
	if (!event->isAutoRepeat()) {
		Qt::Key qtKey = static_cast<Qt::Key>(event->key());
		try {
			JamKey jk = getJamKeyFromLayoutMapping(qtKey, config_);
			emit jamKeyOffEvent(jk);
		} catch (std::invalid_argument&) {}
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
	ui->op1Table->setGroupEnabled(instFM->getOperatorEnabled(0));
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
	ui->op2Table->setGroupEnabled(instFM->getOperatorEnabled(1));
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
	ui->op3Table->setGroupEnabled(instFM->getOperatorEnabled(2));
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
	ui->op4Table->setGroupEnabled(instFM->getOperatorEnabled(3));
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

void InstrumentEditorFMForm::setInstrumentEnvelopeParameters(int envTypeNum, QString data)
{
	QStringList digits;
	QRegularExpression re(R"((-?\d+))");
	auto it = re.globalMatch(data);
	while (it.hasNext()) {
		auto match = it.next();
		digits.append(match.captured(1));
	}

	std::vector<FMEnvelopeTextType> set
			= config_.lock()->getFMEnvelopeTexts().at(static_cast<size_t>(envTypeNum)).texts;
	if (static_cast<int>(set.size()) != digits.size()) {
		auto name = config_.lock()->getFMEnvelopeTexts().at(static_cast<size_t>(envTypeNum)).name;
		QMessageBox::critical(this, tr("Error"),
							  tr("Did not match the clipboard text format with %1.")
							  .arg(gui_utils::utf8ToQString(name)));
		return;
	}

	auto rangeCheck = [](int v, int min, int max) -> int {
		if (v < min || max < v) throw std::out_of_range("");
		else return v;
	};
	auto rangeCheckDT = [rangeCheck](int v) -> int {
		switch (v) {
		case -3:	v = 7;	break;
		case -2:	v = 6;	break;
		case -1:	v = 5;	break;
		default:			break;
		}
		return rangeCheck(v, 0, 7);
	};
	try {
		for (int i = 0; i < digits.size(); ++i) {
			int d = digits[i].toInt();
			switch (set[static_cast<size_t>(i)]) {
			case FMEnvelopeTextType::Skip:	break;
			case FMEnvelopeTextType::AL:	ui->alSlider->setValue(rangeCheck(d, 0, 7));	break;
			case FMEnvelopeTextType::FB:	ui->fbSlider->setValue(rangeCheck(d, 0, 7));	break;
			case FMEnvelopeTextType::AR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::AR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::DR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::DR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::SR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::SR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::RR1:	ui->op1Table->setValue(Ui::FMOperatorParameter::RR, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::SL1:	ui->op1Table->setValue(Ui::FMOperatorParameter::SL, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::TL1:	ui->op1Table->setValue(Ui::FMOperatorParameter::TL, rangeCheck(d, 0, 127));	break;
			case FMEnvelopeTextType::KS1:	ui->op1Table->setValue(Ui::FMOperatorParameter::KS, rangeCheck(d, 0, 3));	break;
			case FMEnvelopeTextType::ML1:	ui->op1Table->setValue(Ui::FMOperatorParameter::ML, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::DT1:	ui->op1Table->setValue(Ui::FMOperatorParameter::DT, rangeCheckDT(d));	break;
			case FMEnvelopeTextType::AR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::AR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::DR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::DR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::SR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::SR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::RR2:	ui->op2Table->setValue(Ui::FMOperatorParameter::RR, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::SL2:	ui->op2Table->setValue(Ui::FMOperatorParameter::SL, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::TL2:	ui->op2Table->setValue(Ui::FMOperatorParameter::TL, rangeCheck(d, 0, 127));	break;
			case FMEnvelopeTextType::KS2:	ui->op2Table->setValue(Ui::FMOperatorParameter::KS, rangeCheck(d, 0, 3));	break;
			case FMEnvelopeTextType::ML2:	ui->op2Table->setValue(Ui::FMOperatorParameter::ML, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::DT2:	ui->op2Table->setValue(Ui::FMOperatorParameter::DT, rangeCheckDT(d));	break;
			case FMEnvelopeTextType::AR3:	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::DR3:	ui->op3Table->setValue(Ui::FMOperatorParameter::DR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::SR3:	ui->op3Table->setValue(Ui::FMOperatorParameter::SR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::RR3:	ui->op3Table->setValue(Ui::FMOperatorParameter::RR, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::SL3:	ui->op3Table->setValue(Ui::FMOperatorParameter::SL, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::TL3:	ui->op3Table->setValue(Ui::FMOperatorParameter::TL, rangeCheck(d, 0, 127));	break;
			case FMEnvelopeTextType::KS3:	ui->op3Table->setValue(Ui::FMOperatorParameter::KS, rangeCheck(d, 0, 4));	break;
			case FMEnvelopeTextType::ML3:	ui->op3Table->setValue(Ui::FMOperatorParameter::ML, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::DT3:	ui->op3Table->setValue(Ui::FMOperatorParameter::DT, rangeCheckDT(d));	break;
			case FMEnvelopeTextType::AR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::DR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::DR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::SR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::SR, rangeCheck(d, 0, 31));	break;
			case FMEnvelopeTextType::RR4:	ui->op4Table->setValue(Ui::FMOperatorParameter::RR, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::SL4:	ui->op4Table->setValue(Ui::FMOperatorParameter::SL, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::TL4:	ui->op4Table->setValue(Ui::FMOperatorParameter::TL, rangeCheck(d, 0, 127));	break;
			case FMEnvelopeTextType::KS4:	ui->op4Table->setValue(Ui::FMOperatorParameter::KS, rangeCheck(d, 0, 4));	break;
			case FMEnvelopeTextType::ML4:	ui->op4Table->setValue(Ui::FMOperatorParameter::ML, rangeCheck(d, 0, 15));	break;
			case FMEnvelopeTextType::DT4:	ui->op4Table->setValue(Ui::FMOperatorParameter::DT, rangeCheckDT(d));	break;
			}
		}
	}
	catch (...) {
		auto name = config_.lock()->getFMEnvelopeTexts().at(static_cast<size_t>(envTypeNum)).name;
		QMessageBox::critical(this, tr("Error"),
							  tr("Did not match the clipboard text format with %1.")
							  .arg(gui_utils::utf8ToQString(name)));
		return;
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
									   .arg(ui->fbSlider->value()).arg(ui->alSlider->value())
									   .arg(ui->op1Table->toString(), ui->op2Table->toString(),
											ui->op3Table->toString(), ui->op4Table->toString()));
}

void InstrumentEditorFMForm::pasteEnvelope()
{
	QString data = QApplication::clipboard()->text().remove("FM_ENVELOPE:");
	setInstrumentEnvelopeParameters(data);
}

void InstrumentEditorFMForm::pasteEnvelopeFrom(int typenum)
{
	setInstrumentEnvelopeParameters(typenum, QApplication::clipboard()->text());
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
	std::vector<FMEnvelopeText> textsSet = config_.lock()->getFMEnvelopeTexts();
	for (size_t i = 0; i < textsSet.size(); ++i) {
		QAction* act = pasteFrom->addAction(gui_utils::utf8ToQString(textsSet[i].name));
		act->setData(static_cast<int>(i));
	}
	QObject::connect(pasteFrom, &QMenu::triggered,
					 this, [&](QAction* action) { pasteEnvelopeFrom(action->data().toInt()); });

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
	std::multiset<int> users = bt_.lock()->getEnvelopeFMUsers(ui->envNumSpinBox->value());
	ui->envUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
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
	ui->lfoStartSpinBox->setValue(instFM->getLFOParameter(FMLFOParameter::Count));
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
			.arg(ui->lfoFreqSlider->value())
			.arg(ui->pmsSlider->value())
			.arg(ui->amsSlider->value())
			.arg(ui->lfoStartSpinBox->value())
			.arg(ui->amOp1CheckBox->isChecked() ? 1 : 0)
			.arg(ui->amOp2CheckBox->isChecked() ? 1 : 0)
			.arg(ui->amOp3CheckBox->isChecked() ? 1 : 0)
			.arg(ui->amOp4CheckBox->isChecked() ? 1 : 0);
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
	std::multiset<int> users = bt_.lock()->getLFOFMUsers(ui->lfoNumSpinBox->value());
	ui->lfoUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
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
	for (auto& unit : instFM->getOperatorSequenceSequence(param)) {
		ui->opSeqEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instFM->getOperatorSequenceLoopRoot(param).getAllLoops()) {
		ui->opSeqEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->opSeqEditor->setRelease(instFM->getOperatorSequenceRelease(param));
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
			ui->opSeqEditor->AddRow(QString::number(i), false);
		}
		ui->opSeqEditor->autoFitLabelWidth();
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
	std::multiset<int> users = bt_.lock()->getOperatorSequenceFMUsers(getOperatorSequenceParameter(), ui->opSeqNumSpinBox->value());
	ui->opSeqUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
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

	if (!isIgnoreEvent_) setInstrumentOperatorSequenceParameters();
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
FMOperatorType InstrumentEditorFMForm::getArpeggioOperator() const
{
	return static_cast<FMOperatorType>(ui->arpOpComboBox->currentData(Qt::UserRole).toInt());
}

void InstrumentEditorFMForm::setInstrumentArpeggioParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	FMOperatorType param = getArpeggioOperator();

	ui->arpNumSpinBox->setValue(instFM->getArpeggioNumber(param));
	ui->arpEditor->clearData();
	for (auto& unit : instFM->getArpeggioSequence(param)) {
		ui->arpEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instFM->getArpeggioLoopRoot(param).getAllLoops()) {
		ui->arpEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->arpEditor->setRelease(instFM->getArpeggioRelease(param));
	for (int i = 0; i < ui->arpTypeComboBox->count(); ++i) {
		if (instFM->getArpeggioType(param) == static_cast<SequenceType>(ui->arpTypeComboBox->itemData(i).toInt())) {
			ui->arpTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instFM->getArpeggioEnabled(param)) {
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
	std::multiset<int> users = bt_.lock()->getArpeggioFMUsers(ui->arpNumSpinBox->value());
	ui->arpUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorFMForm::onArpeggioParameterChanged(int tnNum)
{
	if (ui->arpNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentArpeggioParameters();
	}
}

void InstrumentEditorFMForm::onArpeggioOperatorChanged(int op)
{
	Q_UNUSED(op)

	if (!isIgnoreEvent_) setInstrumentArpeggioParameters();
}

void InstrumentEditorFMForm::onArpeggioTypeChanged(int index)
{
	Q_UNUSED(index)

	auto type = static_cast<SequenceType>(ui->arpTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setArpeggioFMType(ui->arpNumSpinBox->value(), type);
		emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->arpEditor->setSequenceType(type);
}

void InstrumentEditorFMForm::on_arpEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMArpeggioEnabled(instNum_, getArpeggioOperator(), arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

void InstrumentEditorFMForm::on_arpNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMArpeggio(instNum_, getArpeggioOperator(), arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

//--- Pitch
FMOperatorType InstrumentEditorFMForm::getPitchOperator() const
{
	return static_cast<FMOperatorType>(ui->ptOpComboBox->currentData(Qt::UserRole).toInt());
}

void InstrumentEditorFMForm::setInstrumentPitchParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	FMOperatorType param = getPitchOperator();

	ui->ptNumSpinBox->setValue(instFM->getPitchNumber(param));
	ui->ptEditor->clearData();
	for (auto& unit : instFM->getPitchSequence(param)) {
		ui->ptEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instFM->getPitchLoopRoot(param).getAllLoops()) {
		ui->ptEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->ptEditor->setRelease(instFM->getPitchRelease(param));
	for (int i = 0; i < ui->ptTypeComboBox->count(); ++i) {
		if (instFM->getPitchType(param) == static_cast<SequenceType>(ui->ptTypeComboBox->itemData(i).toInt())) {
			ui->ptTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instFM->getPitchEnabled(param)) {
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
	std::multiset<int> users = bt_.lock()->getPitchFMUsers(ui->ptNumSpinBox->value());
	ui->ptUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorFMForm::onPitchParameterChanged(int ptNum)
{
	if (ui->ptNumSpinBox->value() == ptNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentPitchParameters();
	}
}

void InstrumentEditorFMForm::onPitchOperatorChanged(int op)
{
	Q_UNUSED(op)

	if (!isIgnoreEvent_) setInstrumentPitchParameters();
}

void InstrumentEditorFMForm::onPitchTypeChanged(int index)
{
	Q_UNUSED(index)

	auto type = static_cast<SequenceType>(ui->ptTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setPitchFMType(ui->ptNumSpinBox->value(), type);
		emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->ptEditor->setSequenceType(type);
}

void InstrumentEditorFMForm::on_ptEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMPitchEnabled(instNum_, getPitchOperator(), arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}

void InstrumentEditorFMForm::on_ptNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMPitch(instNum_, getPitchOperator(), arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}

//--- Pan
void InstrumentEditorFMForm::setInstrumentPanParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->panNumSpinBox->setValue(instFM->getPanNumber());
	ui->panEditor->clearData();
	for (auto& unit : instFM->getPanSequence()) {
		ui->panEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instFM->getPanLoopRoot().getAllLoops()) {
		ui->panEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->panEditor->setRelease(instFM->getPanRelease());

	if (instFM->getPanEnabled()) {
		ui->panEditGroupBox->setChecked(true);
		onPanNumberChanged();
	}
	else {
		ui->panEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorFMForm::onPanNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getPanFMUsers(ui->panNumSpinBox->value());
	ui->panUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorFMForm::onPanParameterChanged(int panNum)
{
	if (ui->panNumSpinBox->value() == panNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentPanParameters();
	}
}

void InstrumentEditorFMForm::on_panEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMPanEnabled(instNum_, arg1);
		setInstrumentPanParameters();
		emit panNumberChanged();
		emit modified();
	}

	onPanNumberChanged();
}

void InstrumentEditorFMForm::on_panNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMPan(instNum_, arg1);
		setInstrumentPanParameters();
		emit panNumberChanged();
		emit modified();
	}

	onPanNumberChanged();
}

//========== Others ==========//
void InstrumentEditorFMForm::setInstrumentEnvelopeResetParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->envResetCheckBox->setChecked(instFM->getEnvelopeResetEnabled(FMOperatorType::All));
	ui->envResetOp1CheckBox->setChecked(instFM->getEnvelopeResetEnabled(FMOperatorType::Op1));
	ui->envResetOp2CheckBox->setChecked(instFM->getEnvelopeResetEnabled(FMOperatorType::Op2));
	ui->envResetOp3CheckBox->setChecked(instFM->getEnvelopeResetEnabled(FMOperatorType::Op3));
	ui->envResetOp4CheckBox->setChecked(instFM->getEnvelopeResetEnabled(FMOperatorType::Op4));
}
