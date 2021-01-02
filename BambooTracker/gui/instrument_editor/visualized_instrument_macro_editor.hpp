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

#ifndef VISUALIZED_INSTRUMENT_MACRO_EDITOR_HPP
#define VISUALIZED_INSTRUMENT_MACRO_EDITOR_HPP

#include <QWidget>
#include <QObject>
#include <QPixmap>
#include <QFont>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QColor>
#include <QString>
#include <vector>
#include <memory>
#include "instrument/command_sequence.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class VisualizedInstrumentMacroEditor;
}

class VisualizedInstrumentMacroEditor : public QWidget
{
	Q_OBJECT

public:
	explicit VisualizedInstrumentMacroEditor(QWidget *parent = nullptr);
	~VisualizedInstrumentMacroEditor() override;

	void setColorPalette(std::shared_ptr<ColorPalette> palette);

	void AddRow(QString label = "", bool fitLabelWidth = true);
	void setMaximumDisplayedRowCount(int count);
	void setDefaultRow(int row);
	int getSequenceLength() const;

	void setSequenceCommand(int row, int col, QString str = "", int data = -1);
	void setText(int col, QString text);
	void setData(int col, int data);

	int getSequenceAt(int col) const;
	int getSequenceDataAt(int col) const;

	void setMultipleReleaseState(bool enabled);

	void addSequenceCommand(int row, QString str = "", int data = -1);
	void removeSequenceCommand();
	void clearSequenceCommand();

	void addLoop(int begin, int end, int times);

	void setSequenceType(SequenceType type);

	enum PermittedReleaseFlag : int
	{
		NO_RELEASE = 0,
		FIXED_RELEASE = 1,
		ABSOLUTE_RELEASE = 2,
		RELATIVE_RELEASE = 4
	};

	void setPermittedReleaseTypes(int types);
	void setRelease(const InstrumentSequenceRelease& release);
	// TODO: DEPRECATED
	void setRelease(PermittedReleaseFlag type, int point);

	void clearData();
	void clearRow();

	void setUpperRow(int row);

	void setLabel(int row, QString text);
	void clearAllLabelText();
	void setLabelDiaplayMode(bool isOmitted);
	void autoFitLabelWidth();

	void setMMLDisplay0As(int n);

signals:
	void sequenceCommandChanged(int row, int col);
	void sequenceCommandAdded(int row, int col);
	void sequenceCommandRemoved();
	void loopCleared();
	void loopAdded(InstrumentSequenceLoop loop);
	void loopRemoved(int begin, int end);
	void loopChangedImproved(int prevBegin, int prevEnd, InstrumentSequenceLoop loop);
	void releaseChangedImproved(InstrumentSequenceRelease release);
	// TODO: DEPRECATED
	void loopChanged(std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void releaseChanged(PermittedReleaseFlag type, int point);
	//===============

protected:
	bool eventFilter(QObject*object, QEvent* event) override;
	void paintEventInView(QPaintEvent* event);
	void resizeEventInView(QResizeEvent* event);
	void mousePressEventInView(QMouseEvent* event);
	void mouseReleaseEventInView(QMouseEvent* event);
	void mouseMoveEventInView();
	void mouseHoverdEventInView(QHoverEvent* event);
	void leaveEventInView();
	void wheelEventInView(QWheelEvent* event);

	QPixmap pixmap_;
	std::shared_ptr<ColorPalette> palette_;

	QFont font_;
	QFontMetrics met_;
	int fontWidth_, fontHeight_, fontAscend_, fontLeading_;
	int labWidth_;
	std::vector<int> rowHeights_, colWidths_;
	int rowHeight_;
	int fieldHeight_;

	int maxDispRowCnt_;
	int upperRow_, defaultRow_;

	int hovRow_, hovCol_;

	virtual void drawField();
	virtual void updateLabels();

	SequenceType type_;

	struct Column
	{
		int row, data;
		QString text;
	};

	std::vector<QString> labels_;
	std::vector<Column> cols_;

	int permittedReleaseType_;

	bool isLabelOmitted_;

	virtual int detectRowNumberForMouseEvent(int col, int internalRow) const;

	struct Loop
	{
		int begin, end, times;
	};

	std::vector<Loop> loops_;
	InstrumentSequenceRelease release_;


	void printMML();
	virtual QString convertSequenceDataUnitToMML(Column col);
	virtual int maxInMML() const;
	void interpretMML();
	bool interpretSlopeInMML(QString& text, int& cnt, std::vector<Column>& column);
	virtual bool interpretDataInMML(QString& text, int& cnt, std::vector<Column>& column);

	int panelWidth() const;

	inline int getDisplayedRowCount() const
	{
		int labCnt = static_cast<int>(labels_.size());
		return (maxDispRowCnt_ > labCnt) ? labCnt : maxDispRowCnt_;
	}

private slots:
	void on_colIncrToolButton_clicked();
	void on_colDecrToolButton_clicked();
	void on_verticalScrollBar_valueChanged(int value);
	void on_lineEdit_editingFinished();
	// TODO: DEPRECATED
	void onLoopChanged();

private:
	Ui::VisualizedInstrumentMacroEditor *ui;

	int pressRow_, pressCol_;
	int prevPressRow_, prevPressCol_;

	int loopY_, releaseY_;
	int loopBaseY_, releaseBaseY_;

	int grabLoop_;
	bool isGrabLoopHead_;
	bool isGrabRelease_;

	bool isMultiReleaseState_;

	int mmlBase_;

	bool isIgnoreEvent_;

	void initDisplay();

	void drawLoop();
	void drawRelease();
	void drawBorder();
	void drawShadow();

	int checkLoopRegion(int col);
	void moveLoop();

	void updateColumnWidth();
	void updateRowHeight();

	inline void scrollUp(int pos)
	{
		upperRow_ = pos - 1 + getDisplayedRowCount();
	}
};

#endif // VISUALIZED_INSTRUMENT_MACRO_EDITOR_HPP
