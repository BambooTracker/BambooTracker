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

#ifndef INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP
#define INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP

#include <memory>
#include <QDialog>
#include <QKeyEvent>
#include <QTreeWidgetItem>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "jamming.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class InstrumentEditorDrumkitForm;
}

class InstrumentEditorDrumkitForm : public QDialog
{
	Q_OBJECT

public:
	explicit InstrumentEditorDrumkitForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorDrumkitForm() override;
	void setInstrumentNumber(int num);
	int getInstrumentNumber() const;
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);
signals:
	void jamKeyOnEvent(JamKey key);
	void jamKeyOffEvent(JamKey key);
	void modified();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void showEvent(QShowEvent*) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::InstrumentEditorDrumkitForm *ui;
	int instNum_;
	bool isIgnoreEvent_;
	bool hasShown_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	void updateInstrumentParameters();

private slots:
	void on_keyTreeWidget_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
	void on_splitter_splitterMoved(int pos, int);

	//========== Pitch ==========//
private slots:
	void on_pitchSpinBox_valueChanged(int arg1);

	//========== Pan ==========//
private slots:
	void on_panHorizontalSlider_valueChanged(int value);

	//========== Sample ==========//
signals:
	void sampleNumberChanged();
	void sampleParameterChanged(int sampNum, int fromInstNum);
	void sampleAssignRequested();
	void sampleMemoryChanged();

public slots:
	void onSampleNumberChanged();
	void onSampleParameterChanged(int sampNum);
	void onSampleMemoryUpdated();

private slots:
	void on_sampleGroupBox_clicked(bool checked);

private:
	void setInstrumentSampleParameters(int key);
};

#endif // INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP
