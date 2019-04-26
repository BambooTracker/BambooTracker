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

	void addLoop(int begin, int end, int times);

	enum class ReleaseType
	{
		NO_RELEASE,
		FIX,
		ABSOLUTE,
		RELATIVE
	};

	void setRelease(VisualizedInstrumentMacroEditor::ReleaseType type, int point);

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
	void loopChanged(std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void releaseChanged(VisualizedInstrumentMacroEditor::ReleaseType type, int point);

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

private slots:
	void on_colIncrToolButton_clicked();
	void on_colDecrToolButton_clicked();
	void on_verticalScrollBar_valueChanged(int value);
	void on_lineEdit_editingFinished();
	void onLoopChanged();

private:
	Ui::VisualizedInstrumentMacroEditor *ui;

	std::unique_ptr<QPixmap> pixmap_;

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
	int pressRow_, pressCol_;

	int loopY_, releaseY_;
	int loopBaseY_, releaseBaseY_;

	int grabLoop_;
	bool isGrabLoopHead_;
	bool isGrabRelease_;

	struct Loop
	{
		int begin, end, times;
	};

	struct Release
	{
		VisualizedInstrumentMacroEditor::ReleaseType type;
		int point;
	};

	std::vector<Loop> loops_;
	Release release_;

	struct Column
	{
		int row, data;
		QString text;
	};

	std::vector<QString> labels_;
	std::vector<Column> cols_;

	bool isMultiReleaseState_;
	bool isLabelOmitted_;

	int mmlBase_;

	bool isIgnoreEvent_;

	void initDisplay();
	void drawField();
	void drawLoop();
	void drawRelease();
	void drawBorder();
	void drawShadow();

	void makeMML();

	int checkLoopRegion(int col);
	void moveLoop();

	void updateColumnWidth();
	void updateRowHeight();

	inline int getDisplayedRowCount() const
	{
		int labCnt = static_cast<int>(labels_.size());
		return (maxDispRowCnt_ > labCnt) ? labCnt : maxDispRowCnt_;
	}

	inline void scrollUp(int pos)
	{
		upperRow_ = pos - 1 + getDisplayedRowCount();
	}
};

#endif // VISUALIZED_INSTRUMENT_MACRO_EDITOR_HPP
