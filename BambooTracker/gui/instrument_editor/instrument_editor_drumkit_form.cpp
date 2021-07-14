/*
 * Copyright (C) 2020-2021 Rerrah
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

#include "instrument_editor_drumkit_form.hpp"
#include "ui_instrument_editor_drumkit_form.h"
#include <QString>
#include "instrument.hpp"
#include "note.hpp"
#include "gui/event_guard.hpp"
#include "gui/jam_layout.hpp"
#include "gui/note_name_manager.hpp"
#include "gui/gui_utils.hpp"
#include "gui/slider_style.hpp"

namespace
{
const QString PAN_TEXT[] = {
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

InstrumentEditorDrumkitForm::InstrumentEditorDrumkitForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorDrumkitForm),
	instNum_(num),
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
	ui->panHorizontalSlider->setStyle(new SliderStyle);
	ui->panHorizontalSlider->installEventFilter(this);
	ui->panPosLabel->setText(PAN_TEXT[ui->panHorizontalSlider->value()]);
}

InstrumentEditorDrumkitForm::~InstrumentEditorDrumkitForm()
{
	delete ui;
}

void InstrumentEditorDrumkitForm::setInstrumentNumber(int num)
{
	instNum_ = num;
}

int InstrumentEditorDrumkitForm::getInstrumentNumber() const
{
	return instNum_;
}

void InstrumentEditorDrumkitForm::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
	ui->sampleEditor->setCore(core);
	updateInstrumentParameters();
}

void InstrumentEditorDrumkitForm::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
	ui->sampleEditor->setConfiguration(config);
}

void InstrumentEditorDrumkitForm::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
	ui->sampleEditor->setColorPalette(palette);
}

void InstrumentEditorDrumkitForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());
	auto name = gui_utils::utf8ToQString(instKit->getName());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	for (const auto& key : instKit->getAssignedKeys()) {
		setInstrumentSampleParameters(key);
	}
}

/********** Events **********/
bool InstrumentEditorDrumkitForm::eventFilter(QObject* watched, QEvent* event)
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

void InstrumentEditorDrumkitForm::showEvent(QShowEvent*)
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

	bool enabled = instKit->getSampleEnabled(key);
	ui->sampleGroupBox->setChecked(enabled);
	if (enabled) {
		setInstrumentSampleParameters(key);
		ui->pitchSpinBox->setValue(instKit->getPitch(key));
		ui->panHorizontalSlider->setValue(convertPanInternalToUi(instKit->getPan(key)));
	}
}

void InstrumentEditorDrumkitForm::on_splitter_splitterMoved(int pos, int)
{
	config_.lock()->setInstrumentDrumkitWindowHorizontalSplit(pos);
}

//--- Pitch
void InstrumentEditorDrumkitForm::on_pitchSpinBox_valueChanged(int arg1)
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
void InstrumentEditorDrumkitForm::on_panHorizontalSlider_valueChanged(int value)
{
	const QString text = PAN_TEXT[value];
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
void InstrumentEditorDrumkitForm::setInstrumentSampleParameters(int key)
{
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instKit = dynamic_cast<InstrumentDrumkit*>(inst.get());
	QTreeWidgetItem* item = ui->keyTreeWidget->topLevelItem(key);

	if (instKit->getSampleEnabled(key)) {
		int sampNum = instKit->getSampleNumber(key);
		ui->sampleEditor->setInstrumentSampleParameters(
					sampNum, instKit->isSampleRepeatable(key),
					instKit->getSampleRootKeyNumber(key), instKit->getSampleRootDeltaN(key),
					instKit->getSampleStartAddress(key), instKit->getSampleStopAddress(key),
					instKit->getRawSample(key));
		item->setText(1, QString::number(sampNum));
		item->setText(2, QString::number(instKit->getPitch(key)));
		item->setText(3, PAN_TEXT[convertPanInternalToUi(instKit->getPan(key))]);
	}
	else {
		ui->sampleEditor->setInstrumentSampleParameters(
					0, bt_.lock()->getSampleADPCMRepeatEnabled(0),
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
void InstrumentEditorDrumkitForm::onSampleNumberChanged()
{
	ui->sampleEditor->onSampleNumberChanged();
}

void InstrumentEditorDrumkitForm::onSampleParameterChanged(int sampNum)
{
	if (ui->sampleEditor->getSampleNumber() == sampNum) {
		setInstrumentSampleParameters(ui->keyTreeWidget->currentIndex().row());
	}
}

void InstrumentEditorDrumkitForm::onSampleMemoryUpdated()
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

void InstrumentEditorDrumkitForm::on_sampleGroupBox_clicked(bool checked)
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
					0, bt_.lock()->getSampleADPCMRepeatEnabled(0),
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
