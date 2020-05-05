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
					 this, [&](int num) { emit currentTrackChanged(num); });
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
	freezed_ = true;
	ui->panel->freeze();
}

void OrderListEditor::unfreeze()
{
	freezed_ = false;
	ui->panel->unfreeze();
}

QString OrderListEditor::getHeaderFont() const
{
	return ui->panel->getHeaderFont();
}

int OrderListEditor::getHeaderFontSize() const
{
	return ui->panel->getHeaderFontSize();
}

QString OrderListEditor::getRowsFont() const
{
	return ui->panel->getRowsFont();
}

int OrderListEditor::getRowsFontSize() const
{
	return ui->panel->getRowsFontSize();
}

void OrderListEditor::setFonts(QString headerFont, int headerSize, QString rowsFont, int rowsSize)
{
	ui->panel->setFonts(headerFont, headerSize, rowsFont, rowsSize);
}

void OrderListEditor::setHorizontalScrollMode(bool cellBased, bool refresh)
{
	hScrollCellMove_ = cellBased;
	if (refresh) updateHorizontalSliderMaximum();
}

bool OrderListEditor::eventFilter(QObject *watched, QEvent *event)
{
	Q_UNUSED(watched)

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

void OrderListEditor::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)

	// For view-based scroll
	updateHorizontalSliderMaximum();
}

/********** Slots **********/
void OrderListEditor::setCurrentTrack(int num)
{
	ui->panel->setCurrentTrack(num);
}

void OrderListEditor::setCurrentOrder(int num, int max)
{
	ui->panel->setCurrentOrder(num);
	ui->verticalScrollBar->setMaximum(max);
	ui->verticalScrollBar->setValue(num);
}

void OrderListEditor::onSongLoaded()
{
	ui->horizontalScrollBar->setValue(0);

	ui->panel->onSongLoaded();

	setMaximumWidth(ui->panel->maximumWidth() + ui->verticalScrollBar->width() + 2);
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
	int song = bt_->getCurrentSongNumber();
	int max = hScrollCellMove_ ? static_cast<int>(bt_->getSongStyle(song).trackAttribs.size()) - 1
							   : ui->panel->getScrollableCountByTrack();
	ui->horizontalScrollBar->setMaximum(max);
}
