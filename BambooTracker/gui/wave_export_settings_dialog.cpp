#include "wave_export_settings_dialog.hpp"
#include "ui_wave_export_settings_dialog.h"

WaveExportSettingsDialog::WaveExportSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::WaveExportSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
}

WaveExportSettingsDialog::~WaveExportSettingsDialog()
{
	delete ui;
}

int WaveExportSettingsDialog::getLoopCount() const
{
	return ui->loopSpinBox->value();
}
