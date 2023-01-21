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

#ifndef ADPCM_SAMPLE_EDITOR_HPP
#define ADPCM_SAMPLE_EDITOR_HPP

#include <memory>
#include <cstdint>
#include <QWidget>
#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPixmap>
#include <QString>
#include <QPoint>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "instrument/sample_repeat.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class ADPCMSampleEditor;
}

class ADPCMSampleEditor : public QWidget
{
	Q_OBJECT

public:
	explicit ADPCMSampleEditor(QWidget *parent = nullptr);
	~ADPCMSampleEditor() override;
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

	int getSampleNumber() const;

	void setInstrumentSampleParameters(int sampNum, bool repeatable, const SampleRepeatRange& repeatRange,
									   int rKeyNum, int rDeltaN, size_t start, size_t stop, std::vector<uint8_t> sample);

signals:
	void modified();
	void sampleNumberChanged(int n);	// NEED Direct connection
	void sampleParameterChanged(int wfNum);
	void sampleAssignRequested();
	void sampleMemoryChanged();

public slots:
	void onSampleNumberChanged();
	void onSampleMemoryUpdated(size_t start, size_t stop);
	void onNoteNamesUpdated();

protected:
	bool eventFilter(QObject* obj, QEvent* ev) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private:
	Ui::ADPCMSampleEditor *ui;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	QPixmap memPixmap_, sampViewPixmap_;
	int zoom_, viewedSampLen_, gridIntr_;
	QPoint cursorSamp_, prevPressedSamp_;

	size_t addrStart_, addrStop_;
	std::vector<int16_t> sample_;

	void importSampleFrom(const QString file);
	void updateSampleMemoryBar();
	void updateSampleView();
	void updateUsersView();

	void detectCursorSamplePosition(int cx, int cy);

	void sendEditedSample();

	inline QString updateDetailView() const
	{
		return QString("(%1, %2), %3, x%4")
				.arg(cursorSamp_.x()).arg(cursorSamp_.y()).arg(sample_.size()).arg(zoom_ + 1);
	}

	enum class DrawMode { Disabled, Normal, Direct };
	DrawMode drawMode_;

private slots:
	void on_sampleNumSpinBox_valueChanged(int arg1);
	void on_repeatCheckBox_toggled(bool checked);
	void on_repeatBeginSpinBox_valueChanged(int);
	void on_repeatEndSpinBox_valueChanged(int);
	void on_rootRateSpinBox_valueChanged(int arg1);
	void on_action_Resize_triggered();
	void on_actionRe_verse_triggered();
	void on_actionZoom_In_triggered();
	void on_actionZoom_Out_triggered();
	void on_horizontalScrollBar_valueChanged(int);
	void on_action_Import_triggered();
	void on_action_Clear_triggered();
	void on_action_Grid_View_triggered();
	void on_actionG_rid_Settings_triggered();
	void on_action_Draw_Sample_triggered(bool checked);
	void on_actionDirec_t_Draw_triggered(bool checked);
};

#endif // ADPCM_SAMPLE_EDITOR_HPP
