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

#include "configuration_dialog.hpp"
#include "ui_configuration_dialog.h"
#include <algorithm>
#include <functional>
#include <map>
#include <QPushButton>
#include <QMenu>
#include <QMessageBox>
#include <QFont>
#include <QColorDialog>
#include <QFileDialog>
#include <QApplication>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include "chip/opna.hpp"
#include "audio/audio_stream.hpp"
#include "midi/midi.hpp"
#include "jamming.hpp"
#include "gui/slider_style.hpp"
#include "gui/fm_envelope_set_edit_dialog.hpp"
#include "gui/note_name_manager.hpp"
#include "gui/gui_utils.hpp"

namespace
{
inline Qt::CheckState toCheckState(bool enabled)
{
	return enabled ? Qt::Checked : Qt::Unchecked;
}

inline bool fromCheckState(Qt::CheckState state)
{
	return (state == Qt::Checked) ? true : false;
}

struct NotationSystemAttribute
{
	QString name, dispName;
	NoteNotationSystem ev;
};

const NotationSystemAttribute NOTATION_SYSS[] = {
	{ "English", QT_TRANSLATE_NOOP("NotationSystem", "English"), NoteNotationSystem::ENGLISH },
	{ "German", QT_TRANSLATE_NOOP("NotationSystem", "German"), NoteNotationSystem::GERMAN }
};
}

