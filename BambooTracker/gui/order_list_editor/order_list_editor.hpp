/*
 * Copyright (C) 2018-2022 Rerrah
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

#ifndef ORDER_LIST_EDITOR_HPP
#define ORDER_LIST_EDITOR_HPP

#include <QFrame>
#include <QUndoStack>
#include <QEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class OrderListEditor;
}

class OrderListEditor : public QFrame
{
	Q_OBJECT

public:
	explicit OrderListEditor(QWidget *parent = nullptr);
	~OrderListEditor() override;

	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);
	void setConfiguration(std::shared_ptr<Configuration> config);
	void setColorPallete(std::shared_ptr<ColorPalette> palette);
	void addActionToPanel(QAction* action);

	void changeEditable();
	void updatePositionByOrderUpdate(bool isFirstUpdate);
	void updatePositionByPositionJump(bool trackChanged = false);

	void copySelectedCells();
	void deleteOrder();
	void insertOrderBelow();

	void freeze();
	void unfreeze();

	QFont getHeaderFont() const;
	QFont getRowsFont() const;
	QFont getDefaultHeaderFont() const;
	QFont getDefaultRowsFont() const;
	void setFonts(const QFont& headerFont, const QFont& rowsFont);

	void setHorizontalScrollMode(bool cellBased, bool refresh = true);
	void setVisibleTracks(std::vector<int> tracks);

signals:
	void currentTrackChanged(int idx);
	void currentOrderChanged(int num);

	void orderEdited();

	void focusIn();
	void focusOut();
	void selected(bool isSelected);

public slots:
	void onPatternEditorCurrentTrackChanged(int idx);
	void onPatternEditorCurrentOrderChanged(int num, int max);
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

	void onOrderDataGlobalChanged();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void resizeEvent(QResizeEvent*) override;

private:
	Ui::OrderListEditor *ui;
	std::shared_ptr<BambooTracker> bt_;

	bool freezed_;
	bool songLoaded_;

	bool hScrollCellMove_;
	void updateHorizontalSliderMaximum();
	void updateMaximumWidth();
};

#endif // ORDER_LIST_EDITOR_HPP
