/*
 * Copyright (C) 2020-2023 Rerrah
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

#include "adpcm_drumkit_editor.hpp"
#include "ui_adpcm_drumkit_editor.h"
#include <QString>
#include <QCoreApplication>
#include "instrument.hpp"
#include "note.hpp"
#include "gui/event_guard.hpp"
#include "gui/jam_layout.hpp"
#include "gui/note_name_manager.hpp"
#include "gui/slider_style.hpp"

namespace
{
const char* PAN_TEXT[] = {
	QT_TRANSLATE_NOOP("Panning", "Left"),
	QT_TRANSLATE_NOOP("Panning", "Center"),
	QT_TRANSLATE_NOOP("Panning", "Right")
};
constexpr int PAN_UI2INTRNL[] = { PanType::LEFT, PanType::CENTER, PanType::RIGHT };

inline int convertPanInternalToUi(int intrPan)
{
	return std::distance(std::begin(PAN_UI2INTRNL),
						 std::find(std::begin(PAN_UI2INTRNL), std::end(PAN_UI2INTRNL), intrPan));
}
}

AdpcmDrumkitEditor::AdpcmDrumkitEditor(int num, QWidget* parent)
	: InstrumentEditor(num, parent),
	  ui(new Ui::AdpcmDrumkitEditor),
	  isIgnoreEvent_(false),
	  hasShown_(false)
{
	ui->setupUi(this);

	ui->keyTreeWidget->header()->setDefaultSectionSize(40);	// Rearrange header widths
	NoteNameManager& nnm = NoteNameManager::getManager();
	for (int i = 0; i < Note::NOTE_NUMBER_RANGE; ++i) {
		ui->keyTreeWidget->addTopLevelItem(
					new QTreeWidgetItem({ nnm.getNoteString(i), "-", "-" }));
	}

	//========== Sample ==========//
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::modified, this, [&] { emit modified(); });
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleNumberChanged,
					 this, [&](int n) {
		bt_.lock()->setInstrumentDrumkitSample(instNum_, ui->keyTreeWidget->currentIndex().row(), n);
		setInstrumentSampleParameters(ui->keyTreeWidget->currentIndex().row());
		emit sampleNumberChanged();
		emit modified();

		if (config_.lock()->getWriteOnlyUsedSamples()) {
			emit sampleAssignRequested();
		}
	}, Qt::DirectConnection);
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleParameterChanged,
					 this, [&](int sampNum) {
		emit sampleParameterChanged(sampNum, instNum_);
	});
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleAssignRequested,
					 this, [&] { emit sampleAssignRequested(); });
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleMemoryChanged,
					 this, [&] { emit sampleMemoryChanged(); });

	//========== Pan ==========//
	ui->panHorizontalSlider->setStyle(SliderStyle::instance());
	ui->panHorizontalSlider->installEventFilter(this);
	ui->panPosLabel->setText(QCoreApplication::translate("Panning", PAN_TEXT[ui->panHorizontalSlider->value()]));
}

AdpcmDrumkitEditor::~AdpcmDrumkitEditor()
{
	delete ui;
}

SoundSource AdpcmDrumkitEditor::getSoundSource() const
{
	return SoundSource::ADPCM;
}

InstrumentType AdpcmDrumkitEditor::getInstrumentType() const
{
	return InstrumentType::Drumkit;
}

void AdpcmDrumkitEditor::updateBySettingCore()
{
	ui->sampleEditor->setCore(bt_);
	updateInstrumentParameters();
}

void AdpcmDrumkitEditor::updateBySettingConfiguration()
{
	ui->sampleEditor->setConfiguration(config_);
}

void AdpcmDrumkitEditor::updateBySettingColorPalette()
{
	ui->sampleEditor->setColorPalette(palette_);
}

void AdpcmDrumkitEditor::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	updateWindowTitle();

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());

	for (const auto& key : instKit->getAssignedKeys()) {
		setInstrumentSampleParameters(key);
	}
}

/********** Events **********/
bool AdpcmDrumkitEditor::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui->panHorizontalSlider) {
		if (event->type() == QEvent::Wheel) {
			auto e = dynamic_cast<QWheelEvent*>(event);
			if (e->angleDelta().y() > 0) ui->panHorizontalSlider->setValue(ui->panHorizontalSlider->value() + 1);
			else if (e->angleDelta().y() < 0) ui->panHorizontalSlider->setValue(ui->panHorizontalSlider->value() - 1);
			return true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

void AdpcmDrumkitEditor::showEvent(QShowEvent*)
{
	if (!hasShown_) {
		ui->keyTreeWidget->setCurrentItem(ui->keyTreeWidget->topLevelItem(Note::DEFAULT_NOTE_NUM));
		ui->keyTreeWidget->scrollTo(ui->keyTreeWidget->model()->index(Note::DEFAULT_NOTE_NUM, 0),
									QAbstractItemView::PositionAtTop);
		int x = config_.lock()->getInstrumentDrumkitWindowHorizontalSplit();
		if (x == -1) {
			config_.lock()->setInstrumentDrumkitWindowHorizontalSplit(ui->splitter->sizes().at(0));
		}
		else {
			ui->splitter->setSizes({ x, ui->splitter->width() - ui->splitter->handleWidth() - x });
		}
	}
	hasShown_ = true;
}

// MUST DIRECT CONNECTION
void AdpcmDrumkitEditor::keyPressEvent(QKeyEvent *event)
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
void AdpcmDrumkitEditor::keyReleaseEvent(QKeyEvent *event)
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
void AdpcmDrumkitEditor::on_keyTreeWidget_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
	Ui::EventGuard eg(isIgnoreEvent_);

	int key = ui->keyTreeWidget->currentIndex().row();
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());

	bool enabled = instKit->getSampleEnabled(key);
	ui->sampleGroupBox->setChecked(enabled);
	if (enabled) {
		setInstrumentSampleParameters(key);
		ui->pitchSpinBox->setValue(instKit->getPitch(key));
		ui->panHorizontalSlider->setValue(convertPanInternalToUi(instKit->getPan(key)));
	}
}