ConfigurationDialog::ConfigurationDialog(std::weak_ptr<Configuration> config, std::weak_ptr<ColorPalette> palette,
										 std::weak_ptr<const AudioStream> stream, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ConfigurationDialog),
	  config_(config),
	  refPalette_(palette),
	  stream_(stream)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QObject::connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
					 this, [&] {
		on_ConfigurationDialog_accepted();
		emit applyPressed();
	});

	std::shared_ptr<Configuration> configLocked = config.lock();
	// General //
	// General settings
	auto glfunc = [&](int i, bool enabled, QString desc) {
		QListWidgetItem* item = ui->generalSettingsListWidget->item(i);
		item->setCheckState(toCheckState(enabled));
		item->setData(Qt::UserRole, desc);
	};
	glfunc(0, configLocked->getWarpCursor(),
		   tr("Warp the cursor around the edges of the pattern editor."));
	glfunc(1, configLocked->getWarpAcrossOrders(),
		   tr("Move to previous or next order when reaching top or bottom in the pattern editor."));
	glfunc(2, configLocked->getShowRowNumberInHex(),
		   tr("Display row numbers and the playback position on the status bar in hexadecimal."));
	glfunc(3, configLocked->getShowPreviousNextOrders(),
		   tr("Preview previous and next orders in the pattern editor."));
	glfunc(4, configLocked->getBackupModules(),
		   tr("Create a backup copy of the existing file when saving a module."));
	glfunc(5, configLocked->getDontSelectOnDoubleClick(),
		   tr("Don't select the whole track when double-clicking in the pattern editor."));
	glfunc(6, configLocked->getReverseFMVolumeOrder(),
		   tr("Reverse the order of FM volume so that 00 is the quietest in the pattern editor."));
	glfunc(7, configLocked->getMoveCursorToRight(),
		   tr("Move the cursor to right after entering effects in the pattern editor."));
	glfunc(8, configLocked->getRetrieveChannelState(),
		   tr("Reconstruct the current channel's state from previous orders upon playing."));
	glfunc(9, configLocked->getEnableTranslation(),
		   tr("Translate to your language from the next launch. See readme to check supported languages."));
	glfunc(10, configLocked->getShowFMDetuneAsSigned(),
		   tr("Display FM detune values as signed numbers in the FM envelope editor."));
	glfunc(11, configLocked->getFill00ToEffectValue(),
		   tr("Fill 00 to effect value column upon entering effect id."));
	glfunc(12, configLocked->getMoveCursorByHorizontalScroll(),
		   tr("Move the cursor position by cell with horizontal scroll bar in the order list and the pattern editor."));
	glfunc(13, configLocked->getOverwriteUnusedUneditedPropety(),
		   tr("Overwrite unused and unedited instrument properties on creating new properties. "
			  "When disabled, override unused properties regardless of editing."));
	glfunc(14, configLocked->getWriteOnlyUsedSamples(),
		   tr("Send only ADPCM samples used by instruments to the ADPCM memory. "
			  "Recommend to turn off if you change ADPCM samples frequently due to take the high rewriting cost."));
	glfunc(15, configLocked->getReflectInstrumentNumberChange(),
		   tr("Correspond the instrument number in patterns when the instrument changes its number."));
	glfunc(16, configLocked->getFixJammingVolume(),
		   tr("Set maximum volume during jam mode. When unchecked, the volume is changed by the volume spinbox."));
	glfunc(17, configLocked->getMuteHiddenTracks(),
		   tr("Mute hidden tracks when visibility of tracks is changed."));
	glfunc(18, configLocked->getRestoreTrackVisibility(),
		   tr("Restore the previous track visibility on startup."));

	// Edit settings
	ui->pageJumpLengthSpinBox->setValue(static_cast<int>(configLocked->getPageJumpLength()));

	// Wave view
	ui->waveViewRateSpinBox->setValue(configLocked->getWaveViewFrameRate());

	// Note names
	{
		QSignalBlocker blocker(ui->noteNameComboBox);
		for (const auto& attrib : NOTATION_SYSS) {
			ui->noteNameComboBox->addItem(attrib.dispName);
			if (attrib.ev == configLocked->getNotationSystem()) {
				ui->noteNameComboBox->setCurrentIndex(ui->noteNameComboBox->count() - 1);
			}
		}
	}

	// Keys
	ui->shortcutsTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->shortcutsTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
	std::map<Configuration::ShortcutAction, QString> shortcutsActions = {
		{ Configuration::ShortcutAction::KeyOff, tr("Key off") },
		{ Configuration::ShortcutAction::OctaveUp, tr("Octave up") },
		{ Configuration::ShortcutAction::OctaveDown, tr("Octave down") },
		{ Configuration::ShortcutAction::EchoBuffer, tr("Echo buffer") },
		{ Configuration::ShortcutAction::PlayAndStop, tr("Play and stop") },
		{ Configuration::ShortcutAction::Play, tr("Play") },
		{ Configuration::ShortcutAction::PlayFromStart, tr("Play from start") },
		{ Configuration::ShortcutAction::PlayPattern, tr("Play pattern") },
		{ Configuration::ShortcutAction::PlayFromCursor, tr("Play from cursor") },
		{ Configuration::ShortcutAction::PlayFromMarker, tr("Play from marker") },
		{ Configuration::ShortcutAction::PlayStep, tr("Play step") },
		{ Configuration::ShortcutAction::Stop, tr("Stop") },
		{ Configuration::ShortcutAction::FocusOnPattern, tr("Focus on pattern editor") },
		{ Configuration::ShortcutAction::FocusOnOrder, tr("Focus on order list") },
		{ Configuration::ShortcutAction::FocusOnInstrument, tr("Focus on instrument list") },
		{ Configuration::ShortcutAction::ToggleEditJam, tr("Toggle edit/jam mode") },
		{ Configuration::ShortcutAction::SetMarker, tr("Set marker") },
		{ Configuration::ShortcutAction::PasteMix, tr("Paste and mix") },
		{ Configuration::ShortcutAction::PasteOverwrite, tr("Paste and overwrite") },
		{ Configuration::ShortcutAction::PasteInsert, tr("Paste and insert") },
		{ Configuration::ShortcutAction::SelectAll, tr("Select all") },
		{ Configuration::ShortcutAction::Deselect, tr("Deselect") },
		{ Configuration::ShortcutAction::SelectRow, tr("Select row") },
		{ Configuration::ShortcutAction::SelectColumn, tr("Select column") },
		{ Configuration::ShortcutAction::SelectPattern, tr("Select pattern") },
		{ Configuration::ShortcutAction::SelectOrder, tr("Select order") },
		{ Configuration::ShortcutAction::GoToStep, tr("Go to step") },
		{ Configuration::ShortcutAction::ToggleTrack, tr("Toggle track") },
		{ Configuration::ShortcutAction::SoloTrack, tr("Solo track") },
		{ Configuration::ShortcutAction::Interpolate, tr("Interpolate") },
		{ Configuration::ShortcutAction::Reverse, tr("Reverse") },
		{ Configuration::ShortcutAction::GoToPrevOrder, tr("Go to previous order") },
		{ Configuration::ShortcutAction::GoToNextOrder, tr("Go to next order") },
		{ Configuration::ShortcutAction::ToggleBookmark, tr("Toggle bookmark") },
		{ Configuration::ShortcutAction::PrevBookmark, tr("Previous bookmark") },
		{ Configuration::ShortcutAction::NextBookmark, tr("Next bookmark") },
		{ Configuration::ShortcutAction::DecreaseNote, tr("Transpose, decrease note") },
		{ Configuration::ShortcutAction::IncreaseNote, tr("Transpose, increase note") },
		{ Configuration::ShortcutAction::DecreaseOctave, tr("Transpose, decrease octave") },
		{ Configuration::ShortcutAction::IncreaseOctave, tr("Transpose, increase octave") },
		{ Configuration::ShortcutAction::PrevInstrument, tr("Previous instrument") },
		{ Configuration::ShortcutAction::NextInstrument, tr("Next instrument") },
		{ Configuration::ShortcutAction::MaskInstrument, tr("Mask instrument") },
		{ Configuration::ShortcutAction::MaskVolume, tr("Mask volume") },
		{ Configuration::ShortcutAction::EditInstrument, tr("Edit instrument") },
		{ Configuration::ShortcutAction::FollowMode, tr("Follow mode") },
		{ Configuration::ShortcutAction::DuplicateOrder, tr("Duplicate order") },
		{ Configuration::ShortcutAction::ClonePatterns, tr("Clone patterns") },
		{ Configuration::ShortcutAction::CloneOrder, tr("Clone order") },
		{ Configuration::ShortcutAction::ReplaceInstrument, tr("Replace instrument") },
		{ Configuration::ShortcutAction::ExpandPattern, tr("Expand pattern") },
		{ Configuration::ShortcutAction::ShrinkPattern, tr("Shrink pattern") },
		{ Configuration::ShortcutAction::FineDecreaseValues, tr("Fine decrease values") },
		{ Configuration::ShortcutAction::FineIncreaseValues, tr("Fine increase values") },
		{ Configuration::ShortcutAction::CoarseDecreaseValues, tr("Coarse decrease values") },
		{ Configuration::ShortcutAction::CoarseIncreaseValuse, tr("Coarse increase valuse") },
		{ Configuration::ShortcutAction::ExpandEffect, tr("Expand effect column") },
		{ Configuration::ShortcutAction::ShrinkEffect, tr("Shrink effect column") },
		{ Configuration::ShortcutAction::PrevHighlighted, tr("Previous highlighted step") },
		{ Configuration::ShortcutAction::NextHighlighted, tr("Next highlighted step") },
		{ Configuration::ShortcutAction::IncreasePatternSize, tr("Increase pattern size") },
		{ Configuration::ShortcutAction::DecreasePatternSize, tr("Decrease pattern size") },
		{ Configuration::ShortcutAction::IncreaseEditStep, tr("Increase edit step") },
		{ Configuration::ShortcutAction::DecreaseEditStep, tr("Decrease edit step") },
		{ Configuration::ShortcutAction::DisplayEffectList, tr("Display effect list") },
		{ Configuration::ShortcutAction::PreviousSong, tr("Previous song") },
		{ Configuration::ShortcutAction::NextSong, tr("Next song") },
		{ Configuration::ShortcutAction::JamVolumeUp, tr("Jam volume up") },
		{ Configuration::ShortcutAction::JamVolumeDown, tr("Jam volume down") }
	};
	std::unordered_map<Configuration::ShortcutAction, std::string> shortcuts = configLocked->getShortcuts();
	for (const auto& pair : shortcutsActions) {
		int row = ui->shortcutsTreeWidget->topLevelItemCount();
		auto item = new QTreeWidgetItem();
		item->setText(0, pair.second);
		ui->shortcutsTreeWidget->insertTopLevelItem(row, item);
		auto widget = new QWidget();
		widget->setLayout(new QHBoxLayout());
		auto seq = new QKeySequenceEdit(gui_utils::utf8ToQString(shortcuts.at(pair.first)));
		shortcutsMap_[pair.first] = seq;
		auto button = new QToolButton();
		button->setIcon(QIcon(":/icon/remove_inst"));
		QObject::connect(button, &QToolButton::clicked, seq, &QKeySequenceEdit::clear);
		auto layout = widget->layout();
		layout->setSpacing(0);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(seq);
		layout->addWidget(button);
		ui->shortcutsTreeWidget->setItemWidget(item, 1, widget);
	}

	ui->keyboardTypeComboBox->setCurrentIndex(static_cast<int>(configLocked->getNoteEntryLayout()));
	customLayoutKeysMap_ = {
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
	for (const auto& pair : configLocked->getCustomLayoutKeys()) {
		customLayoutKeysMap_.at(pair.second)->setKeySequence(QKeySequence(gui_utils::utf8ToQString(pair.first)));
	}
	updateNoteNames();

	// Emulation //
	ui->emulatorComboBox->addItem("MAME YM2608", static_cast<int>(chip::OpnaEmulator::Mame));
	ui->emulatorComboBox->addItem("Nuked OPN-Mod", static_cast<int>(chip::OpnaEmulator::Nuked));
	ui->emulatorComboBox->setCurrentIndex(ui->emulatorComboBox->findData(configLocked->getEmulator()));

	// Sound //
	{
		QSignalBlocker blocker(ui->audioApiComboBox);
		int sndApiRow = -1;
		int defSndApiRow = 0;
		for (auto& name : stream.lock()->getAvailableBackends()) {
			ui->audioApiComboBox->addItem(name);
			if (name == QString::fromStdString(configLocked->getSoundAPI()))
				sndApiRow = ui->audioApiComboBox->count() - 1;
			if (name == stream.lock()->getCurrentBackend())
				defSndApiRow = sndApiRow = ui->audioApiComboBox->count() - 1;
		}
		ui->audioApiComboBox->setCurrentIndex((sndApiRow == -1) ? defSndApiRow : sndApiRow);
	}
	on_audioApiComboBox_currentIndexChanged(ui->audioApiComboBox->currentText());

	ui->realChipComboBox->addItem(tr("None"), static_cast<int>(RealChipInterfaceType::NONE));
#ifdef USE_REAL_CHIP
	ui->realChipComboBox->addItem("SCCI", static_cast<int>(RealChipInterfaceType::SCCI));
	ui->realChipComboBox->addItem("C86CTL", static_cast<int>(RealChipInterfaceType::C86CTL));
#endif
	switch (configLocked->getRealChipInterface()) {
	default:	// Fall through
	case RealChipInterfaceType::NONE:	ui->realChipComboBox->setCurrentIndex(0);	break;
	case RealChipInterfaceType::SCCI:	ui->realChipComboBox->setCurrentIndex(1);	break;
	case RealChipInterfaceType::C86CTL:	ui->realChipComboBox->setCurrentIndex(2);	break;
	}

	{
		QSignalBlocker blocker1(ui->midiInputDeviceComboBox), blocker2(ui->midiApiComboBox);
		MidiInterface& midiIntf = MidiInterface::getInstance();
		int midiApiRow = -1;
		int defMidiApiRow = 0;
		for (auto& name : midiIntf.getAvailableApis()) {
			ui->midiApiComboBox->addItem(gui_utils::utf8ToQString(name));
			if (name == configLocked->getMidiAPI())
				midiApiRow = ui->midiApiComboBox->count() - 1;
			if (name == midiIntf.currentApiName())
				defMidiApiRow = midiApiRow = ui->midiApiComboBox->count() - 1;
		}
		ui->midiApiComboBox->setCurrentIndex((midiApiRow == -1) ? defMidiApiRow : midiApiRow);
	}
	onMidiApiChanged(ui->midiApiComboBox->currentText(), false);
	ui->midiInputGroupBox->setChecked(configLocked->getMidiEnabled());

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
	updateColorTreeFrom(palette.lock().get());
	ui->ptnHdFontComboBox->setCurrentFont(QFont(gui_utils::utf8ToQString(configLocked->getPatternEditorHeaderFont())));
	ui->ptnHdFontSizeComboBox->setCurrentText(QString::number(configLocked->getPatternEditorHeaderFontSize()));
	ui->ptnRowFontComboBox->setCurrentFont(QFont(gui_utils::utf8ToQString(configLocked->getPatternEditorRowsFont())));
	ui->ptnRowFontSizeComboBox->setCurrentText(QString::number(configLocked->getPatternEditorRowsFontSize()));
	ui->odrHdFontComboBox->setCurrentFont(QFont(gui_utils::utf8ToQString(configLocked->getOrderListHeaderFont())));
	ui->odrHdFontSizeComboBox->setCurrentText(QString::number(configLocked->getOrderListHeaderFontSize()));
	ui->odrRowFontComboBox->setCurrentFont(QFont(gui_utils::utf8ToQString(configLocked->getOrderListRowsFont())));
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
	configLocked->setFill00ToEffectValue(fromCheckState(ui->generalSettingsListWidget->item(11)->checkState()));
	configLocked->setMoveCursorByHorizontalScroll(fromCheckState(ui->generalSettingsListWidget->item(12)->checkState()));
	configLocked->setOverwriteUnusedUneditedPropety(fromCheckState(ui->generalSettingsListWidget->item(13)->checkState()));
	configLocked->setWriteOnlyUsedSamples(fromCheckState(ui->generalSettingsListWidget->item(14)->checkState()));
	configLocked->setReflectInstrumentNumberChange(fromCheckState(ui->generalSettingsListWidget->item(15)->checkState()));
	configLocked->setFixJammingVolume(fromCheckState(ui->generalSettingsListWidget->item(16)->checkState()));
	configLocked->setMuteHiddenTracks(fromCheckState(ui->generalSettingsListWidget->item(17)->checkState()));
	configLocked->setRestoreTrackVisibility(fromCheckState(ui->generalSettingsListWidget->item(18)->checkState()));

	// Edit settings
	configLocked->setPageJumpLength(static_cast<size_t>(ui->pageJumpLengthSpinBox->value()));

	// Wave view
	configLocked->setWaveViewFrameRate(ui->waveViewRateSpinBox->value());

	// Note names
	configLocked->setNotationSystem(NOTATION_SYSS[ui->noteNameComboBox->currentIndex()].ev);

	// Keys
	std::unordered_map<Configuration::ShortcutAction, std::string> shortcuts;
	for (const auto& pair: shortcutsMap_) {
		shortcuts[pair.first] = pair.second->keySequence().toString().toStdString();
	}
	configLocked->setShortcuts(shortcuts);

	configLocked->setNoteEntryLayout(static_cast<Configuration::KeyboardLayout>(ui->keyboardTypeComboBox->currentIndex()));
	std::unordered_map<std::string, JamKey> customLayoutNewKeys;
	for (const auto& pair : customLayoutKeysMap_) {
		customLayoutNewKeys[pair.second->keySequence().toString().toStdString()] = pair.first;
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
	configLocked->setSoundDevice(ui->audioDeviceComboBox->currentText().toUtf8().toStdString());
	configLocked->setSoundAPI(ui->audioApiComboBox->currentText().toUtf8().toStdString());
	configLocked->setRealChipInterface(static_cast<RealChipInterfaceType>(
										   ui->realChipComboBox->currentData(Qt::UserRole).toInt()));
	configLocked->setMidiEnabled(ui->midiInputGroupBox->isChecked());
	configLocked->setMidiAPI(ui->midiApiComboBox->currentText().toUtf8().toStdString());
	configLocked->setMidiInputPort(ui->midiInputDeviceComboBox->currentData().toString().toUtf8().toStdString());
	configLocked->setSampleRate(ui->sampleRateComboBox->currentData(Qt::UserRole).toUInt());
	configLocked->setBufferLength(static_cast<size_t>(ui->bufferLengthHorizontalSlider->value()));

	// Mixer //
	configLocked->setMixerVolumeMaster(ui->masterMixerSlider->value());
	configLocked->setMixerVolumeFM(ui->fmMixerSlider->value() * 0.1);
	configLocked->setMixerVolumeSSG(ui->ssgMixerSlider->value() * 0.1);

	// Formats //
	std::sort(fmEnvelopeTexts_.begin(), fmEnvelopeTexts_.end(),
			  [](const FMEnvelopeText& a, const FMEnvelopeText& b) -> bool { return (a.name < b.name); });
	configLocked->setFMEnvelopeTexts(fmEnvelopeTexts_);

	// Appearance //
	setPaletteFromColorTree(refPalette_.lock().get());
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
	QString text("");
	if (QListWidgetItem* item = ui->generalSettingsListWidget->currentItem()) {
		text = item->data(Qt::UserRole).toString();
	}
	ui->descPlainTextEdit->setPlainText(tr("Description: %1").arg(text));
}

void ConfigurationDialog::on_noteNameComboBox_currentIndexChanged(int index)
{
	// Change notation system temporary
	NoteNameManager& man = NoteNameManager::getManager();
	man.setNotationSystem(NOTATION_SYSS[index].ev);
	updateNoteNames();

	// Restore global notation system
	for (const auto& attrib : NOTATION_SYSS) {
		if (attrib.ev == config_.lock()->getNotationSystem()) {
			man.setNotationSystem(attrib.ev);
			return;
		}
	}
}

/***** Mixer *****/
void ConfigurationDialog::on_mixerResetPushButton_clicked()
{
	ui->fmMixerSlider->setValue(0);
	ui->ssgMixerSlider->setValue(0);
}

/***** Sound *****/
void ConfigurationDialog::on_audioApiComboBox_currentIndexChanged(const QString &arg1)
{
	ui->audioDeviceComboBox->clear();
	std::vector<QString> devices = stream_.lock()->getAvailableDevices(arg1);
	if (devices.empty()) {
		ui->audioDeviceComboBox->setEnabled(false);
		return;
	}
	else {
		ui->audioDeviceComboBox->setEnabled(true);
	}
	int devRow = -1;
	int defDevRow = 0;
	for (auto& name : devices) {
		ui->audioDeviceComboBox->addItem(name);
		if (name == gui_utils::utf8ToQString(config_.lock()->getSoundDevice()))
			devRow = ui->audioDeviceComboBox->count() - 1;
		if (name == stream_.lock()->getDefaultOutputDevice(arg1))
			defDevRow = ui->audioDeviceComboBox->count() - 1;
	}
	ui->audioDeviceComboBox->setCurrentIndex((devRow == -1) ? defDevRow : devRow);
}

void ConfigurationDialog::on_midiApiComboBox_currentIndexChanged(const QString &arg1)
{
	onMidiApiChanged(arg1);
}

void ConfigurationDialog::onMidiApiChanged(const QString &arg1, bool hasInitialized)
{
	ui->midiInputDeviceComboBox->clear();

	MidiInterface &intf = MidiInterface::getInstance();
	std::vector<std::string> ports;
	bool vport;
	std::string apiName = arg1.toStdString();
	if (intf.currentApiName() == apiName) {
		ports = intf.getRealInputPorts();
		vport = intf.supportsVirtualPort();
	}
	else {
		ports = intf.getRealInputPorts(apiName);
		vport = intf.supportsVirtualPort(apiName);
	}

	int devRow = -1;
	if (vport) {
		ui->midiInputDeviceComboBox->addItem(tr("Virtual port"), QString());
	}
	else if (ports.empty()) {
		ui->midiInputDeviceComboBox->setEnabled(false);
		return;
	}
	if (hasInitialized)	// To reflect unchecked groupbox when displaying the dialog for the first time
		ui->midiInputDeviceComboBox->setEnabled(true);
	for (auto& portName : ports) {
		auto name = QString::fromStdString(portName);
		ui->midiInputDeviceComboBox->addItem(name, name);
		if (portName == config_.lock()->getMidiInputPort())
			devRow = ui->midiInputDeviceComboBox->count() - 1;
	}
	ui->midiInputDeviceComboBox->setCurrentIndex((devRow == -1) ? 0 : devRow);
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
	FMEnvelopeSetEditDialog dialog(fmEnvelopeTexts_.at(row).texts, this);
	dialog.setWindowTitle(dialog.windowTitle() + ": " + ui->envelopeSetNameLineEdit->text());
	if (dialog.exec() == QDialog::Accepted) {
		fmEnvelopeTexts_.at(row).texts = dialog.getSet();
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
		ui->envelopeTypeListWidget->addItem(gui_utils::utf8ToQString(texts.name));
}

/***** Keys *****/
void ConfigurationDialog::on_keyboardTypeComboBox_currentIndexChanged(int)
{
	bool enableCustomLayoutInterface = ui->keyboardTypeComboBox->currentIndex() == 0;
	ui->lowHighKeysTabWidget->setEnabled(enableCustomLayoutInterface);
	ui->customLayoutResetButton->setEnabled(enableCustomLayoutInterface);
}

void ConfigurationDialog::on_customLayoutResetButton_clicked()
{
	std::unordered_map<std::string, JamKey> QWERTYLayoutMapping = config_.lock()->mappingLayouts.at (Configuration::KeyboardLayout::QWERTY);
	std::unordered_map<std::string, JamKey>::const_iterator QWERTYLayoutMappingIterator = QWERTYLayoutMapping.begin();
	while (QWERTYLayoutMappingIterator != QWERTYLayoutMapping.end()) {
		customLayoutKeysMap_.at(QWERTYLayoutMappingIterator->second)->setKeySequence(QKeySequence(QString::fromStdString(QWERTYLayoutMappingIterator->first)));
		QWERTYLayoutMappingIterator++;
	}
}

void ConfigurationDialog::addShortcutItem(QString action, std::string shortcut)
{
	int row = ui->shortcutsTreeWidget->topLevelItemCount();
	auto titem = new QTreeWidgetItem();
	titem->setText(0, action);
	ui->shortcutsTreeWidget->insertTopLevelItem(row, titem);
	ui->shortcutsTreeWidget->setItemWidget(titem, 1, new QKeySequenceEdit(gui_utils::utf8ToQString(shortcut)));
}

std::string ConfigurationDialog::getShortcutString(int row) const
{
	return qobject_cast<QKeySequenceEdit*>(
				ui->shortcutsTreeWidget->itemWidget(ui->shortcutsTreeWidget->topLevelItem(row), 1)
				)->keySequence().toString().toStdString();
}

void ConfigurationDialog::updateNoteNames()
{
	const NoteNameManager& man = NoteNameManager::getManager();
	ui->lowCLabel->setText(man.getNoteName(0));
	ui->lowCSLabel->setText(man.getNoteName(1));
	ui->lowDLabel->setText(man.getNoteName(2));
	ui->lowDSLabel->setText(man.getNoteName(3));
	ui->lowELabel->setText(man.getNoteName(4));
	ui->lowFLabel->setText(man.getNoteName(5));
	ui->lowFSLabel->setText(man.getNoteName(6));
	ui->lowGLabel->setText(man.getNoteName(7));
	ui->lowGSLabel->setText(man.getNoteName(8));
	ui->lowALabel->setText(man.getNoteName(9));
	ui->lowASLabel->setText(man.getNoteName(10));
	ui->lowBLabel->setText(man.getNoteName(11));
	ui->lowHighCLabel->setText(man.getNoteName(0));
	ui->lowHighCLabel->setText(man.getNoteName(1));
	ui->lowHighCSLabel->setText(man.getNoteName(2));
	ui->lowHighDLabel->setText(man.getNoteName(3));
	ui->highCLabel->setText(man.getNoteName(0));
	ui->highCSLabel->setText(man.getNoteName(1));
	ui->highDLabel->setText(man.getNoteName(2));
	ui->highDSLabel->setText(man.getNoteName(3));
	ui->highELabel->setText(man.getNoteName(4));
	ui->highFLabel->setText(man.getNoteName(5));
	ui->highFSLabel->setText(man.getNoteName(6));
	ui->highGLabel->setText(man.getNoteName(7));
	ui->highGSLabel->setText(man.getNoteName(8));
	ui->highALabel->setText(man.getNoteName(9));
	ui->highASLabel->setText(man.getNoteName(10));
	ui->highBLabel->setText(man.getNoteName(11));
	ui->highHighCLabel->setText(man.getNoteName(0));
	ui->highHighCLabel->setText(man.getNoteName(1));
	ui->highHighCSLabel->setText(man.getNoteName(2));
	ui->highHighDLabel->setText(man.getNoteName(3));
}

/***** Appearance *****/
void ConfigurationDialog::on_colorEditPushButton_clicked()
{
	QTreeWidgetItem* item = ui->colorsTreeWidget->currentItem();
	if (item == nullptr || item->parent() == nullptr) return;
	QColorDialog dialog(item->data(1, Qt::BackgroundRole).value<QColor>(), this);
	dialog.setOption(QColorDialog::ShowAlphaChannel);
	if (dialog.exec() == QDialog::Accepted)
		item->setData(1, Qt::BackgroundRole, dialog.currentColor());
}

void ConfigurationDialog::on_colorLoadPushButton_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open color scheme"),
												QApplication::applicationDirPath() + "/skins",
												tr("ini file (*.ini)") + ";;" + tr("All files (*)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
												, QFileDialog::DontUseNativeDialog
#endif
												);
	if (file.isNull()) return;

	ColorPalette palette;
	if (io::loadPalette(file, &palette)) {
		updateColorTreeFrom(&palette);
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("An unknown error occurred while loading the color scheme."));
	}
}

