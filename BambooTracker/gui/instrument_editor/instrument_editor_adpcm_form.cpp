#include "instrument_editor_adpcm_form.hpp"
#include "ui_instrument_editor_adpcm_form.h"
#include "instrument.hpp"
#include "gui/event_guard.hpp"
#include "gui/jam_layout.hpp"
#include "gui/instrument_editor/instrument_editor_util.hpp"

InstrumentEditorADPCMForm::InstrumentEditorADPCMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorADPCMForm),
	instNum_(num),
	isIgnoreEvent_(false)
{
	ui->setupUi(this);

	//========== Waveform ==========//
	ui->waveEditor->setNumber(instNum_);
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::modified, this, [&] { emit modified(); });
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::waveformNumberChanged,
					 this, [&] { emit waveformNumberChanged(); });
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::waveformParameterChanged,
					 this, [&](int wfNum, int fromInstNum) {
		emit waveformParameterChanged(wfNum, fromInstNum);
	});
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::waveformAssignRequested,
					 this, [&] { emit waveformAssignRequested(); });

	//========== Envelope ==========//
	ui->envEditor->setMaximumDisplayedRowCount(64);
	ui->envEditor->setDefaultRow(255);
	ui->envEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 256; ++i) {
		ui->envEditor->AddRow(QString::number(i), false);
	}
	ui->envEditor->autoFitLabelWidth();
	ui->envEditor->setUpperRow(255);
	ui->envEditor->setMultipleReleaseState(true);
	ui->envEditor->setPermittedReleaseTypes(
				VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE_RELEASE
				| VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE_RELEASE
				| VisualizedInstrumentMacroEditor::ReleaseType::FIXED_RELEASE);

	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addEnvelopeADPCMSequenceCommand(
						ui->envNumSpinBox->value(), row, ui->envEditor->getSequenceDataAt(col));
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeEnvelopeADPCMSequenceCommand(ui->envNumSpinBox->value());
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeADPCMSequenceCommand(
						ui->envNumSpinBox->value(), col, row, ui->envEditor->getSequenceDataAt(col));
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeADPCMLoops(
						ui->envNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setEnvelopeADPCMRelease(ui->envNumSpinBox->value(), t, point);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Arpeggio ==========//
	ui->arpTypeComboBox->addItem(tr("Absolute"), VisualizedInstrumentMacroEditor::SequenceType::AbsoluteSequence);
	ui->arpTypeComboBox->addItem(tr("Fixed"), VisualizedInstrumentMacroEditor::SequenceType::FixedSequence);
	ui->arpTypeComboBox->addItem(tr("Relative"), VisualizedInstrumentMacroEditor::SequenceType::RelativeSequence);

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

	ui->ptTypeComboBox->addItem(tr("Absolute"), VisualizedInstrumentMacroEditor::SequenceType::AbsoluteSequence);
	ui->ptTypeComboBox->addItem(tr("Relative"), VisualizedInstrumentMacroEditor::SequenceType::RelativeSequence);

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
	ui->waveEditor->setCore(core);
	updateInstrumentParameters();
}

void InstrumentEditorADPCMForm::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
	ui->waveEditor->setConfiguration(config);
}

void InstrumentEditorADPCMForm::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
	ui->waveEditor->setColorPalette(palette);
	ui->envEditor->setColorPalette(palette);
	ui->arpEditor->setColorPalette(palette);
	ui->ptEditor->setColorPalette(palette);
}

void InstrumentEditorADPCMForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());
	auto name = QString::fromUtf8(instADPCM->getName().c_str(), static_cast<int>(instADPCM->getName().length()));
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentEnvelopeParameters();
	setInstrumentArpeggioParameters();
	setInstrumentPitchParameters();
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorADPCMForm::keyPressEvent(QKeyEvent *event)
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
/********** Slots **********/
void InstrumentEditorADPCMForm::onWaveformNumberChanged()
{
	ui->waveEditor->onWaveformNumberChanged();
}

void InstrumentEditorADPCMForm::onWaveformParameterChanged(int wfNum)
{
	ui->waveEditor->onWaveformParameterChanged(wfNum);
}

void InstrumentEditorADPCMForm::onWaveformSampleMemoryUpdated()
{
	ui->waveEditor->onWaveformSampleMemoryUpdated();
}

//--- Envelope
void InstrumentEditorADPCMForm::setInstrumentEnvelopeParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->envNumSpinBox->setValue(instADPCM->getEnvelopeNumber());
	ui->envEditor->clearData();
	for (auto& com : instADPCM->getEnvelopeSequence()) {
		ui->envEditor->addSequenceCommand(com.type);
	}
	for (auto& l : instADPCM->getEnvelopeLoops()) {
		ui->envEditor->addLoop(l.begin, l.end, l.times);
	}
	ui->envEditor->setRelease(convertReleaseTypeForUI(instADPCM->getEnvelopeRelease().type),
							  instADPCM->getEnvelopeRelease().begin);
	if (instADPCM->getEnvelopeEnabled()) {
		ui->envEditGroupBox->setChecked(true);
		onEnvelopeNumberChanged();
	}
	else {
		ui->envEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onEnvelopeNumberChanged()
{
	// Change users view
	std::vector<int> users = bt_.lock()->getEnvelopeADPCMUsers(ui->envNumSpinBox->value());
	QStringList l;
	std::transform(users.begin(), users.end(), std::back_inserter(l), [](int n) {
		return QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
	});
	ui->envUsersLineEdit->setText(l.join((",")));
}

void InstrumentEditorADPCMForm::onEnvelopeParameterChanged(int envNum)
{
	if (ui->envNumSpinBox->value() == envNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentEnvelopeParameters();
	}
}

void InstrumentEditorADPCMForm::on_envEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMEnvelopeEnabled(instNum_, arg1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

void InstrumentEditorADPCMForm::on_envNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMEnvelope(instNum_, arg1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

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
