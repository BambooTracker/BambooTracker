#include "swap_tracks_dialog.hpp"
#include "ui_swap_tracks_dialog.h"
#include "song.hpp"
#include <QString>
#include "gui/gui_util.hpp"

SwapTracksDialog::SwapTracksDialog(const SongStyle& style, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SwapTracksDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	for (const auto& attrib : style.trackAttribs) {
		QString text = getTrackName(style.type, attrib.source, attrib.channelInSource);
		ui->track1ComboBox->addItem(text, attrib.number);
		ui->track2ComboBox->addItem(text, attrib.number);
	}
}

SwapTracksDialog::~SwapTracksDialog()
{
	delete ui;
}

int SwapTracksDialog::getTrack1() const
{
	return ui->track1ComboBox->currentData().toInt();
}

int SwapTracksDialog::getTrack2() const
{
	return ui->track2ComboBox->currentData().toInt();
}