void ConfigurationDialog::on_colorSavePushButton_clicked()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Save color scheme"),
												QApplication::applicationDirPath() + "/skins",
												tr("ini file (*.ini)") + ";;" + tr("All files (*)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
												, QFileDialog::DontUseNativeDialog
#endif
												);
	if (file.isNull()) return;
	if (!file.endsWith(".ini")) file += ".ini";	// For linux

	ColorPalette palette;
	setPaletteFromColorTree(&palette);
	if (!io::savePalette(file, &palette))
		QMessageBox::critical(this, tr("Error"), tr("Failed to save the color scheme."));
}

void ConfigurationDialog::updateColorTreeFrom(const ColorPalette* const palette)
{
	QTreeWidgetItem* ptnColors = ui->colorsTreeWidget->topLevelItem(0);
	ptnColors->child(0)->setData(1, Qt::BackgroundRole, palette->ptnDefTextColor);
	ptnColors->child(1)->setData(1, Qt::BackgroundRole, palette->ptnDefStepColor);
	ptnColors->child(2)->setData(1, Qt::BackgroundRole, palette->ptnHl1StepColor);
	ptnColors->child(3)->setData(1, Qt::BackgroundRole, palette->ptnHl2StepColor);
	ptnColors->child(4)->setData(1, Qt::BackgroundRole, palette->ptnCurTextColor);
	ptnColors->child(5)->setData(1, Qt::BackgroundRole, palette->ptnCurStepColor);
	ptnColors->child(6)->setData(1, Qt::BackgroundRole, palette->ptnCurEditStepColor);
	ptnColors->child(7)->setData(1, Qt::BackgroundRole, palette->ptnCurCellColor);
	ptnColors->child(8)->setData(1, Qt::BackgroundRole, palette->ptnPlayStepColor);
	ptnColors->child(9)->setData(1, Qt::BackgroundRole, palette->ptnSelCellColor);
	ptnColors->child(10)->setData(1, Qt::BackgroundRole, palette->ptnHovCellColor);
	ptnColors->child(11)->setData(1, Qt::BackgroundRole, palette->ptnDefStepNumColor);
	ptnColors->child(12)->setData(1, Qt::BackgroundRole, palette->ptnHl1StepNumColor);
	ptnColors->child(13)->setData(1, Qt::BackgroundRole, palette->ptnHl2StepNumColor);
	ptnColors->child(14)->setData(1, Qt::BackgroundRole, palette->ptnNoteColor);
	ptnColors->child(15)->setData(1, Qt::BackgroundRole, palette->ptnInstColor);
	ptnColors->child(16)->setData(1, Qt::BackgroundRole, palette->ptnVolColor);
	ptnColors->child(17)->setData(1, Qt::BackgroundRole, palette->ptnEffColor);
	ptnColors->child(18)->setData(1, Qt::BackgroundRole, palette->ptnErrorColor);
	ptnColors->child(19)->setData(1, Qt::BackgroundRole, palette->ptnHeaderTextColor);
	ptnColors->child(20)->setData(1, Qt::BackgroundRole, palette->ptnHeaderRowColor);
	ptnColors->child(21)->setData(1, Qt::BackgroundRole, palette->ptnMaskColor);
	ptnColors->child(22)->setData(1, Qt::BackgroundRole, palette->ptnBorderColor);
	ptnColors->child(23)->setData(1, Qt::BackgroundRole, palette->ptnHeaderBorderColor);
	ptnColors->child(24)->setData(1, Qt::BackgroundRole, palette->ptnMuteColor);
	ptnColors->child(25)->setData(1, Qt::BackgroundRole, palette->ptnUnmuteColor);
	ptnColors->child(26)->setData(1, Qt::BackgroundRole, palette->ptnBackColor);
	ptnColors->child(27)->setData(1, Qt::BackgroundRole, palette->ptnMarkerColor);
	ptnColors->child(28)->setData(1, Qt::BackgroundRole, palette->ptnUnfocusedShadowColor);

	QTreeWidgetItem* odrColors = ui->colorsTreeWidget->topLevelItem(1);
	odrColors->child(0)->setData(1, Qt::BackgroundRole, palette->odrDefTextColor);
	odrColors->child(1)->setData(1, Qt::BackgroundRole, palette->odrDefRowColor);
	odrColors->child(2)->setData(1, Qt::BackgroundRole, palette->odrCurTextColor);
	odrColors->child(3)->setData(1, Qt::BackgroundRole, palette->odrCurRowColor);
	odrColors->child(4)->setData(1, Qt::BackgroundRole, palette->odrCurEditRowColor);
	odrColors->child(5)->setData(1, Qt::BackgroundRole, palette->odrCurCellColor);
	odrColors->child(6)->setData(1, Qt::BackgroundRole, palette->odrPlayRowColor);
	odrColors->child(7)->setData(1, Qt::BackgroundRole, palette->odrSelCellColor);
	odrColors->child(8)->setData(1, Qt::BackgroundRole, palette->odrHovCellColor);
	odrColors->child(9)->setData(1, Qt::BackgroundRole, palette->odrRowNumColor);
	odrColors->child(10)->setData(1, Qt::BackgroundRole, palette->odrHeaderTextColor);
	odrColors->child(11)->setData(1, Qt::BackgroundRole, palette->odrHeaderRowColor);
	odrColors->child(12)->setData(1, Qt::BackgroundRole, palette->odrBorderColor);
	odrColors->child(13)->setData(1, Qt::BackgroundRole, palette->odrHeaderBorderColor);
	odrColors->child(14)->setData(1, Qt::BackgroundRole, palette->odrBackColor);
	odrColors->child(15)->setData(1, Qt::BackgroundRole, palette->odrUnfocusedShadowColor);

	QTreeWidgetItem* ilistColors = ui->colorsTreeWidget->topLevelItem(2);
	ilistColors->child(0)->setData(1, Qt::BackgroundRole, palette->ilistTextColor);
	ilistColors->child(1)->setData(1, Qt::BackgroundRole, palette->ilistBackColor);
	ilistColors->child(2)->setData(1, Qt::BackgroundRole, palette->ilistSelBackColor);
	ilistColors->child(3)->setData(1, Qt::BackgroundRole, palette->ilistHovBackColor);
	ilistColors->child(4)->setData(1, Qt::BackgroundRole, palette->ilistHovSelBackColor);

	QTreeWidgetItem* wavColors = ui->colorsTreeWidget->topLevelItem(3);
	wavColors->child(0)->setData(1, Qt::BackgroundRole, palette->wavBackColor);
	wavColors->child(1)->setData(1, Qt::BackgroundRole, palette->wavDrawColor);
}

