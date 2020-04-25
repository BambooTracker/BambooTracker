#include "instrument_editor_drumkit_form.hpp"
#include "ui_instrument_editor_drumkit_form.h"
#include <QString>
#include "instrument.hpp"
#include "gui/event_guard.hpp"
#include "gui/jam_layout.hpp"
#include "gui/gui_util.hpp"

InstrumentEditorDrumkitForm::InstrumentEditorDrumkitForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorDrumkitForm),
	instNum_(num),
	isIgnoreEvent_(false),
	hasShown_(false)
{
	ui->setupUi(this);

	ui->keyTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	QString tone[] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
	for (int i = 0; i < 96; ++i) {
		ui->keyTreeWidget->addTopLevelItem(
					new QTreeWidgetItem({ tone[i % 12] + QString::number(i / 12), "-", "-" }));
	}

	//========== Waveform ==========//
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::modified, this, [&] { emit modified(); });
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::waveformNumberChanged,
					 this, [&](int n) {
		bt_.lock()->setInstrumentDrumkitWaveform(instNum_, ui->keyTreeWidget->currentIndex().row(), n);
		setInstrumentWaveformParameters(ui->keyTreeWidget->currentIndex().row());
		emit waveformNumberChanged();
		emit modified();

		if (config_.lock()->getWriteOnlyUsedSamples()) {
			emit waveformAssignRequested();
		}
	}, Qt::DirectConnection);
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::waveformParameterChanged,
					 this, [&](int wfNum) {
		emit waveformParameterChanged(wfNum, instNum_);
	});
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::waveformAssignRequested,
					 this, [&] { emit waveformAssignRequested(); });
	QObject::connect(ui->waveEditor, &ADPCMWaveformEditor::waveformMemoryChanged,
					 this, [&] { emit waveformMemoryChanged(); });
}

InstrumentEditorDrumkitForm::~InstrumentEditorDrumkitForm()
{
	delete ui;
}

int InstrumentEditorDrumkitForm::getInstrumentNumber() const
{
	return instNum_;
}

void InstrumentEditorDrumkitForm::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
	ui->waveEditor->setCore(core);
	updateInstrumentParameters();
}

void InstrumentEditorDrumkitForm::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
	ui->waveEditor->setConfiguration(config);
}

void InstrumentEditorDrumkitForm::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
	ui->waveEditor->setColorPalette(palette);
}

void InstrumentEditorDrumkitForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());
	auto name = utf8ToQString(instKit->getName());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	for (const auto& key : instKit->getAssignedKeys()) {
		setInstrumentWaveformParameters(key);
	}
}

/********** Events **********/
void InstrumentEditorDrumkitForm::showEvent(QShowEvent*)
{
	if (!hasShown_) {
		ui->keyTreeWidget->setCurrentItem(ui->keyTreeWidget->topLevelItem(48));
		ui->keyTreeWidget->scrollTo(ui->keyTreeWidget->model()->index(48, 0),
									QAbstractItemView::PositionAtTop);
	}
	hasShown_ = true;
}

// MUST DIRECT CONNECTION
void InstrumentEditorDrumkitForm::keyPressEvent(QKeyEvent *event)
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
void InstrumentEditorDrumkitForm::keyReleaseEvent(QKeyEvent *event)
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

/********** Slots **********/
void InstrumentEditorDrumkitForm::on_keyTreeWidget_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
	Ui::EventGuard eg(isIgnoreEvent_);

	int key = ui->keyTreeWidget->currentIndex().row();
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());

	bool enabled = instKit->getWaveformEnabled(key);
	ui->waveGroupBox->setChecked(enabled);
	if (enabled) {
		setInstrumentWaveformParameters(key);
		ui->pitshSpinBox->setValue(instKit->getPitch(key));
	}
}

