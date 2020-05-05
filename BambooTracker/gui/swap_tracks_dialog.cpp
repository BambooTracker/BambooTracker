#include "swap_tracks_dialog.hpp"
#include "ui_swap_tracks_dialog.h"

SwapTracksDialog::SwapTracksDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SwapTracksDialog)
{
	ui->setupUi(this);
}

SwapTracksDialog::~SwapTracksDialog()
{
	delete ui;
}
