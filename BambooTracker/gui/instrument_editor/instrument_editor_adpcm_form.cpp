#include "instrument_editor_adpcm_form.hpp"
#include "ui_instrument_editor_adpcm_form.h"
#include "instrument.hpp"
#include "gui/event_guard.hpp"
#include "gui/jam_layout.hpp"

InstrumentEditorADPCMForm::InstrumentEditorADPCMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorADPCMForm),
	instNum_(num)
{
	ui->setupUi(this);

//	//========== Waveform ==========//
//	ui->waveEditor->setMaximumDisplayedRowCount(7);
//	ui->waveEditor->setDefaultRow(0);
//	ui->waveEditor->AddRow(tr("Sq"), false);
//	ui->waveEditor->AddRow(tr("Tri"), false);
//	ui->waveEditor->AddRow(tr("Saw"), false);
//	ui->waveEditor->AddRow(tr("InvSaw"), false);
//	ui->waveEditor->AddRow(tr("SMTri"), false);
//	ui->waveEditor->AddRow(tr("SMSaw"), false);
//	ui->waveEditor->AddRow(tr("SMInvSaw"), false);
//	ui->waveEditor->autoFitLabelWidth();

//	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
//					 this, [&](int row, int col) {
//		if (!isIgnoreEvent_) {
//			if (isModulatedWaveformADPCM(row)) setWaveformSequenceColumn(col);	// Set square-mask frequency
//			bt_.lock()->addWaveformADPCMSequenceCommand(
//						ui->waveNumSpinBox->value(), row, ui->waveEditor->getSequenceDataAt(col));
//			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
//					 this, [&]() {
//		if (!isIgnoreEvent_) {
//			bt_.lock()->removeWaveformADPCMSequenceCommand(ui->waveNumSpinBox->value());
//			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
//					 this, [&](int row, int col) {
//		if (!isIgnoreEvent_) {
//			if (isModulatedWaveformADPCM(row)) setWaveformSequenceColumn(col);	// Set square-mask frequency
//			bt_.lock()->setWaveformADPCMSequenceCommand(
//						ui->waveNumSpinBox->value(), col, row, ui->waveEditor->getSequenceDataAt(col));
//			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::loopChanged,
//					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
//		if (!isIgnoreEvent_) {
//			bt_.lock()->setWaveformADPCMLoops(
//						ui->waveNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
//			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
//					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
//		if (!isIgnoreEvent_) {
//			ReleaseType t = convertReleaseTypeForData(type);
//			bt_.lock()->setWaveformADPCMRelease(ui->waveNumSpinBox->value(), t, point);
//			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});

//	//========== Envelope ==========//
//	ui->envEditor->setMaximumDisplayedRowCount(16);
//	ui->envEditor->setDefaultRow(15);
//	for (int i = 0; i < 16; ++i) {
//		ui->envEditor->AddRow(QString::number(i), false);
//	}
//	for (int i = 0; i < 8; ++i) {
//		ui->envEditor->AddRow(tr("HEnv %1").arg(i), false);
//	}
//	ui->envEditor->autoFitLabelWidth();
//	ui->envEditor->setMultipleReleaseState(true);
//	ui->envEditor->setPermittedReleaseTypes(
//				VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE
//				| VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE
//				| VisualizedInstrumentMacroEditor::ReleaseType::FIXED);

//	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
//					 this, [&](int row, int col) {
//		if (!isIgnoreEvent_) {
//			if (row >= 16) setEnvelopeSequenceColumn(col);	// Set hard frequency
//			bt_.lock()->addEnvelopeADPCMSequenceCommand(
//						ui->envNumSpinBox->value(), row, ui->envEditor->getSequenceDataAt(col));
//			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
//					 this, [&]() {
//		if (!isIgnoreEvent_) {
//			bt_.lock()->removeEnvelopeADPCMSequenceCommand(ui->envNumSpinBox->value());
//			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
//					 this, [&](int row, int col) {
//		if (!isIgnoreEvent_) {
//			if (row >= 16) setEnvelopeSequenceColumn(col);	// Set hard frequency
//			bt_.lock()->setEnvelopeADPCMSequenceCommand(
//						ui->envNumSpinBox->value(), col, row, ui->envEditor->getSequenceDataAt(col));
//			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopChanged,
//					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
//		if (!isIgnoreEvent_) {
//			bt_.lock()->setEnvelopeADPCMLoops(
//						ui->envNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
//			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});
//	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
//					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
//		if (!isIgnoreEvent_) {
//			ReleaseType t = convertReleaseTypeForData(type);
//			bt_.lock()->setEnvelopeADPCMRelease(ui->envNumSpinBox->value(), t, point);
//			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
//			emit modified();
//		}
//	});

	//========== Arpeggio ==========//
	ui->arpTypeComboBox->addItem(tr("Absolute"), VisualizedInstrumentMacroEditor::SequenceType::Absolute);
	ui->arpTypeComboBox->addItem(tr("Fixed"), VisualizedInstrumentMacroEditor::SequenceType::Fixed);
	ui->arpTypeComboBox->addItem(tr("Relative"), VisualizedInstrumentMacroEditor::SequenceType::Relative);

	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioADPCMSequenceCommand(
						ui->arpNumSpinBox->value(), row, ui->arpEditor->getSequenceDataAt(col));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioADPCMSequenceCommand(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioADPCMSequenceCommand(
						ui->arpNumSpinBox->value(), col, row, ui->arpEditor->getSequenceDataAt(col));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioADPCMLoops(
						ui->arpNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setArpeggioADPCMRelease(ui->arpNumSpinBox->value(), t, point);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->arpTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorADPCMForm::onArpeggioTypeChanged);

	//========== Pitch ==========//
	ui->ptEditor->setMaximumDisplayedRowCount(15);
	ui->ptEditor->setDefaultRow(127);
	ui->ptEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 255; ++i) {
		ui->ptEditor->AddRow(QString::asprintf("%+d", i - 127), false);
	}
	ui->ptEditor->autoFitLabelWidth();
	ui->ptEditor->setUpperRow(134);
	ui->ptEditor->setMMLDisplay0As(-127);

	ui->ptTypeComboBox->addItem(tr("Absolute"), VisualizedInstrumentMacroEditor::SequenceType::Absolute);
	ui->ptTypeComboBox->addItem(tr("Relative"), VisualizedInstrumentMacroEditor::SequenceType::Relative);

	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPitchADPCMSequenceCommand(
						ui->ptNumSpinBox->value(), row, ui->ptEditor->getSequenceDataAt(col));
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchADPCMSequenceCommand(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchADPCMSequenceCommand(
						ui->ptNumSpinBox->value(), col, row, ui->ptEditor->getSequenceDataAt(col));
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchADPCMLoops(
						ui->ptNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setPitchADPCMRelease(ui->ptNumSpinBox->value(), t, point);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->ptTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorADPCMForm::onPitchTypeChanged);
}

InstrumentEditorADPCMForm::~InstrumentEditorADPCMForm()
{
	delete ui;
}

int InstrumentEditorADPCMForm::getInstrumentNumber() const
{
	return instNum_;
}

void InstrumentEditorADPCMForm::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
	updateInstrumentParameters();
}

