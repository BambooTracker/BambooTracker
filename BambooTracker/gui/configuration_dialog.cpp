#include "configuration_dialog.hpp"
#include "ui_configuration_dialog.h"
#include <algorithm>
#include <functional>
#include <QPushButton>
#include <QMenu>
#include <QMessageBox>
#include <QAudio>
#include <QAudioDeviceInfo>
#include <QFont>
#include <QColorDialog>
#include <QFileDialog>
#include <QApplication>
#include "slider_style.hpp"
#include "fm_envelope_set_edit_dialog.hpp"
#include "midi/midi.hpp"
#include "jam_manager.hpp"
#include "chips/chip_misc.h"
#include "color_palette_handler.hpp"

ConfigurationDialog::ConfigurationDialog(std::weak_ptr<Configuration> config, std::weak_ptr<ColorPalette> palette,
										 std::string curApi, std::vector<std::string> apis, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ConfigurationDialog),
	  config_(config),
	  palette_(palette)
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
	ui->generalSettingsListWidget->item(12)->setCheckState(toCheckState(configLocked->getFill00ToEffectValue()));
	ui->generalSettingsListWidget->item(13)->setCheckState(toCheckState(configLocked->getAutosetInstrument()));
	ui->generalSettingsListWidget->item(14)->setCheckState(toCheckState(configLocked->getMoveCursorByHorizontalScroll()));

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
		{ JamKey::LowC,     ui->lowCEdit },
		{ JamKey::LowCS,    ui->lowCSEdit },
		{ JamKey::LowD,     ui->lowDEdit },
		{ JamKey::LowDS,    ui->lowDSEdit },
		{ JamKey::LowE,     ui->lowEEdit },
		{ JamKey::LowF,     ui->lowFEdit },
		{ JamKey::LowFS,    ui->lowFSEdit },
		{ JamKey::LowG,     ui->lowGEdit },
		{ JamKey::LowGS,    ui->lowGSEdit },
		{ JamKey::LowA,     ui->lowAEdit },
		{ JamKey::LowAS,    ui->lowASEdit },
		{ JamKey::LowB,     ui->lowBEdit },
		{ JamKey::LowC2,   ui->lowHighCEdit },
		{ JamKey::LowCS2,  ui->lowHighCSEdit },
		{ JamKey::LowD2,   ui->lowHighDEdit },

		{ JamKey::HighC,    ui->highCEdit },
		{ JamKey::HighCS,   ui->highCSEdit },
		{ JamKey::HighD,    ui->highDEdit },
		{ JamKey::HighDS,   ui->highDSEdit },
		{ JamKey::HighE,    ui->highEEdit },
		{ JamKey::HighF,    ui->highFEdit },
		{ JamKey::HighFS,   ui->highFSEdit },
		{ JamKey::HighG,    ui->highGEdit },
		{ JamKey::HighGS,   ui->highGSEdit },
		{ JamKey::HighA,    ui->highAEdit },
		{ JamKey::HighAS,   ui->highASEdit },
		{ JamKey::HighB,    ui->highBEdit },
		{ JamKey::HighC2,  ui->highHighCEdit },
		{ JamKey::HighCS2, ui->highHighCSEdit },
		{ JamKey::HighD2,  ui->highHighDEdit }
	};
	std::unordered_map<std::string, JamKey> customLayoutMapping = configLocked->getCustomLayoutKeys();
	std::unordered_map<std::string, JamKey>::const_iterator customLayoutMappingIterator = customLayoutMapping.begin();
	while (customLayoutMappingIterator != customLayoutMapping.end()) {
		customLayoutKeysMap.at(customLayoutMappingIterator->second)->setKeySequence(QKeySequence(QString::fromStdString(customLayoutMappingIterator->first)));
		customLayoutMappingIterator++;
	}

	// Emulation //
	ui->emulatorComboBox->addItem("MAME YM2608", static_cast<int>(chip::Emu::Mame));
	ui->emulatorComboBox->addItem("Nuked OPN-Mod", static_cast<int>(chip::Emu::Nuked));
	ui->emulatorComboBox->setCurrentIndex(ui->emulatorComboBox->findData(configLocked->getEmulator()));

	// Sound //
	int apiRow = -1;
	int defApiRow = 0;
	for (auto& name : apis) {
		ui->soundAPIComboBox->addItem(QString::fromUtf8(name.c_str(), static_cast<int>(name.length())));
		if (name == configLocked->getSoundAPI()) apiRow = ui->soundAPIComboBox->count() - 1;
		if (name == curApi) defApiRow = apiRow = ui->soundAPIComboBox->count() - 1;
	}
	ui->soundAPIComboBox->setCurrentIndex((apiRow == -1) ? defApiRow : apiRow);

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

	ui->realChipComboBox->addItem(tr("None"), static_cast<int>(RealChipInterface::NONE));
	ui->realChipComboBox->addItem("SCCI", static_cast<int>(RealChipInterface::SCCI));
	ui->realChipComboBox->addItem("C86CTL", static_cast<int>(RealChipInterface::C86CTL));
	switch (configLocked->getRealChipInterface()) {
	default:	// Fall through
	case RealChipInterface::NONE:	ui->realChipComboBox->setCurrentIndex(0);	break;
	case RealChipInterface::SCCI:	ui->realChipComboBox->setCurrentIndex(1);	break;
	case RealChipInterface::C86CTL:	ui->realChipComboBox->setCurrentIndex(2);	break;
	}

	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("55466Hz", 55466);
	switch (configLocked->getSampleRate()) {
	default:	// Fall through
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
	ui->masterMixerSlider->setMaximum(200);
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

	// Formats //
	fmEnvelopeTexts_ = configLocked->getFMEnvelopeTexts();
	updateEnvelopeSetUi();

	// Appearance //
	ui->colorsTreeWidget->setColumnWidth(0, 250);
	updateColorTree();

	ui->ptnHdFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getPatternEditorHeaderFont().c_str(),
										static_cast<int>(configLocked->getPatternEditorHeaderFont().size()))));
	ui->ptnHdFontSizeComboBox->setCurrentText(QString::number(configLocked->getPatternEditorHeaderFontSize()));
	ui->ptnRowFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getPatternEditorRowsFont().c_str(),
										static_cast<int>(configLocked->getPatternEditorRowsFont().size()))));
	ui->ptnRowFontSizeComboBox->setCurrentText(QString::number(configLocked->getPatternEditorRowsFontSize()));
	ui->odrHdFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getOrderListHeaderFont().c_str(),
										static_cast<int>(configLocked->getOrderListHeaderFont().size()))));
	ui->odrHdFontSizeComboBox->setCurrentText(QString::number(configLocked->getOrderListHeaderFontSize()));
	ui->odrRowFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getOrderListRowsFont().c_str(),
										static_cast<int>(configLocked->getOrderListRowsFont().size()))));
	ui->odrRowFontSizeComboBox->setCurrentText(QString::number(configLocked->getOrderListRowsFontSize()));
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
	configLocked->setFill00ToEffectValue(fromCheckState(ui->generalSettingsListWidget->item(12)->checkState()));
	configLocked->setAutosetInstrument(fromCheckState(ui->generalSettingsListWidget->item(13)->checkState()));
	configLocked->setMoveCursorByHorizontalScroll(fromCheckState(ui->generalSettingsListWidget->item(14)->checkState()));

	// Edit settings
	configLocked->setPageJumpLength(static_cast<size_t>(ui->pageJumpLengthSpinBox->value()));

	// Keys
	configLocked->setKeyOffKey(ui->keyOffKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setOctaveUpKey(ui->octaveUpKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setOctaveDownKey(ui->octaveDownKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setEchoBufferKey(ui->echoBufferKeySequenceEdit->keySequence().toString().toStdString());
	configLocked->setNoteEntryLayout(static_cast<Configuration::KeyboardLayout>(ui->keyboardTypeComboBox->currentIndex()));
	std::unordered_map<std::string, JamKey> customLayoutNewKeys;
	std::unordered_map<JamKey, QKeySequenceEdit *>::const_iterator customLayoutKeysMapIterator = customLayoutKeysMap.begin();
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
	configLocked->setSoundAPI(ui->soundAPIComboBox->currentText().toUtf8().toStdString());
	configLocked->setRealChipInterface(static_cast<RealChipInterface>(
										   ui->realChipComboBox->currentData(Qt::UserRole).toInt()));
	configLocked->setSampleRate(ui->sampleRateComboBox->currentData(Qt::UserRole).toUInt());
	configLocked->setBufferLength(static_cast<size_t>(ui->bufferLengthHorizontalSlider->value()));

	// Midi //
	configLocked->setMidiInputPort(ui->midiInputNameLine->text().toStdString());

	// Mixer //
	configLocked->setMixerVolumeMaster(ui->masterMixerSlider->value());
	configLocked->setMixerVolumeFM(ui->fmMixerSlider->value() * 0.1);
	configLocked->setMixerVolumeSSG(ui->ssgMixerSlider->value() * 0.1);

	// Formats //
	std::sort(fmEnvelopeTexts_.begin(), fmEnvelopeTexts_.end(),
			  [](const FMEnvelopeText& a, const FMEnvelopeText& b) -> bool { return (a.name < b.name); });
	configLocked->setFMEnvelopeTexts(fmEnvelopeTexts_);

	// Appearance //
	auto* pl = palette_.lock().get();
	QTreeWidgetItem* ptnColors = ui->colorsTreeWidget->topLevelItem(0);
	pl->ptnDefTextColor = ptnColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnDefStepColor = ptnColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl1StepColor = ptnColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl2StepColor = ptnColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurTextColor = ptnColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurStepColor = ptnColors->child(5)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurEditStepColor = ptnColors->child(6)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurCellColor = ptnColors->child(7)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnPlayStepColor = ptnColors->child(8)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnSelCellColor = ptnColors->child(9)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHovCellColor = ptnColors->child(10)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnDefStepNumColor = ptnColors->child(11)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl1StepNumColor = ptnColors->child(12)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl2StepNumColor = ptnColors->child(13)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnNoteColor = ptnColors->child(14)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnInstColor = ptnColors->child(15)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnVolColor = ptnColors->child(16)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnEffColor = ptnColors->child(17)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnErrorColor = ptnColors->child(18)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHeaderTextColor = ptnColors->child(19)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHeaderRowColor = ptnColors->child(20)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnMaskColor = ptnColors->child(21)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnBorderColor = ptnColors->child(22)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnMuteColor = ptnColors->child(23)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnUnmuteColor = ptnColors->child(24)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnBackColor = ptnColors->child(25)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* odrColors = ui->colorsTreeWidget->topLevelItem(1);
	pl->odrDefTextColor = odrColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrDefRowColor = odrColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurTextColor = odrColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurRowColor = odrColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurEditRowColor = odrColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurCellColor = odrColors->child(5)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrPlayRowColor = odrColors->child(6)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrSelCellColor = odrColors->child(7)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrHovCellColor = odrColors->child(8)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrRowNumColor = odrColors->child(9)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrHeaderTextColor = odrColors->child(10)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrHeaderRowColor = odrColors->child(11)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrBorderColor = odrColors->child(12)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrBackColor = odrColors->child(13)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* ilistColors = ui->colorsTreeWidget->topLevelItem(2);
	pl->ilistTextColor = ilistColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistBackColor = ilistColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistSelBackColor = ilistColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistHovBackColor = ilistColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistHovSelBackColor = ilistColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* wavColors = ui->colorsTreeWidget->topLevelItem(3);
	pl->wavBackColor = wavColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->wavDrawColor = wavColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();

	configLocked->setPatternEditorHeaderFont(ui->ptnHdFontComboBox->currentFont().family().toStdString());
	configLocked->setPatternEditorHeaderFontSize(ui->ptnHdFontSizeComboBox->currentText().toInt());
	configLocked->setPatternEditorRowsFont(ui->ptnRowFontComboBox->currentFont().family().toStdString());
	configLocked->setPatternEditorRowsFontSize(ui->ptnRowFontSizeComboBox->currentText().toInt());
	configLocked->setOrderListHeaderFont(ui->odrHdFontComboBox->currentFont().family().toStdString());
	configLocked->setOrderListHeaderFontSize(ui->odrHdFontSizeComboBox->currentText().toInt());
	configLocked->setOrderListRowsFont(ui->odrRowFontComboBox->currentFont().family().toStdString());
	configLocked->setOrderListRowsFontSize(ui->odrRowFontSizeComboBox->currentText().toInt());

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
		text = tr("Display row numbers and the playback position on the status bar in hexadecimal.");
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
	case 12:	// Fill 00 to effect value
		text = tr("Fill 00 to effect value column upon entering effect id.");
		break;
	case 13:	// Autoset instrument
		text = tr("Set current instrument upon entering note.");
		break;
	case 14:	// Move cursor by horizontal scroll
		text = tr("Move the cursor position by cell with horizontal scroll bar in the order list and the pattern editor.");
		break;
	default:
		text = "";
		break;
	}
	ui->descPlainTextEdit->setPlainText(tr("Description: %1").arg(text));
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

/*****Formats *****/
void ConfigurationDialog::on_addEnvelopeSetPushButton_clicked()
{
	auto name = tr("Set %1").arg(fmEnvelopeTexts_.size() + 1);
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

void ConfigurationDialog::updateEnvelopeSetUi()
{
	std::sort(fmEnvelopeTexts_.begin(), fmEnvelopeTexts_.end(),
			  [](const FMEnvelopeText& a, const FMEnvelopeText& b) -> bool { return (a.name < b.name); });

	ui->envelopeTypeListWidget->clear();
	for (auto& texts : fmEnvelopeTexts_)
		ui->envelopeTypeListWidget->addItem(
					QString::fromUtf8(texts.name.c_str(), static_cast<int>(texts.name.length())));
}

/***** Keys *****/
void ConfigurationDialog::on_keyboardTypeComboBox_currentIndexChanged(int index)
{
	Q_UNUSED(index)
	bool enableCustomLayoutInterface = ui->keyboardTypeComboBox->currentIndex() == 0;
	ui->lowHighKeysTabWidget->setEnabled(enableCustomLayoutInterface);
	ui->customLayoutResetButton->setEnabled(enableCustomLayoutInterface);
}

void ConfigurationDialog::on_customLayoutResetButton_clicked()
{
	std::unordered_map<std::string, JamKey> QWERTYLayoutMapping = config_.lock()->mappingLayouts.at (Configuration::KeyboardLayout::QWERTY);
	std::unordered_map<std::string, JamKey>::const_iterator QWERTYLayoutMappingIterator = QWERTYLayoutMapping.begin();
	while (QWERTYLayoutMappingIterator != QWERTYLayoutMapping.end()) {
		customLayoutKeysMap.at(QWERTYLayoutMappingIterator->second)->setKeySequence(QKeySequence(QString::fromStdString(QWERTYLayoutMappingIterator->first)));
		QWERTYLayoutMappingIterator++;
	}
}

/***** Appearance *****/
void ConfigurationDialog::on_colorEditPushButton_clicked()
{
	QTreeWidgetItem* item = ui->colorsTreeWidget->currentItem();
	if (item == nullptr || item->parent() == nullptr) return;
	QColorDialog diag(item->data(1, Qt::BackgroundRole).value<QColor>());
	diag.setOption(QColorDialog::ShowAlphaChannel);
	if (diag.exec() == QDialog::Accepted)
		item->setData(1, Qt::BackgroundRole, diag.currentColor());
}

void ConfigurationDialog::on_colorLoadPushButton_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open color scheme"),
												QApplication::applicationDirPath() + "/skins",
												tr("ini file (*.ini)"));
	if (file.isNull()) return;

	if (ColorPaletteHandler::loadPalette(file, palette_)) {
		updateColorTree();
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("An unknown error occurred while loading the color scheme."));
	}
}

