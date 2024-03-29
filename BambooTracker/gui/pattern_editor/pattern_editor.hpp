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

#ifndef PATTERN_EDITOR_HPP
#define PATTERN_EDITOR_HPP

#include <QFrame>
#include <QUndoStack>
#include <QEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class PatternEditor;
}

class PatternEditor : public QFrame
{
	Q_OBJECT

public:
	explicit PatternEditor(QWidget *parent = nullptr);
	~PatternEditor() override;
	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);
	void setConfiguration(std::shared_ptr<Configuration> config);
	void setColorPallete(std::shared_ptr<ColorPalette> palette);
	void addActionToPanel(QAction* action);

	void changeEditable();
	void updatePositionByStepUpdate(bool isFirstUpdate);
	void updatepositionByPositionJump(bool trackChanged = false);
	void changeMarker();

	void copySelectedCells();
	void cutSelectedCells();

	void freeze();
	void unfreeze();

	QFont getHeaderFont() const;
	QFont getRowsFont() const;
	QFont getDefaultHeaderFont() const;
	QFont getDefaultRowsFont() const;
	void setFonts(const QFont& headerFont, const QFont& rowsFont);

	void setHorizontalScrollMode(bool cellBased, bool refresh = true);
	void setVisibleTracks(std::vector<int> tracks);
	std::vector<int> getVisibleTracks() const;

signals:
	void currentTrackChanged(int idx);
	void currentOrderChanged(int num, int max);

	void focusIn();
	void focusOut();
	void selected(bool isSelected);
	void instrumentEntered(int num);
	void volumeEntered(int volume);
	void effectEntered(QString text);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;
	void resizeEvent(QResizeEvent*) override;

public slots:
	void onOrderListCurrentTrackChanged(int idx);
	void onOrderListCrrentOrderChanged(int num);

	void onOrderListEdited();
	void onDefaultPatternSizeChanged();
	void onShortcutUpdated();

	void onPatternDataGlobalChanged();

	void setPatternHighlight1Count(int count);
	void setPatternHighlight2Count(int count);
	void setEditableStep(int n);

	void onSongLoaded();

	void onDeletePressed();
	void onPastePressed();
	void onPasteMixPressed();
	void onPasteInsertPressed();
	void onPasteOverwritePressed();
	/// 0: None
	/// 1: All
	/// 2: Row
	/// 3: Column
	/// 4: Pattern
	/// 5: Order
	void onSelectPressed(int type);
	void onTransposePressed(bool isOctave, bool isIncreased);
	void onChangeValuesPressed(bool isCoarse, bool isIncreased);
	void onToggleTrackPressed();
	void onSoloTrackPressed();
	void onExpandPressed();
	void onShrinkPressed();
	void onInterpolatePressed();
	void onReversePressed();
	void onReplaceInstrumentPressed();
	void onFollowModeChanged();
	void onStoppedPlaySong();
	void onDuplicateInstrumentsRemoved();
	void onPlayStepPressed();

private:
	Ui::PatternEditor *ui;
	std::shared_ptr<BambooTracker> bt_;

	bool freezed_;
	bool songLoaded_;

	bool hScrollCellMove_;
	void updateHorizontalSliderMaximum();
};

#endif // PATTERN_EDITOR_HPP