void InstrumentEditorADPCMForm::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
}

void InstrumentEditorADPCMForm::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
//	ui->waveEditor->setColorPalette(palette);
//	ui->envEditor->setColorPalette(palette);
	ui->arpEditor->setColorPalette(palette);
	ui->ptEditor->setColorPalette(palette);
}

SequenceType InstrumentEditorADPCMForm::convertSequenceTypeForData(VisualizedInstrumentMacroEditor::SequenceType type)
{
	switch (type) {
	case VisualizedInstrumentMacroEditor::SequenceType::NoType:
		return SequenceType::NO_SEQUENCE_TYPE;
	case VisualizedInstrumentMacroEditor::SequenceType::Fixed:
		return SequenceType::FIXED_SEQUENCE;
	case VisualizedInstrumentMacroEditor::SequenceType::Absolute:
		return SequenceType::ABSOLUTE_SEQUENCE;
	case VisualizedInstrumentMacroEditor::SequenceType::Relative:
		return SequenceType::RELATIVE_SEQUENCE;
	default:
		throw std::invalid_argument("Unexpected SequenceType.");
	}
}

VisualizedInstrumentMacroEditor::SequenceType InstrumentEditorADPCMForm::convertSequenceTypeForUI(SequenceType type)
{
	switch (type) {
	case SequenceType::NO_SEQUENCE_TYPE:
		return VisualizedInstrumentMacroEditor::SequenceType::NoType;
	case SequenceType::FIXED_SEQUENCE:
		return VisualizedInstrumentMacroEditor::SequenceType::Fixed;
	case SequenceType::ABSOLUTE_SEQUENCE:
		return VisualizedInstrumentMacroEditor::SequenceType::Absolute;
	case SequenceType::RELATIVE_SEQUENCE:
		return VisualizedInstrumentMacroEditor::SequenceType::Relative;
	default:
		throw std::invalid_argument("Unexpected SequenceType.");
	}
}

