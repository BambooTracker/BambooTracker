#include "go_to_dialog.hpp"
#include "ui_go_to_dialog.h"
#include <QString>
#include "gui/gui_util.hpp"

GoToDialog::GoToDialog(std::weak_ptr<BambooTracker> bt, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GoToDialog),
	bt_(bt),
	song_(bt.lock()->getCurrentSongNumber())
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);


	ui->orderSpinBox->setMaximum(bt.lock()->getOrderSize(song_) - 1);
	ui->orderSpinBox->setValue(bt.lock()->getCurrentOrderNumber());
	ui->stepSpinBox->setMaximum(bt.lock()->getPatternSizeFromOrderNumber(song_, ui->orderSpinBox->value()));
	ui->stepSpinBox->setValue(bt.lock()->getCurrentStepNumber());
	auto style = bt.lock()->getSongStyle(bt.lock()->getCurrentSongNumber());
	for (auto& attrib : style.trackAttribs) {
		ui->trackComboBox->addItem(
					getTrackName(style.type, attrib.source, attrib.channelInSource), attrib.number);
		if (bt.lock()->getCurrentTrackAttribute().number == attrib.number)
			ui->trackComboBox->setCurrentIndex(ui->trackComboBox->count() - 1);
	}
}

GoToDialog::~GoToDialog()
{
	delete ui;
}

int GoToDialog::getOrder() const
{
	return ui->orderSpinBox->value();
}

int GoToDialog::getStep() const
{
	return ui->stepSpinBox->value();
}

int GoToDialog::getTrack() const
{
	return ui->trackComboBox->currentData().toInt();
}

void GoToDialog::on_orderSpinBox_valueChanged(int arg1)
{
	ui->stepSpinBox->setMaximum(bt_.lock()->getPatternSizeFromOrderNumber(song_, arg1));
}