void ConfigurationDialog::on_colorSavePushButton_clicked()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Save color scheme"),
												QApplication::applicationDirPath() + "/skins",
												tr("ini file (*.ini)"));
	if (file.isNull()) return;
	if (!file.endsWith(".ini")) file += ".ini";	// For linux

	if (!ColorPaletteHandler::savePalette(file, palette_))
		QMessageBox::critical(this, tr("Error"), tr("Failed to save the color scheme."));
}

void ConfigurationDialog::updateColorTree()
{
	auto* pl = palette_.lock().get();
	QTreeWidgetItem* ptnColors = ui->colorsTreeWidget->topLevelItem(0);
	ptnColors->child(0)->setData(1, Qt::BackgroundRole, pl->ptnDefTextColor);
	ptnColors->child(1)->setData(1, Qt::BackgroundRole, pl->ptnDefStepColor);
	ptnColors->child(2)->setData(1, Qt::BackgroundRole, pl->ptnHl1StepColor);
	ptnColors->child(3)->setData(1, Qt::BackgroundRole, pl->ptnHl2StepColor);
	ptnColors->child(4)->setData(1, Qt::BackgroundRole, pl->ptnCurTextColor);
	ptnColors->child(5)->setData(1, Qt::BackgroundRole, pl->ptnCurStepColor);
	ptnColors->child(6)->setData(1, Qt::BackgroundRole, pl->ptnCurEditStepColor);
	ptnColors->child(7)->setData(1, Qt::BackgroundRole, pl->ptnCurCellColor);
	ptnColors->child(8)->setData(1, Qt::BackgroundRole, pl->ptnPlayStepColor);
	ptnColors->child(9)->setData(1, Qt::BackgroundRole, pl->ptnSelCellColor);
	ptnColors->child(10)->setData(1, Qt::BackgroundRole, pl->ptnHovCellColor);
	ptnColors->child(11)->setData(1, Qt::BackgroundRole, pl->ptnDefStepNumColor);
	ptnColors->child(12)->setData(1, Qt::BackgroundRole, pl->ptnHl1StepNumColor);
	ptnColors->child(13)->setData(1, Qt::BackgroundRole, pl->ptnHl2StepNumColor);
	ptnColors->child(14)->setData(1, Qt::BackgroundRole, pl->ptnNoteColor);
	ptnColors->child(15)->setData(1, Qt::BackgroundRole, pl->ptnInstColor);
	ptnColors->child(16)->setData(1, Qt::BackgroundRole, pl->ptnVolColor);
	ptnColors->child(17)->setData(1, Qt::BackgroundRole, pl->ptnEffColor);
	ptnColors->child(18)->setData(1, Qt::BackgroundRole, pl->ptnErrorColor);
	ptnColors->child(19)->setData(1, Qt::BackgroundRole, pl->ptnHeaderTextColor);
	ptnColors->child(20)->setData(1, Qt::BackgroundRole, pl->ptnHeaderRowColor);
	ptnColors->child(21)->setData(1, Qt::BackgroundRole, pl->ptnMaskColor);
	ptnColors->child(22)->setData(1, Qt::BackgroundRole, pl->ptnBorderColor);
	ptnColors->child(23)->setData(1, Qt::BackgroundRole, pl->ptnMuteColor);
	ptnColors->child(24)->setData(1, Qt::BackgroundRole, pl->ptnUnmuteColor);
	ptnColors->child(25)->setData(1, Qt::BackgroundRole, pl->ptnBackColor);

	QTreeWidgetItem* odrColors = ui->colorsTreeWidget->topLevelItem(1);
	odrColors->child(0)->setData(1, Qt::BackgroundRole, pl->odrDefTextColor);
	odrColors->child(1)->setData(1, Qt::BackgroundRole, pl->odrDefRowColor);
	odrColors->child(2)->setData(1, Qt::BackgroundRole, pl->odrCurTextColor);
	odrColors->child(3)->setData(1, Qt::BackgroundRole, pl->odrCurRowColor);
	odrColors->child(4)->setData(1, Qt::BackgroundRole, pl->odrCurEditRowColor);
	odrColors->child(5)->setData(1, Qt::BackgroundRole, pl->odrCurCellColor);
	odrColors->child(6)->setData(1, Qt::BackgroundRole, pl->odrPlayRowColor);
	odrColors->child(7)->setData(1, Qt::BackgroundRole, pl->odrSelCellColor);
	odrColors->child(8)->setData(1, Qt::BackgroundRole, pl->odrHovCellColor);
	odrColors->child(9)->setData(1, Qt::BackgroundRole, pl->odrRowNumColor);
	odrColors->child(10)->setData(1, Qt::BackgroundRole, pl->odrHeaderTextColor);
	odrColors->child(11)->setData(1, Qt::BackgroundRole, pl->odrHeaderRowColor);
	odrColors->child(12)->setData(1, Qt::BackgroundRole, pl->odrBorderColor);
	odrColors->child(13)->setData(1, Qt::BackgroundRole, pl->odrBackColor);

	QTreeWidgetItem* ilistColors = ui->colorsTreeWidget->topLevelItem(2);
	ilistColors->child(0)->setData(1, Qt::BackgroundRole, pl->ilistTextColor);
	ilistColors->child(1)->setData(1, Qt::BackgroundRole, pl->ilistBackColor);
	ilistColors->child(2)->setData(1, Qt::BackgroundRole, pl->ilistSelBackColor);
	ilistColors->child(3)->setData(1, Qt::BackgroundRole, pl->ilistHovBackColor);
	ilistColors->child(4)->setData(1, Qt::BackgroundRole, pl->ilistHovSelBackColor);

	QTreeWidgetItem* wavColors = ui->colorsTreeWidget->topLevelItem(3);
	wavColors->child(0)->setData(1, Qt::BackgroundRole, pl->wavBackColor);
	wavColors->child(1)->setData(1, Qt::BackgroundRole, pl->wavDrawColor);
}