ReleaseType InstrumentEditorADPCMForm::convertReleaseTypeForData(VisualizedInstrumentMacroEditor::ReleaseType type)
{
	switch (type) {
	case VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE:
		return ReleaseType::NoRelease;
	case VisualizedInstrumentMacroEditor::ReleaseType::FIXED:
		return ReleaseType::FixedRelease;
	case VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE:
		return ReleaseType::AbsoluteRelease;
	case VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE:
		return ReleaseType::RelativeRelease;
	default:
		throw std::invalid_argument("Unexpected ReleaseType.");
	}
}

VisualizedInstrumentMacroEditor::ReleaseType InstrumentEditorADPCMForm::convertReleaseTypeForUI(ReleaseType type)
{
	switch (type) {
	case ReleaseType::NoRelease:
		return VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE;
	case ReleaseType::FixedRelease:
		return VisualizedInstrumentMacroEditor::ReleaseType::FIXED;
	case ReleaseType::AbsoluteRelease:
		return VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE;
	case ReleaseType::RelativeRelease:
		return VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE;
	default:
		throw std::invalid_argument("Unexpected ReleaseType.");
	}
}

void InstrumentEditorADPCMForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());
	auto name = QString::fromUtf8(instADPCM->getName().c_str(), static_cast<int>(instADPCM->getName().length()));
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