void ConfigurationDialog::setPaletteFromColorTree(ColorPalette* const palette)
{
	QTreeWidgetItem* ptnColors = ui->colorsTreeWidget->topLevelItem(0);
	palette->ptnDefTextColor = ptnColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnDefStepColor = ptnColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHl1StepColor = ptnColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHl2StepColor = ptnColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnCurTextColor = ptnColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnCurStepColor = ptnColors->child(5)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnCurEditStepColor = ptnColors->child(6)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnCurCellColor = ptnColors->child(7)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnPlayStepColor = ptnColors->child(8)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnSelCellColor = ptnColors->child(9)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHovCellColor = ptnColors->child(10)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnDefStepNumColor = ptnColors->child(11)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHl1StepNumColor = ptnColors->child(12)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHl2StepNumColor = ptnColors->child(13)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnNoteColor = ptnColors->child(14)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnInstColor = ptnColors->child(15)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnVolColor = ptnColors->child(16)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnEffColor = ptnColors->child(17)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnErrorColor = ptnColors->child(18)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHeaderTextColor = ptnColors->child(19)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHeaderRowColor = ptnColors->child(20)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnMaskColor = ptnColors->child(21)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnBorderColor = ptnColors->child(22)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnHeaderBorderColor = ptnColors->child(23)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnMuteColor = ptnColors->child(24)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnUnmuteColor = ptnColors->child(25)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnBackColor = ptnColors->child(26)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnMarkerColor = ptnColors->child(27)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ptnUnfocusedShadowColor = ptnColors->child(28)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* odrColors = ui->colorsTreeWidget->topLevelItem(1);
	palette->odrDefTextColor = odrColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrDefRowColor = odrColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrCurTextColor = odrColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrCurRowColor = odrColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrCurEditRowColor = odrColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrCurCellColor = odrColors->child(5)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrPlayRowColor = odrColors->child(6)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrSelCellColor = odrColors->child(7)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrHovCellColor = odrColors->child(8)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrRowNumColor = odrColors->child(9)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrHeaderTextColor = odrColors->child(10)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrHeaderRowColor = odrColors->child(11)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrBorderColor = odrColors->child(12)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrHeaderBorderColor = odrColors->child(13)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrBackColor = odrColors->child(14)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->odrUnfocusedShadowColor = odrColors->child(15)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* ilistColors = ui->colorsTreeWidget->topLevelItem(2);
	palette->ilistTextColor = ilistColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ilistBackColor = ilistColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ilistSelBackColor = ilistColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ilistHovBackColor = ilistColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->ilistHovSelBackColor = ilistColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* wavColors = ui->colorsTreeWidget->topLevelItem(3);
	palette->wavBackColor = wavColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	palette->wavDrawColor = wavColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
}
