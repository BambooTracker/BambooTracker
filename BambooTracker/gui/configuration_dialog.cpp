#include "configuration_dialog.hpp"
#include "ui_configuration_dialog.h"
#include <algorithm>
#include <functional>
#include <QPushButton>
#include <QMenu>
#include <QMessageBox>
#include <QAudio>
#include <QAudioDeviceInfo>
#include "slider_style.hpp"
#include "fm_envelope_set_edit_dialog.hpp"
#include "midi/midi.hpp"
#include "jam_manager.hpp"
#include "chips/chip_misc.h"

ConfigurationDialog::ConfigurationDialog(std::weak_ptr<Configuration> config, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ConfigurationDialog),
	  config_(config)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	QObject::connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
					 this, [&] {
		on_ConfigurationDialog_accepted();
		emit applyPressed();
	});

	std::shared_ptr<Configuration> configLocked = config.lock();
	// General //
	// General settings
	ui->generalSettingsListWidget->item(0)->setCheckState(toCheckState(configLocked->getWarpCursor()));
	ui->generalSettingsListWidget->item(1)->setCheckState(toCheckState(configLocked->getWarpAcrossOrders()));
	ui->generalSettingsListWidget->item(2)->setCheckState(toCheckState(configLocked->getShowRowNumberInHex()));
	ui->generalSettingsListWidget->item(3)->setCheckState(toCheckState(configLocked->getShowPreviousNextOrders()));
	ui->generalSettingsListWidget->item(4)->setCheckState(toCheckState(configLocked->getBackupModules()));
	ui->generalSettingsListWidget->item(5)->setCheckState(toCheckState(configLocked->getDontSelectOnDoubleClick()));
	ui->generalSettingsListWidget->item(6)->setCheckState(toCheckState(configLocked->getReverseFMVolumeOrder()));
	ui->generalSettingsListWidget->item(7)->setCheckState(toCheckState(configLocked->getMoveCursorToRight()));
	ui->generalSettingsListWidget->item(8)->setCheckState(toCheckState(configLocked->getRetrieveChannelState()));
	ui->generalSettingsListWidget->item(9)->setCheckState(toCheckState(configLocked->getEnableTranslation()));
	ui->generalSettingsListWidget->item(10)->setCheckState(toCheckState(configLocked->getShowFMDetuneAsSigned()));
	ui->generalSettingsListWidget->item(11)->setCheckState(toCheckState(configLocked->getShowWaveVisual()));

	// Edit settings
	ui->pageJumpLengthSpinBox->setValue(static_cast<int>(configLocked->getPageJumpLength()));

	// Keys
	ui->keyOffKeySequenceEdit->setKeySequence(
				QString::fromUtf8(configLocked->getKeyOffKey().c_str(),
								  static_cast<int>(configLocked->getKeyOffKey().length())));
	ui->octaveUpKeySequenceEdit->setKeySequence(
				QString::fromUtf8(configLocked->getOctaveUpKey().c_str(),
								  static_cast<int>(configLocked->getOctaveUpKey().length())));
	ui->octaveDownKeySequenceEdit->setKeySequence(
				QString::fromUtf8(configLocked->getOctaveDownKey().c_str(),
								  static_cast<int>(configLocked->getOctaveDownKey().length())));
	ui->echoBufferKeySequenceEdit->setKeySequence(
				QString::fromUtf8(configLocked->getEchoBufferKey().c_str(),
								  static_cast<int>(configLocked->getEchoBufferKey().length())));
	ui->keyboardTypeComboBox->setCurrentIndex(static_cast<int>(configLocked->getNoteEntryLayout()));

	customLayoutKeysMap = {
		{JamKey::LOW_C,     ui->lowCEdit},
		{JamKey::LOW_CS,    ui->lowCSEdit},
		{JamKey::LOW_D,     ui->lowDEdit},
		{JamKey::LOW_DS,    ui->lowDSEdit},
		{JamKey::LOW_E,     ui->lowEEdit},
		{JamKey::LOW_F,     ui->lowFEdit},
		{JamKey::LOW_FS,    ui->lowFSEdit},
		{JamKey::LOW_G,     ui->lowGEdit},
		{JamKey::LOW_GS,    ui->lowGSEdit},
		{JamKey::LOW_A,     ui->lowAEdit},
		{JamKey::LOW_AS,    ui->lowASEdit},
		{JamKey::LOW_B,     ui->lowBEdit},
		{JamKey::LOW_C_H,   ui->lowHighCEdit},
		{JamKey::LOW_CS_H,  ui->lowHighCSEdit},
		{JamKey::LOW_D_H,   ui->lowHighDEdit},

		{JamKey::HIGH_C,    ui->highCEdit},
		{JamKey::HIGH_CS,   ui->highCSEdit},
		{JamKey::HIGH_D,    ui->highDEdit},
		{JamKey::HIGH_DS,   ui->highDSEdit},
		{JamKey::HIGH_E,    ui->highEEdit},
		{JamKey::HIGH_F,    ui->highFEdit},
		{JamKey::HIGH_FS,   ui->highFSEdit},
		{JamKey::HIGH_G,    ui->highGEdit},
		{JamKey::HIGH_GS,   ui->highGSEdit},
		{JamKey::HIGH_A,    ui->highAEdit},
		{JamKey::HIGH_AS,   ui->highASEdit},
		{JamKey::HIGH_B,    ui->highBEdit},
		{JamKey::HIGH_C_H,  ui->highHighCEdit},
		{JamKey::HIGH_CS_H, ui->highHighCSEdit},
		{JamKey::HIGH_D_H,  ui->highHighDEdit}
	};
	std::map<std::string, JamKey> customLayoutMapping = configLocked->getCustomLayoutKeys();
	std::map<std::string, JamKey>::const_iterator customLayoutMappingIterator = customLayoutMapping.begin();
	while (customLayoutMappingIterator != customLayoutMapping.end()) {
		customLayoutKeysMap.at(customLayoutMappingIterator->second)->setKeySequence(QKeySequence(QString::fromStdString(customLayoutMappingIterator->first)));
		customLayoutMappingIterator++;
	}

	// Emulation //
	ui->emulatorComboBox->addItem("MAME YM2608", static_cast<int>(chip::Emu::Mame));
	ui->emulatorComboBox->addItem("Nuked OPN-Mod", static_cast<int>(chip::Emu::Nuked));
	ui->emulatorComboBox->setCurrentIndex(ui->emulatorComboBox->findData(configLocked->getEmulator()));

	// Sound //
	int devRow = -1;
	int defDevRow = 0;
	for (auto& info : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
		ui->soundDeviceComboBox->addItem(info.deviceName());
		if (info.deviceName() == QString::fromUtf8(configLocked->getSoundDevice().c_str(),
												   static_cast<int>(configLocked->getSoundDevice().length())))
			devRow = ui->soundDeviceComboBox->count() - 1;
		if (info.deviceName() == QAudioDeviceInfo::defaultOutputDevice().deviceName()) {
			defDevRow = ui->soundDeviceComboBox->count() - 1;
		}
	}
	ui->soundDeviceComboBox->setCurrentIndex((devRow == -1) ? defDevRow : devRow);

	ui->useSCCICheckBox->setCheckState(configLocked->getUseSCCI() ? Qt::Checked : Qt::Unchecked);
	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("55466Hz", 55466);
	switch (configLocked->getSampleRate()) {
	case 44100:	ui->sampleRateComboBox->setCurrentIndex(0);	break;
	case 48000:	ui->sampleRateComboBox->setCurrentIndex(1);	break;
	case 55466:	ui->sampleRateComboBox->setCurrentIndex(2);	break;
	}
	ui->bufferLengthHorizontalSlider->setStyle(new SliderStyle());
	QObject::connect(ui->bufferLengthHorizontalSlider, &QSlider::valueChanged,
					 this, [&](int value) {
		ui->bufferLengthLabel->setText(QString::number(value) + "ms");
	});
	ui->bufferLengthHorizontalSlider->setValue(static_cast<int>(configLocked->getBufferLength()));

	// Midi //
	MidiInterface &midiIntf = MidiInterface::instance();
	if (midiIntf.supportsVirtualPort())
		ui->midiInputNameLine->setPlaceholderText(tr("Virtual port"));
	ui->midiInputNameLine->setText(QString::fromStdString(configLocked->getMidiInputPort()));

	// Mixer //
	ui->masterMixerSlider->setText(tr("Master"));
	ui->masterMixerSlider->setSuffix("%");
	ui->masterMixerSlider->setMaximum(100);
	ui->masterMixerSlider->setMinimum(0);
	ui->masterMixerSlider->setTickPosition(QSlider::TicksBothSides);
	ui->masterMixerSlider->setTickInterval(20);
	ui->masterMixerSlider->setValue(configLocked->getMixerVolumeMaster());

	ui->fmMixerSlider->setText("FM");
	ui->fmMixerSlider->setSuffix("dB");
	ui->fmMixerSlider->setMaximum(120);
	ui->fmMixerSlider->setMinimum(-120);
	ui->fmMixerSlider->setValueRate(0.1);
	ui->fmMixerSlider->setSign(true);
	ui->fmMixerSlider->setTickPosition(QSlider::TicksBothSides);
	ui->fmMixerSlider->setTickInterval(20);
	ui->fmMixerSlider->setValue(static_cast<int>(configLocked->getMixerVolumeFM() * 10));

	ui->ssgMixerSlider->setText("SSG");
	ui->ssgMixerSlider->setSuffix("dB");
	ui->ssgMixerSlider->setMaximum(120);
	ui->ssgMixerSlider->setMinimum(-120);
	ui->ssgMixerSlider->setValueRate(0.1);
	ui->ssgMixerSlider->setSign(true);
	ui->ssgMixerSlider->setTickPosition(QSlider::TicksBothSides);
	ui->ssgMixerSlider->setTickInterval(20);
	ui->ssgMixerSlider->setValue(static_cast<int>(configLocked->getMixerVolumeSSG() * 10));

	// Input //
	fmEnvelopeTexts_ = configLocked->getFMEnvelopeTexts();
	updateEnvelopeSetUi();
}

