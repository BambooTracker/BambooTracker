#include "order_list_editor.hpp"
#include "ui_order_list_editor.h"
#include <QDebug>
OrderListEditor::OrderListEditor(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::OrderListEditor)
{
	ui->setupUi(this);

	QObject::connect(ui->panel, &OrderListPanel::currentTrackChanged,
					 ui->horizontalScrollBar, &QScrollBar::setValue);

	auto focusSlot = [&]() { ui->panel->setFocus(); };

	QObject::connect(ui->horizontalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &OrderListPanel::setCurrentTrack);
	QObject::connect(ui->horizontalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);
	ui->horizontalScrollBar->installEventFilter(this);

	QObject::connect(ui->verticalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);
	ui->verticalScrollBar->installEventFilter(this);
}

OrderListEditor::~OrderListEditor()
{
	delete ui;
}

void OrderListEditor::setCore(std::shared_ptr<BambooTracker> core)
{
	ui->panel->setCore(core);

	ui->horizontalScrollBar->setMaximum(core->getModuleStyle().trackAttribs.size() - 1);
	ui->verticalScrollBar->setMaximum(core->getOrderList(0, 0).size() - 1);
}

void OrderListEditor::changeEditable()
{
	ui->panel->changeEditable();
}