void AdpcmDrumkitEditor::on_splitter_splitterMoved(int pos, int)
{
	config_.lock()->setInstrumentDrumkitWindowHorizontalSplit(pos);
}

//--- Pitch
void AdpcmDrumkitEditor::on_pitchSpinBox_valueChanged(int arg1)
{
	int key = ui->keyTreeWidget->currentIndex().row();
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());

	if (instKit->getSampleEnabled(key)) {
		bt_.lock()->setInstrumentDrumkitPitch(instNum_, key, arg1);
		ui->keyTreeWidget->currentItem()->setText(2, QString::number(arg1));
		emit modified();
	}
}

//--- Pan
void AdpcmDrumkitEditor::on_panHorizontalSlider_valueChanged(int value)
{
	const QString text = QCoreApplication::translate("Panning", PAN_TEXT[value]);
	ui->panPosLabel->setText(text);

	int key = ui->keyTreeWidget->currentIndex().row();
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());

	if (instKit->getSampleEnabled(key)) {
		bt_.lock()->setInstrumentDrumkitPan(instNum_, key, PAN_UI2INTRNL[value]);
		ui->keyTreeWidget->currentItem()->setText(3, text);
		emit modified();
	}
}

//--- Sample
void AdpcmDrumkitEditor::setInstrumentSampleParameters(int key)
{
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());
	QTreeWidgetItem* item = ui->keyTreeWidget->topLevelItem(key);

	if (instKit->getSampleEnabled(key)) {
		int sampNum = instKit->getSampleNumber(key);
		ui->sampleEditor->setInstrumentSampleParameters(
					sampNum,
					instKit->isSampleRepeatable(key),
					instKit->getSampleRepeatRange(key),
					instKit->getSampleRootKeyNumber(key),
					instKit->getSampleRootDeltaN(key),
					instKit->getSampleStartAddress(key),
					instKit->getSampleStopAddress(key),
					instKit->getRawSample(key));
		item->setText(1, QString::number(sampNum));
		item->setText(2, QString::number(instKit->getPitch(key)));
		item->setText(3, QCoreApplication::translate("Panning", PAN_TEXT[convertPanInternalToUi(instKit->getPan(key))]));
	}
	else {
		ui->sampleEditor->setInstrumentSampleParameters(
					0,
					bt_.lock()->getSampleADPCMRepeatEnabled(0),
					bt_.lock()->getSampleADPCMRepeatRange(0),
					bt_.lock()->getSampleADPCMRootKeyNumber(0),
					bt_.lock()->getSampleADPCMRootDeltaN(0),
					bt_.lock()->getSampleADPCMStartAddress(0),
					bt_.lock()->getSampleADPCMStopAddress(0),
					bt_.lock()->getSampleADPCMRawSample(0));
		item->setText(1, "-");
		item->setText(2, "-");
		item->setText(3, "-");
	}
}

/********** Slots **********/
void AdpcmDrumkitEditor::onSampleNumberChanged()
{
	ui->sampleEditor->onSampleNumberChanged();
}

void AdpcmDrumkitEditor::onSampleParameterChanged(int sampNum)
{
	if (ui->sampleEditor->getSampleNumber() == sampNum) {
		setInstrumentSampleParameters(ui->keyTreeWidget->currentIndex().row());
	}
}

void AdpcmDrumkitEditor::onSampleMemoryUpdated()
{
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());
	int key = ui->keyTreeWidget->currentIndex().row();

	if (instKit->getSampleEnabled(key)) {
		ui->sampleEditor->onSampleMemoryUpdated(instKit->getSampleStartAddress(key),
												instKit->getSampleStopAddress(key));
	}
	else {
		// Clear addresses
		ui->sampleEditor->onSampleMemoryUpdated(0, 0);
	}
}

void AdpcmDrumkitEditor::on_sampleGroupBox_clicked(bool checked)
{
	Ui::EventGuard eg(isIgnoreEvent_);

	int key = ui->keyTreeWidget->currentIndex().row();
	bt_.lock()->setInstrumentDrumkitSampleEnabled(instNum_, key, checked);
	if (checked) {
		ui->pitchSpinBox->setValue(0);
		ui->panHorizontalSlider->setValue(convertPanInternalToUi(PanType::CENTER));
		setInstrumentSampleParameters(key);
	}
	else {
		// Clear parameters
		ui->sampleEditor->setInstrumentSampleParameters(
					0,
					bt_.lock()->getSampleADPCMRepeatEnabled(0),
					bt_.lock()->getSampleADPCMRepeatRange(0),
					bt_.lock()->getSampleADPCMRootKeyNumber(0),
					bt_.lock()->getSampleADPCMRootDeltaN(0),
					bt_.lock()->getSampleADPCMStartAddress(0),
					bt_.lock()->getSampleADPCMStopAddress(0),
					bt_.lock()->getSampleADPCMRawSample(0));

		auto item = ui->keyTreeWidget->currentItem();
		item->setText(1, "-");
		item->setText(2, "-");
		item->setText(3, "-");
	}

	emit sampleNumberChanged();
	emit modified();
}