void InstrumentEditorDrumkitForm::on_pitshSpinBox_valueChanged(int arg1)
{
	int key = ui->keyTreeWidget->currentIndex().row();
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());

	if (instKit->getWaveformEnabled(key)) {
		bt_.lock()->setInstrumentDrumkitPitch(instNum_, key, arg1);
		ui->keyTreeWidget->currentItem()->setText(2, QString::number(arg1));
	}
}

//--- Waveform
void InstrumentEditorDrumkitForm::setInstrumentWaveformParameters(int key)
{
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());

	if (instKit->getWaveformEnabled(key)) {
		int wfNum = instKit->getWaveformNumber(key);
		ui->waveEditor->setInstrumentWaveformParameters(
					wfNum, instKit->isWaveformRepeatable(key),
					instKit->getWaveformRootKeyNumber(key), instKit->getWaveformRootDeltaN(key),
					instKit->getWaveformStartAddress(key), instKit->getWaveformStopAddress(key),
					instKit->getWaveformSamples(key));
		ui->keyTreeWidget->currentItem()->setText(1, QString::number(wfNum));
		ui->keyTreeWidget->currentItem()->setText(2, QString::number(instKit->getPitch(key)));
	}
	else {
		ui->waveEditor->setInstrumentWaveformParameters(
					0, bt_.lock()->getWaveformADPCMRepeatEnabled(0),
					bt_.lock()->getWaveformADPCMRootKeyNumber(0),
					bt_.lock()->getWaveformADPCMRootDeltaN(0),
					bt_.lock()->getWaveformADPCMStartAddress(0),
					bt_.lock()->getWaveformADPCMStopAddress(0),
					bt_.lock()->getWaveformADPCMSample(0));
		ui->keyTreeWidget->currentItem()->setText(1, "-");
		ui->keyTreeWidget->currentItem()->setText(2, "-");
	}
}

/********** Slots **********/
void InstrumentEditorDrumkitForm::onWaveformNumberChanged()
{
	ui->waveEditor->onWaveformNumberChanged();
}

void InstrumentEditorDrumkitForm::onWaveformParameterChanged(int wfNum)
{
	if (ui->waveEditor->getWaveformNumber() == wfNum) {
		setInstrumentWaveformParameters(ui->keyTreeWidget->currentIndex().row());
	}
}

void InstrumentEditorDrumkitForm::onWaveformSampleMemoryUpdated()
{
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());
	int key = ui->keyTreeWidget->currentIndex().row();

	if (instKit->getWaveformEnabled(key)) {
			ui->waveEditor->onWaveformSampleMemoryUpdated(instKit->getWaveformStartAddress(key),
														  instKit->getWaveformStopAddress(key));
	}
	else {
		// Clear addresses
		ui->waveEditor->onWaveformSampleMemoryUpdated(0, 0);
	}
}

void InstrumentEditorDrumkitForm::on_waveGroupBox_clicked(bool checked)
{
	Ui::EventGuard eg(isIgnoreEvent_);

	int key = ui->keyTreeWidget->currentIndex().row();
	bt_.lock()->setInstrumentDrumkitWaveformEnabled(instNum_, key, checked);
	if (checked) {
		ui->pitshSpinBox->setValue(0);
		setInstrumentWaveformParameters(key);
	}
	else {
		// Clear parameters
		ui->waveEditor->setInstrumentWaveformParameters(
					0, bt_.lock()->getWaveformADPCMRepeatEnabled(0),
					bt_.lock()->getWaveformADPCMRootKeyNumber(0),
					bt_.lock()->getWaveformADPCMRootDeltaN(0),
					bt_.lock()->getWaveformADPCMStartAddress(0),
					bt_.lock()->getWaveformADPCMStopAddress(0),
					bt_.lock()->getWaveformADPCMSample(0));

		ui->keyTreeWidget->currentItem()->setText(1, "-");
		ui->keyTreeWidget->currentItem()->setText(2, "-");
	}

	emit waveformNumberChanged();
}
