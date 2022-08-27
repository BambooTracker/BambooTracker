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

#include "order_list_editor.hpp"
#include "ui_order_list_editor.h"

OrderListEditor::OrderListEditor(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::OrderListEditor),
	freezed_(false),
	songLoaded_(false),
	hScrollCellMove_(true)
{
	ui->setupUi(this);

	installEventFilter(this);
	ui->panel->installEventFilter(this);
	ui->verticalScrollBar->installEventFilter(this);

	QObject::connect(ui->panel, &OrderListPanel::hScrollBarChangeRequested,
					 ui->horizontalScrollBar, &QScrollBar::setValue);
	QObject::connect(ui->panel, &OrderListPanel::vScrollBarChangeRequested,
					 this, [&](int num, int max) {
		if (ui->verticalScrollBar->maximum() < num) {
			ui->verticalScrollBar->setMaximum(max);
			ui->verticalScrollBar->setValue(num);
		}
		else {
			ui->verticalScrollBar->setValue(num);
			ui->verticalScrollBar->setMaximum(max);
		}
	});
	QObject::connect(ui->panel, &OrderListPanel::currentTrackChanged,
					 this, [&](int idx) { emit currentTrackChanged(idx); });
	QObject::connect(ui->panel, &OrderListPanel::currentOrderChanged,
					 this, [&](int num) { emit currentOrderChanged(num); });
	QObject::connect(ui->panel, &OrderListPanel::orderEdited,
					 this, [&] { emit orderEdited(); });
	QObject::connect(ui->panel, &OrderListPanel::selected,
					 this, [&](bool isSelected) { emit selected(isSelected); });

	auto focusSlot = [&] { ui->panel->setFocus(); };

	QObject::connect(ui->horizontalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &OrderListPanel::onHScrollBarChanged);
	QObject::connect(ui->horizontalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);

	QObject::connect(ui->verticalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &OrderListPanel::onVScrollBarChanged);
	QObject::connect(ui->verticalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);
}

OrderListEditor::~OrderListEditor()
{
	delete ui;
}

void OrderListEditor::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
	ui->panel->setCore(core);
}

void OrderListEditor::setCommandStack(std::weak_ptr<QUndoStack> stack)
{
	ui->panel->setCommandStack(stack);
}

void OrderListEditor::setConfiguration(std::shared_ptr<Configuration> config)
{
	ui->panel->setConfiguration(config);
}

void OrderListEditor::setColorPallete(std::shared_ptr<ColorPalette> palette)
{
	ui->panel->setColorPallete(palette);
}

void OrderListEditor::addActionToPanel(QAction* action)
{
	ui->panel->addAction(action);
}

void OrderListEditor::changeEditable()
{
	ui->panel->changeEditable();
}

void OrderListEditor::updatePositionByOrderUpdate(bool isFirstUpdate)
{
	ui->panel->updatePositionByOrderUpdate(isFirstUpdate);
}

void OrderListEditor::updatePositionByPositionJump(bool trackChanged)
{
	ui->panel->updatePositionByOrderUpdate(false, true, trackChanged);
}

void OrderListEditor::copySelectedCells()
{
	ui->panel->copySelectedCells();
}

void OrderListEditor::deleteOrder()
{
	ui->panel->deleteOrder();
}

void OrderListEditor::insertOrderBelow()
{
	ui->panel->insertOrderBelow();
}

void OrderListEditor::freeze()
{
	setUpdatesEnabled(false);
	freezed_ = true;
	ui->panel->waitPaintFinish();
}

void OrderListEditor::unfreeze()
{
	freezed_ = false;
	setUpdatesEnabled(true);
}

QFont OrderListEditor::getHeaderFont() const
{
	return ui->panel->getHeaderFont();
}

QFont OrderListEditor::getRowsFont() const
{
	return ui->panel->getRowsFont();
}

QFont OrderListEditor::getDefaultHeaderFont() const
{
	return ui->panel->getDefaultHeaderFont();
}

QFont OrderListEditor::getDefaultRowsFont() const
{
	return ui->panel->getDefaultRowsFont();
}

void OrderListEditor::setFonts(const QFont& headerFont, const QFont& rowsFont)
{
	ui->panel->setFonts(headerFont, rowsFont);
}

void OrderListEditor::setHorizontalScrollMode(bool cellBased, bool refresh)
{
	hScrollCellMove_ = cellBased;
	if (refresh) updateHorizontalSliderMaximum();
}

