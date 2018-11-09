#include "vgm_export_settings_dialog.hpp"
#include "ui_vgm_export_settings_dialog.h"

VgmExportSettingsDialog::VgmExportSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VgmExportSettingsDialog)
{
	ui->setupUi(this);
}

VgmExportSettingsDialog::~VgmExportSettingsDialog()
{
	delete ui;
}
