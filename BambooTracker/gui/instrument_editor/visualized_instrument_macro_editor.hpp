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

namespace Ui {
	class VisualizedInstrumentMacroEditor;
}

class VisualizedInstrumentMacroEditor : public QWidget
{
	Q_OBJECT

public:
	explicit VisualizedInstrumentMacroEditor(QWidget *parent = nullptr);
	~VisualizedInstrumentMacroEditor() override;

	void AddRow(QString label = "");
	void setMaximumDisplayedRowCount(int count);
	void setDefaultRow(int row);
	int getSequenceLength() const;

	void setSequenceCommand(int row, int col, QString str = "", int data = -1);
	void setText(int col, QString text);
	void setData(int col, int data);

	int getSequenceAt(int col) const;
	int getSequenceDataAt(int col) const;

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

	void clear();

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
	void onLoopChanged();

private:
	Ui::VisualizedInstrumentMacroEditor *ui;

	std::unique_ptr<QPixmap> pixmap_;

	QFont font_;
	int fontWidth_, fontHeight_, fontAscend_, fontLeading_;
	int tagWidth_;
	int rowHeight_, colWidth_;
	int fieldHeight_;

	int maxDispRowCnt_, colCnt_;
	int upperRow_, defaultRow_;

	int hovRow_, hovCol_;
	int pressRow_, pressCol_;

	int loopY_, releaseY_;
	int loopBaseY_, releaseBaseY_;

	QColor tagColor_;
	QColor hovColor_;
	QColor loopBackColor_, loopColor_, loopEdgeColor_;
	QColor releaseBackColor_, releaseColor_, releaseEdgeColor_;
	QColor loopFontColor_, releaseFontColor_;
	QColor cellColor_, cellTextColor_;
	QColor borderColor_;
	QColor maskColor_;

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

	bool isIgnoreEvent_;

	void initDisplay();
	void drawField();
	void drawLoop();
	void drawRelease();
	void drawBorder();
	void drawShadow();

	int checkLoopRegion(int col);
	void moveLoop();

	inline void scrollUp(int pos)
	{
		upperRow_ = maxDispRowCnt_ + pos - 1;
	}

	inline void updateColumnWidth(int panelWidth) {
		colWidth_ = colCnt_ ? ((panelWidth - tagWidth_) / colCnt_) : 0;
	}

	inline void updateRowHeight(int panelHeight) {
		if (labels_.size()) {
			int rest = panelHeight - fontHeight_ * 2;
			rowHeight_ = (labels_.size() > maxDispRowCnt_)
						 ? (rest / maxDispRowCnt_)
						 : (rest / labels_.size());
		}
		else {
			rowHeight_ = 0;
		}
	}
};

#endif // VISUALIZED_INSTRUMENT_MACRO_EDITOR_HPP
