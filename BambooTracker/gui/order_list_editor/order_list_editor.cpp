#include "order_list_editor.hpp"
#include "ui_order_list_editor.h"

OrderListEditor::OrderListEditor(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::OrderListEditor)
{
	ui->setupUi(this);

	installEventFilter(this);
	ui->panel->installEventFilter(this);

	QObject::connect(ui->panel, &OrderListPanel::currentTrackChangedForSlider,
					 ui->horizontalScrollBar, &QScrollBar::setValue);
	QObject::connect(ui->panel, &OrderListPanel::currentOrderChangedForSlider,
					 this, [&](int num, int max) {
		ui->verticalScrollBar->setMaximum(max);
		ui->verticalScrollBar->setValue(num);
	});
	QObject::connect(ui->panel, &OrderListPanel::currentTrackChanged,
					 this, [&](int num) { emit currentTrackChanged(num); });
	QObject::connect(ui->panel, &OrderListPanel::currentOrderChanged,
					 this, [&](int num) { emit currentOrderChanged(num); });
	QObject::connect(ui->panel, &OrderListPanel::orderEdited,
					 this, [&] { emit orderEdited(); });
	QObject::connect(ui->panel, &OrderListPanel::selected,
					 this, [&](bool isSelected) { emit selected(isSelected); });
	QObject::connect(ui->panel, &OrderListPanel::returnPressed,
					 this, [&] { emit returnPressed(); });

	auto focusSlot = [&] { ui->panel->setFocus(); };

	QObject::connect(ui->horizontalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &OrderListPanel::setCurrentTrackForSlider);
	QObject::connect(ui->horizontalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);

	QObject::connect(ui->verticalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &OrderListPanel::setCurrentOrderForSlider);
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

void OrderListEditor::setConfiguration(std::weak_ptr<Configuration> config)
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

void OrderListEditor::updatePositionByOrderUpdate()
{
	ui->panel->updatePositionByOrderUpdate();
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

bool OrderListEditor::eventFilter(QObject *watched, QEvent *event)
{
	Q_UNUSED(watched)

	if (watched == this) {
		if (event->type() == QEvent::FocusIn) {
			ui->panel->setFocus();
		}
		return false;
	}

	if (watched == ui->panel) {
		switch (event->type()) {
		case QEvent::FocusIn:	emit focusIn();		return false;
		case QEvent::FocusOut:	emit focusOut();	return false;
		default:	return false;
		}
	}

	return false;
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
	ui->horizontalScrollBar->setMaximum(static_cast<int>(bt_->getSongStyle(song).trackAttribs.size()) - 1);
	ui->verticalScrollBar->setValue(0);	// Left here to set appropriate order size before initialization of order position
	ui->verticalScrollBar->setMaximum(static_cast<int>(bt_->getOrderSize(song)) - 1);
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
