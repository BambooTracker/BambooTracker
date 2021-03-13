/*
 * Copyright (C) 2018-2020 Rerrah
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

#ifndef ORDER_LIST_PANEL_HPP
#define ORDER_LIST_PANEL_HPP

#include <QWidget>
#include <QUndoStack>
#include <QPixmap>
#include <QFont>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QRect>
#include <QColor>
#include <QPoint>
#include <QShortcut>
#include <memory>
#include <atomic>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "gui/order_list_editor/order_position.hpp"
#include "song.hpp"
#include "gui/color_palette.hpp"

class OrderListPanel : public QWidget
{
	Q_OBJECT
public:
	explicit OrderListPanel(QWidget *parent = nullptr);
	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);
	void setConfiguration(std::shared_ptr<Configuration> config);
	void setColorPallete(std::shared_ptr<ColorPalette> palette);

	void changeEditable();
	int getFullColumnSize() const;
	void updatePositionByOrderUpdate(bool isFirstUpdate, bool forceJump = false, bool trackChanged = false);
	int getScrollableCountByTrack() const;

	void copySelectedCells();
	void deleteOrder();
	void insertOrderBelow();

	void redrawByPatternChanged(bool ordersLengthChanged = false);
	void redrawByFocusChanged();
	void redrawByHoverChanged();
	void redrawAll();

	void resetEntryCount();

	void waitPaintFinish();

	QString getHeaderFont() const;
	int getHeaderFontSize() const;
	QString getRowsFont() const;
	int getRowsFontSize() const;
	void setFonts(QString headerFont, int headerSize, QString rowsFont, int rowsSize);

	void setVisibleTracks(std::vector<int> tracks);

public slots:
	void onHScrollBarChanged(int num);
	void onVScrollBarChanged(int num);
	void onPatternEditorCurrentTrackChanged(int idx);
	void onPatternEditorCurrentOrderChanged(int num);

	void onOrderEdited();
	void onSongLoaded();
	void onShortcutUpdated();

	void onPastePressed();
	/// 0: None
	/// 1: All
	/// 2: Row
	/// 3: Column
	/// 4: Pattern
	/// 5: Order
	void onSelectPressed(int type);
	void onDuplicatePressed();
	void onMoveOrderPressed(bool isUp);
	void onClonePatternsPressed();
	void onCloneOrderPressed();
	void onFollowModeChanged();
	void onStoppedPlaySong();
	void onGoOrderRequested(bool toNext);

signals:
	void hScrollBarChangeRequested(int num);
	void vScrollBarChangeRequested(int num, int max);
	void currentTrackChanged(int idx);
	void currentOrderChanged(int num);

	void orderEdited();

	void selected(bool isSelected);

protected:
	bool event(QEvent *event) override;
	bool keyPressed(QKeyEvent* event);
	bool keyReleased(QKeyEvent* event);
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	bool mouseHoverd(QHoverEvent* event);
	void wheelEvent(QWheelEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	QPixmap completePixmap_, textPixmap_, backPixmap_, headerPixmap_;
	std::shared_ptr<BambooTracker> bt_;
	std::weak_ptr<QUndoStack> comStack_;
	std::shared_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	QFont rowFont_, headerFont_;
	std::unique_ptr<QFontMetrics> hdFontMets_;
	int rowFontWidth_, rowFontHeight_, rowFontAscent_, rowFontLeading_;
	int headerFontAscent_;

	int widthSpace_;
	int rowNumWidthCnt_, rowNumWidth_, rowNumBase_;
	int trackWidth_;
	int columnsWidthFromLeftToEnd_;
	int headerHeight_;
	int curRowBaselineY_;
	int curRowY_;

	std::vector<int> visTracks_;

	int leftTrackVisIdx_;
	SongStyle songStyle_;

	int curSongNum_;
	OrderPosition curPos_, hovPos_;
	OrderPosition mousePressPos_, mouseReleasePos_;
	OrderPosition selLeftAbovePos_, selRightBelowPos_;
	OrderPosition shiftPressedPos_;

	bool isIgnoreToSlider_, isIgnoreToPattern_;

	int entryCnt_;

	int selectAllState_;

	int viewedRowCnt_;
	int viewedRegionHeight_;
	int viewedRowsHeight_, viewedRowOffset_, viewedCenterY_, viewedCenterBaseY_;
	OrderPosition viewedFirstPos_, viewedCenterPos_, viewedLastPos_;

	bool backChanged_, textChanged_, headerChanged_, followModeChanged_;
	bool hasFocussedBefore_;
	int orderDownCount_;

	std::atomic_bool repaintable_;	// Recurrensive repaint guard
	std::atomic_int repaintingCnt_;

	int playingRow_;

	QShortcut insSc1_, insSc2_, menuSc_;

	void updateSizes();
	void initDisplay();

	void drawList(const QRect& rect);
	void drawRows(int maxWidth);
	void quickDrawRows(int maxWidth);
	void drawHeaders(int maxWidth);
	void drawBorders(int maxWidth);
	void drawShadow();

	// NOTE: Calculated by visible tracks
	inline int calculateColumnsWidthWithRowNum(int beginIdx, int endIdx) const
	{
		return rowNumWidth_ + trackWidth_ * (endIdx - beginIdx + 1);
	}

	inline void updateTracksWidthFromLeftToEnd()
	{
		columnsWidthFromLeftToEnd_ = calculateColumnsWidthWithRowNum(
										 leftTrackVisIdx_, static_cast<int>(visTracks_.size()) - 1);
	}

	void moveCursorToRight(int n);
	void moveViewToRight(int n);
	void moveCursorToDown(int n);

	bool enterOrder(int key);
	void setCellOrderNum(int n);

	void pasteCopiedCells(const OrderPosition& startPos);
	void clonePatterns(const OrderPosition& singlePos);

	void setSelectedRectangle(const OrderPosition& start, const OrderPosition& end);
	bool isSelectedCell(int trackIdx, int row);

	void showContextMenu(const OrderPosition& pos, const QPoint& point);
};

#endif // ORDER_LIST_PANEL_HPP