ConfigurationDialog::~ConfigurationDialog()
{
	delete ui;
}

void ConfigurationDialog::on_ConfigurationDialog_accepted()
{
	std::shared_ptr<Configuration> configLocked = config_.lock();
	// General //
	// General settings
	configLocked->setWarpCursor(fromCheckState(ui->generalSettingsListWidget->item(0)->checkState()));
	configLocked->setWarpAcrossOrders(fromCheckState(ui->generalSettingsListWidget->item(1)->checkState()));
	configLocked->setShowRowNumberInHex(fromCheckState(ui->generalSettingsListWidget->item(2)->checkState()));
	configLocked->setShowPreviousNextOrders(fromCheckState(ui->generalSettingsListWidget->item(3)->checkState()));
	configLocked->setBackupModules(fromCheckState(ui->generalSettingsListWidget->item(4)->checkState()));
	configLocked->setDontSelectOnDoubleClick(fromCheckState(ui->generalSettingsListWidget->item(5)->checkState()));
	configLocked->setReverseFMVolumeOrder(fromCheckState(ui->generalSettingsListWidget->item(6)->checkState()));
	configLocked->setMoveCursorToRight(fromCheckState(ui->generalSettingsListWidget->item(7)->checkState()));
	configLocked->setRetrieveChannelState(fromCheckState(ui->generalSettingsListWidget->item(8)->checkState()));
	configLocked->setEnableTranslation(fromCheckState(ui->generalSettingsListWidget->item(9)->checkState()));
	configLocked->setShowFMDetuneAsSigned(fromCheckState(ui->generalSettingsListWidget->item(10)->checkState()));
	configLocked->setShowWaveVisual(fromCheckState(ui->generalSettingsListWidget->item(11)->checkState()));

	// Edit settings
	configLocked->setPageJumpLength(static_cast<size_t>(ui->pageJumpLengthSpinBox->value()));

	// Keys
	configLocked->setKeyOffKey(ui->keyOffKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setOctaveUpKey(ui->octaveUpKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setOctaveDownKey(ui->octaveDownKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setEchoBufferKey(ui->echoBufferKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setNoteEntryLayout(static_cast<Configuration::KeyboardLayout>(ui->keyboardTypeComboBox->currentIndex()));
	std::map<std::string, JamKey> customLayoutNewKeys = {};
	std::map<JamKey, QKeySequenceEdit *>::const_iterator customLayoutKeysMapIterator = customLayoutKeysMap.begin();
	while (customLayoutKeysMapIterator != customLayoutKeysMap.end()) {
		customLayoutNewKeys[customLayoutKeysMapIterator->second->keySequence().toString().toStdString()] = customLayoutKeysMapIterator->first;
		customLayoutKeysMapIterator++;
	}
	configLocked->setCustomLayoutKeys(customLayoutNewKeys);

	// Emulation //
	int emu = ui->emulatorComboBox->currentData().toInt();
	bool changedEmu = false;
	if (emu != configLocked->getEmulator()) {
		configLocked->setEmulator(emu);
		changedEmu = true;
	}

	// Sound //
	configLocked->setSoundDevice(ui->soundDeviceComboBox->currentText().toUtf8().toStdString());
	configLocked->setUseSCCI(ui->useSCCICheckBox->checkState() == Qt::Checked);
	configLocked->setSampleRate(ui->sampleRateComboBox->currentData(Qt::UserRole).toUInt());
	configLocked->setBufferLength(static_cast<size_t>(ui->bufferLengthHorizontalSlider->value()));

	// Midi //
	configLocked->setMidiInputPort(ui->midiInputNameLine->text().toStdString());

	// Mixer //
	configLocked->setMixerVolumeMaster(ui->masterMixerSlider->value());
	configLocked->setMixerVolumeFM(ui->fmMixerSlider->value() * 0.1);
	configLocked->setMixerVolumeSSG(ui->ssgMixerSlider->value() * 0.1);

	// Input //
	std::sort(fmEnvelopeTexts_.begin(), fmEnvelopeTexts_.end(),
			  [](const FMEnvelopeText& a, const FMEnvelopeText& b) -> bool { return (a.name < b.name); });
	configLocked->setFMEnvelopeTexts(fmEnvelopeTexts_);

	if (changedEmu) {
		QMessageBox::information(this, tr("Configuration"), tr("The change of emulator will be effective after restarting the program."));
	}
}

/***** General *****/
void ConfigurationDialog::on_generalSettingsListWidget_itemSelectionChanged()
{
	QString text;
	switch (ui->generalSettingsListWidget->currentRow()) {
	case 0:		// Warp cursor
		text = tr("Warp the cursor around the edges of the pattern editor.");
		break;
	case 1:		// Warp across orders
		text = tr("Move to previous or next order when reaching top or bottom in the pattern editor.");
		break;
	case 2:		// Show row numbers in hex
		text = tr("Display order numbers and the order count on the status bar in hexadecimal.");
		break;
	case 3:		// Preview previous/next orders
		text = tr("Preview previous and next orders in the pattern editor.");
		break;
	case 4:		// Backup modeles
		text = tr("Create a backup copy of the existing file when saving a module.");
		break;
	case 5:		// Don't select on double click
		text = tr("Don't select the whole track when double-clicking in the pattern editor.");
		break;
	case 6:		// Reverse FM volume order
		text = tr("Reverse the order of FM volume so that 00 is the quietest in the pattern editor.");
		break;
	case 7:		// Move cursor to right
		text = tr("Move the cursor to right after entering effects in the pattern editor.");
		break;
	case 8:		// Retrieve channel state
		text = tr("Reconstruct the current channel's state from previous orders upon playing.");
		break;
	case 9:		// Enable translation
		text = tr("Translate to your language from the next launch. "
				  "See readme to check supported languages.");
		break;
	case 10:	// Show FM detune as signed
		text = tr("Display FM detune values as signed numbers in the FM envelope editor.");
		break;
	case 11:	// Show wave visual
		text = tr("Enable an oscilloscope which displays a waveform of the sound output.");
		break;
	default:
		text = "";
		break;
	}
	ui->descPlainTextEdit->setPlainText(tr("Description: ") + text);
}

/***** Mixer *****/
void ConfigurationDialog::on_mixerResetPushButton_clicked()
{
	ui->fmMixerSlider->setValue(0);
	ui->ssgMixerSlider->setValue(0);
}

/***** MIDI *****/
void ConfigurationDialog::on_midiInputChoiceButton_clicked()
{
	QToolButton *button = ui->midiInputChoiceButton;
	QMenu menu;
	QAction *action;

	MidiInterface &intf = MidiInterface::instance();
	std::vector<std::string> ports = intf.getRealInputPorts();
	bool vport = intf.supportsVirtualPort();

	if (vport) {
		ui->midiInputNameLine->setPlaceholderText(tr("Virtual port"));
		action = menu.addAction(tr("Virtual port"));
		action->setData(QString());
	}

	for (unsigned i = 0, n = ports.size(); i < n; ++i) {
		if (i == 0 && vport)
			menu.addSeparator();
		QString portName = QString::fromStdString(ports[i]);
		action = menu.addAction(portName);
		action->setData(portName);
	}

	QAction *choice = menu.exec(button->mapToGlobal(button->rect().bottomLeft()));
	if (choice) {
		QString portName = choice->data().toString();
		ui->midiInputNameLine->setText(portName);
	}
}

/***** Input *****/
void ConfigurationDialog::updateEnvelopeSetUi()
{
	std::sort(fmEnvelopeTexts_.begin(), fmEnvelopeTexts_.end(),
			  [](const FMEnvelopeText& a, const FMEnvelopeText& b) -> bool { return (a.name < b.name); });

	ui->envelopeTypeListWidget->clear();
	for (auto& texts : fmEnvelopeTexts_)
		ui->envelopeTypeListWidget->addItem(
					QString::fromUtf8(texts.name.c_str(), static_cast<int>(texts.name.length())));
}

void ConfigurationDialog::on_addEnvelopeSetPushButton_clicked()
{
	auto name = QString("Set %1").arg(fmEnvelopeTexts_.size() + 1);
	fmEnvelopeTexts_.push_back({ name.toUtf8().toStdString(), std::vector<FMEnvelopeTextType>() });
	updateEnvelopeSetUi();
	for (int i = ui->envelopeTypeListWidget->count() - 1; i >= 0; --i) {
		if (ui->envelopeTypeListWidget->item(i)->text() == name) {
			ui->envelopeTypeListWidget->setCurrentRow(i);
			break;
		}
	}
}

void ConfigurationDialog::on_removeEnvelopeSetpushButton_clicked()
{
	fmEnvelopeTexts_.erase(fmEnvelopeTexts_.begin() + ui->envelopeTypeListWidget->currentRow());
	updateEnvelopeSetUi();
}

void ConfigurationDialog::on_editEnvelopeSetPushButton_clicked()
{
	size_t row = static_cast<size_t>(ui->envelopeTypeListWidget->currentRow());
	FMEnvelopeSetEditDialog diag(fmEnvelopeTexts_.at(row).texts);
	diag.setWindowTitle(diag.windowTitle() + ": " + ui->envelopeSetNameLineEdit->text());
	if (diag.exec() == QDialog::Accepted) {
		fmEnvelopeTexts_.at(row).texts = diag.getSet();
	}
}

void ConfigurationDialog::on_envelopeSetNameLineEdit_textChanged(const QString &arg1)
{
	fmEnvelopeTexts_.at(static_cast<size_t>(ui->envelopeTypeListWidget->currentRow())).name = arg1.toStdString();
	ui->envelopeTypeListWidget->currentItem()->setText(arg1);
}

void ConfigurationDialog::on_envelopeTypeListWidget_currentRowChanged(int currentRow)
{
	if (currentRow == -1) {
		ui->editEnvelopeSetPushButton->setEnabled(false);
		ui->removeEnvelopeSetpushButton->setEnabled(false);
		ui->envelopeSetNameLineEdit->setEnabled(false);
	}
	else {
		ui->editEnvelopeSetPushButton->setEnabled(true);
		ui->removeEnvelopeSetpushButton->setEnabled(true);
		ui->envelopeSetNameLineEdit->setEnabled(true);
		ui->envelopeSetNameLineEdit->setText(ui->envelopeTypeListWidget->item(currentRow)->text());
	}
}

void ConfigurationDialog::on_keyboardTypeComboBox_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	bool enableCustomLayoutInterface = ui->keyboardTypeComboBox->currentIndex() == 0;
	ui->lowHighKeysTabWidget->setEnabled(enableCustomLayoutInterface);
	ui->customLayoutResetButton->setEnabled(enableCustomLayoutInterface);
}

void ConfigurationDialog::on_customLayoutResetButton_clicked()
{
	std::map<std::string, JamKey> QWERTYLayoutMapping = config_.lock()->mappingLayouts.at (Configuration::KeyboardLayout::QWERTY);
	std::map<std::string, JamKey>::const_iterator QWERTYLayoutMappingIterator = QWERTYLayoutMapping.begin();
	while (QWERTYLayoutMappingIterator != QWERTYLayoutMapping.end()) {
		customLayoutKeysMap.at(QWERTYLayoutMappingIterator->second)->setKeySequence(QKeySequence(QString::fromStdString(QWERTYLayoutMappingIterator->first)));
		QWERTYLayoutMappingIterator++;
	}
}