void OrderListEditor::setVisibleTracks(std::vector<int> tracks)
{
	ui->horizontalScrollBar->setMaximum(20);	// Dummy
	ui->panel->setVisibleTracks(tracks);
	updateMaximumWidth();
	updateHorizontalSliderMaximum();
}

bool OrderListEditor::eventFilter(QObject* watched, QEvent* event)
{
	if (freezed_) return true;	// Ignore every events

	if (watched == this) {
		if (event->type() == QEvent::FocusIn) {
			ui->panel->setFocus();
		}
		return false;
	}

	if (watched == ui->panel) {
		switch (event->type()) {
		case QEvent::FocusIn:
			ui->panel->redrawByFocusChanged();
			emit focusIn();
			return false;
		case QEvent::FocusOut:
			ui->panel->redrawByFocusChanged();
			emit focusOut();
			return false;
		case QEvent::HoverEnter:
		case QEvent::HoverLeave:
			ui->panel->redrawByHoverChanged();
			return false;
		default:
			return false;
		}
	}

	if (watched == ui->verticalScrollBar) {
		switch (event->type()) {
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::DragMove:
		case QEvent::Wheel:
			return (bt_->isPlaySong() && bt_->isFollowPlay());
		default:
			return false;
		}
	}

	return false;
}

void OrderListEditor::resizeEvent(QResizeEvent*)
{
	// For view-based scroll
	updateHorizontalSliderMaximum();
}

/********** Slots **********/
void OrderListEditor::onPatternEditorCurrentTrackChanged(int idx)
{
	ui->panel->onPatternEditorCurrentTrackChanged(idx);
}

void OrderListEditor::onPatternEditorCurrentOrderChanged(int num, int max)
{
	ui->panel->onPatternEditorCurrentOrderChanged(num);
	ui->verticalScrollBar->setMaximum(max);
	ui->verticalScrollBar->setValue(num);
}

void OrderListEditor::onSongLoaded()
{
	ui->horizontalScrollBar->setValue(0);

	ui->panel->onSongLoaded();

	updateMaximumWidth();
	int song = bt_->getCurrentSongNumber();
	songLoaded_ = true;
	updateHorizontalSliderMaximum();
	ui->verticalScrollBar->setValue(0);	// Left here to set appropriate order size before initialization of order position
	ui->verticalScrollBar->setMaximum(static_cast<int>(bt_->getOrderSize(song)) - 1);
}

void OrderListEditor::onShortcutUpdated()
{
	ui->panel->onShortcutUpdated();
}

void OrderListEditor::onPastePressed()
{
	ui->panel->onPastePressed();
}

void OrderListEditor::onSelectPressed(int type)
{
	ui->panel->onSelectPressed(type);
}

void OrderListEditor::onDuplicatePressed()
{
	ui->panel->onDuplicatePressed();
}

void OrderListEditor::onMoveOrderPressed(bool isUp)
{
	ui->panel->onMoveOrderPressed(isUp);
}

void OrderListEditor::onClonePatternsPressed()
{
	ui->panel->onClonePatternsPressed();
}

void OrderListEditor::onCloneOrderPressed()
{
	ui->panel->onCloneOrderPressed();
}

void OrderListEditor::onFollowModeChanged()
{
	ui->panel->onFollowModeChanged();
}

void OrderListEditor::onStoppedPlaySong()
{
	ui->panel->onStoppedPlaySong();
}

void OrderListEditor::onGoOrderRequested(bool toNext)
{
	ui->panel->onGoOrderRequested(toNext);
}

void OrderListEditor::onOrderDataGlobalChanged()
{
	ui->panel->redrawByPatternChanged();	// Redraw only text
}

void OrderListEditor::updateHorizontalSliderMaximum()
{
	if (!bt_ || !songLoaded_) return;
	int max = hScrollCellMove_ ? ui->panel->getFullColumnSize()
							   : ui->panel->getScrollableCountByTrack();
	ui->horizontalScrollBar->setMaximum(max);
}

void OrderListEditor::updateMaximumWidth()
{
	int w;
	if (ui->horizontalScrollBar->sizeHint().width() < ui->panel->maximumWidth()) {
		w = ui->panel->maximumWidth();
	}
	else {
		w = ui->horizontalScrollBar->sizeHint().width();
		ui->panel->setMaximumWidth(w);
	}
	setMaximumWidth(w + ui->verticalScrollBar->width() + 2);
}