//	setInstrumentWaveformParameters();
//	setInstrumentEnvelopeParameters();
	setInstrumentArpeggioParameters();
	setInstrumentPitchParameters();
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorADPCMForm::keyPressEvent(QKeyEvent *event)
{
	// For jam key on

	// Check keys
	QString seq = QKeySequence(static_cast<int>(event->modifiers()) | event->key()).toString();
	if (seq == QKeySequence(
				QString::fromUtf8(config_.lock()->getOctaveUpKey().c_str(),
								  static_cast<int>(config_.lock()->getOctaveUpKey().length()))).toString()) {
		emit octaveChanged(true);
		return;
	}
	else if (seq == QKeySequence(
				 QString::fromUtf8(config_.lock()->getOctaveDownKey().c_str(),
								   static_cast<int>(config_.lock()->getOctaveDownKey().length()))).toString()) {
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
void InstrumentEditorADPCMForm::keyReleaseEvent(QKeyEvent *event)
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

//--- Waveform
void InstrumentEditorADPCMForm::setInstrumentWaveformParameters()
{
//	Ui::EventGuard ev(isIgnoreEvent_);

//	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
//	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

//	ui->waveNumSpinBox->setValue(instADPCM->getWaveformNumber());
//	ui->waveEditor->clearData();
//	for (auto& com : instADPCM->getWaveformSequence()) {
//		QString str("");
//		if (isModulatedWaveformADPCM(com.type)) {
//			if (CommandSequenceUnit::checkDataType(com.data) == CommandSequenceUnit::RATIO) {
//				auto ratio = CommandSequenceUnit::data2ratio(com.data);
//				str = QString("%1/%2").arg(ratio.first).arg(ratio.second);
//			}
//			else {
//				str = QString::number(com.data);
//			}
//		}
//		ui->waveEditor->addSequenceCommand(com.type, str, com.data);
//	}
//	for (auto& l : instADPCM->getWaveformLoops()) {
//		ui->waveEditor->addLoop(l.begin, l.end, l.times);
//	}
//	ui->waveEditor->setRelease(convertReleaseTypeForUI(instADPCM->getWaveformRelease().type),
//							   instADPCM->getWaveformRelease().begin);
//	if (instADPCM->getWaveformEnabled()) {
//		ui->waveEditGroupBox->setChecked(true);
//		onWaveformNumberChanged();
//	}
//	else {
//		ui->waveEditGroupBox->setChecked(false);
//	}
}

void InstrumentEditorADPCMForm::setWaveformSequenceColumn(int col)
{
//	auto button = ui->squareMaskButtonGroup->checkedButton();
//	if (button == ui->squareMaskRawRadioButton) {
//		ui->waveEditor->setText(col, QString::number(ui->squareMaskRawSpinBox->value()));
//		ui->waveEditor->setData(col, ui->squareMaskRawSpinBox->value());
//	}
//	else {
//		ui->waveEditor->setText(col, QString::number(ui->squareMaskToneSpinBox->value()) + "/"
//								+ QString::number(ui->squareMaskMaskSpinBox->value()));

//		ui->waveEditor->setData(col, CommandSequenceUnit::ratio2data(
//									ui->squareMaskToneSpinBox->value(),
//									ui->squareMaskMaskSpinBox->value()));
//	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onWaveformNumberChanged()
{
//	// Change users view
//	std::vector<int> users = bt_.lock()->getWaveformADPCMUsers(ui->waveNumSpinBox->value());
//	QStringList l;
//	std::transform(users.begin(), users.end(), std::back_inserter(l), [](int n) {
//		return QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
//	});
//	ui->waveUsersLineEdit->setText(l.join(","));
}

void InstrumentEditorADPCMForm::onWaveformParameterChanged(int wfNum)
{
//	if (ui->waveNumSpinBox->value() == wfNum) {
//		Ui::EventGuard eg(isIgnoreEvent_);
//		setInstrumentWaveformParameters();
//	}
}

//void InstrumentEditorADPCMForm::on_waveEditGroupBox_toggled(bool arg1)
//{
//	if (!isIgnoreEvent_) {
//		bt_.lock()->setInstrumentADPCMWaveformEnabled(instNum_, arg1);
//		setInstrumentWaveformParameters();
//		emit waveformNumberChanged();
//		emit modified();
//	}

//	onWaveformNumberChanged();
//}

//void InstrumentEditorADPCMForm::on_waveNumSpinBox_valueChanged(int arg1)
//{
//	if (!isIgnoreEvent_) {
//		bt_.lock()->setInstrumentADPCMWaveform(instNum_, arg1);
//		setInstrumentWaveformParameters();
//		emit waveformNumberChanged();
//		emit modified();
//	}

//	onWaveformNumberChanged();
//}

//void InstrumentEditorADPCMForm::on_squareMaskRawSpinBox_valueChanged(int arg1)
//{
//	ui->squareMaskRawSpinBox->setSuffix(
//				QString(" (0x") + QString("%1 | ").arg(arg1, 3, 16, QChar('0')).toUpper()
//				+ QString("%1Hz)").arg(arg1 ? QString::number(124800.0 / arg1, 'f', 4) : "-")
//				);
//}

//--- Envelope
void InstrumentEditorADPCMForm::setInstrumentEnvelopeParameters()
{
//	Ui::EventGuard ev(isIgnoreEvent_);

//	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
//	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

//	ui->envNumSpinBox->setValue(instADPCM->getEnvelopeNumber());
//	ui->envEditor->clearData();
//	for (auto& com : instADPCM->getEnvelopeSequence()) {
//		QString str("");
//		if (com.type >= 16) {
//			if (CommandSequenceUnit::checkDataType(com.data) == CommandSequenceUnit::RATIO) {
//				auto ratio = CommandSequenceUnit::data2ratio(com.data);
//				str = QString("%1/%2").arg(ratio.first).arg(ratio.second);
//			}
//			else {
//				str = QString::number(com.data);
//			}
//		}
//		ui->envEditor->addSequenceCommand(com.type, str, com.data);
//	}
//	for (auto& l : instADPCM->getEnvelopeLoops()) {
//		ui->envEditor->addLoop(l.begin, l.end, l.times);
//	}
//	ui->envEditor->setRelease(convertReleaseTypeForUI(instADPCM->getEnvelopeRelease().type),
//							  instADPCM->getEnvelopeRelease().begin);
//	if (instADPCM->getEnvelopeEnabled()) {
//		ui->envEditGroupBox->setChecked(true);
//		onEnvelopeNumberChanged();
//	}
//	else {
//		ui->envEditGroupBox->setChecked(false);
//	}
}

void InstrumentEditorADPCMForm::setEnvelopeSequenceColumn(int col)
{
//	if (ui->hardFreqButtonGroup->checkedButton() == ui->hardFreqRawRadioButton) {
//		ui->envEditor->setText(col, QString::number(ui->hardFreqRawSpinBox->value()));
//		ui->envEditor->setData(col, ui->hardFreqRawSpinBox->value());
//	}
//	else {
//		ui->envEditor->setText(col, QString::number(ui->hardFreqToneSpinBox->value()) + "/"
//							   + QString::number(ui->hardFreqHardSpinBox->value()));

//		ui->envEditor->setData(col, CommandSequenceUnit::ratio2data(
//								   ui->hardFreqToneSpinBox->value(),
//								   ui->hardFreqHardSpinBox->value()));
//	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onEnvelopeNumberChanged()
{
//	// Change users view
//	std::vector<int> users = bt_.lock()->getEnvelopeADPCMUsers(ui->envNumSpinBox->value());
//	QStringList l;
//	std::transform(users.begin(), users.end(), std::back_inserter(l), [](int n) {
//		return QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
//	});
//	ui->envUsersLineEdit->setText(l.join((",")));
}

void InstrumentEditorADPCMForm::onEnvelopeParameterChanged(int envNum)
{
//	if (ui->envNumSpinBox->value() == envNum) {
//		Ui::EventGuard eg(isIgnoreEvent_);
//		setInstrumentEnvelopeParameters();
//	}
}

//void InstrumentEditorADPCMForm::on_envEditGroupBox_toggled(bool arg1)
//{
//	if (!isIgnoreEvent_) {
//		bt_.lock()->setInstrumentADPCMEnvelopeEnabled(instNum_, arg1);
//		setInstrumentEnvelopeParameters();
//		emit envelopeNumberChanged();
//		emit modified();
//	}

//	onEnvelopeNumberChanged();
//}

//void InstrumentEditorADPCMForm::on_envNumSpinBox_valueChanged(int arg1)
//{
//	if (!isIgnoreEvent_) {
//		bt_.lock()->setInstrumentADPCMEnvelope(instNum_, arg1);
//		setInstrumentEnvelopeParameters();
//		emit envelopeNumberChanged();
//		emit modified();
//	}

//	onEnvelopeNumberChanged();
//}

//void InstrumentEditorADPCMForm::on_hardFreqRawSpinBox_valueChanged(int arg1)
//{
//	ui->hardFreqRawSpinBox->setSuffix(
//				QString(" (0x") + QString("%1 | ").arg(arg1, 4, 16, QChar('0')).toUpper()
//				+ QString("%1Hz").arg(arg1 ? QString::number(7800.0 / arg1, 'f', 4) : "-"));
//}

//--- Arpeggio
void InstrumentEditorADPCMForm::setInstrumentArpeggioParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->arpNumSpinBox->setValue(instADPCM->getArpeggioNumber());
	ui->arpEditor->clearData();
	for (auto& com : instADPCM->getArpeggioSequence()) {
		ui->arpEditor->addSequenceCommand(com.type);
	}
	for (auto& l : instADPCM->getArpeggioLoops()) {
		ui->arpEditor->addLoop(l.begin, l.end, l.times);
	}
	ui->arpEditor->setRelease(convertReleaseTypeForUI(instADPCM->getArpeggioRelease().type),
							  instADPCM->getArpeggioRelease().begin);
	for (int i = 0; i < ui->arpTypeComboBox->count(); ++i) {
		if (instADPCM->getArpeggioType() == convertSequenceTypeForData(
					static_cast<VisualizedInstrumentMacroEditor::SequenceType>(ui->arpTypeComboBox->itemData(i).toInt()))) {
			ui->arpTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instADPCM->getArpeggioEnabled()) {
		ui->arpEditGroupBox->setChecked(true);
		onArpeggioNumberChanged();
	}
	else {
		ui->arpEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onArpeggioNumberChanged()
{
	// Change users view
	std::vector<int> users = bt_.lock()->getArpeggioADPCMUsers(ui->arpNumSpinBox->value());
	QStringList l;
	std::transform(users.begin(), users.end(), std::back_inserter(l), [](int n) {
		return QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
	});
	ui->arpUsersLineEdit->setText(l.join(","));
}

void InstrumentEditorADPCMForm::onArpeggioParameterChanged(int tnNum)
{
	if (ui->arpNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentArpeggioParameters();
	}
}

void InstrumentEditorADPCMForm::onArpeggioTypeChanged(int index)
{
	Q_UNUSED(index)

	auto type = static_cast<VisualizedInstrumentMacroEditor::SequenceType>(
					ui->arpTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setArpeggioADPCMType(ui->arpNumSpinBox->value(), convertSequenceTypeForData(type));
		emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->arpEditor->setSequenceType(type);
}

void InstrumentEditorADPCMForm::on_arpEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMArpeggioEnabled(instNum_, arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

void InstrumentEditorADPCMForm::on_arpNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMArpeggio(instNum_, arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

//--- Pitch
void InstrumentEditorADPCMForm::setInstrumentPitchParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->ptNumSpinBox->setValue(instADPCM->getPitchNumber());
	ui->ptEditor->clearData();
	for (auto& com : instADPCM->getPitchSequence()) {
		ui->ptEditor->addSequenceCommand(com.type);
	}
	for (auto& l : instADPCM->getPitchLoops()) {
		ui->ptEditor->addLoop(l.begin, l.end, l.times);
	}
	ui->ptEditor->setRelease(convertReleaseTypeForUI(instADPCM->getPitchRelease().type),
							 instADPCM->getPitchRelease().begin);
	for (int i = 0; i < ui->ptTypeComboBox->count(); ++i) {
		if (instADPCM->getPitchType() == convertSequenceTypeForData(
					static_cast<VisualizedInstrumentMacroEditor::SequenceType>(ui->ptTypeComboBox->itemData(i).toInt()))) {
			ui->ptTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instADPCM->getPitchEnabled()) {
		ui->ptEditGroupBox->setChecked(true);
		onPitchNumberChanged();
	}
	else {
		ui->ptEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onPitchNumberChanged()
{
	// Change users view
	std::vector<int> users = bt_.lock()->getPitchADPCMUsers(ui->ptNumSpinBox->value());
	QStringList l;
	std::transform(users.begin(), users.end(), std::back_inserter(l), [](int n) {
		return QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
	});
	ui->ptUsersLineEdit->setText(l.join(","));
}

void InstrumentEditorADPCMForm::onPitchParameterChanged(int tnNum)
{
	if (ui->ptNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentPitchParameters();
	}
}

void InstrumentEditorADPCMForm::onPitchTypeChanged(int index)
{
	Q_UNUSED(index)

	auto type = static_cast<VisualizedInstrumentMacroEditor::SequenceType>(ui->ptTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setPitchADPCMType(ui->ptNumSpinBox->value(), convertSequenceTypeForData(type));
		emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->ptEditor->setSequenceType(type);
}

void InstrumentEditorADPCMForm::on_ptEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMPitchEnabled(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}

void InstrumentEditorADPCMForm::on_ptNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMPitch(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}
