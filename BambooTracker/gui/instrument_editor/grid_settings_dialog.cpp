#include "grid_settings_dialog.hpp"
#include "ui_grid_settings_dialog.h"

GridSettingsDialog::GridSettingsDialog(int interval, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GridSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	ui->intrSpinBox->setValue(interval);
}

GridSettingsDialog::~GridSettingsDialog()
{
	delete ui;
}

int GridSettingsDialog::getInterval() const
{
	return ui->intrSpinBox->value();
}
