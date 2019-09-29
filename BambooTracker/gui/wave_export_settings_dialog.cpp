#include "wave_export_settings_dialog.hpp"
#include "ui_wave_export_settings_dialog.h"

WaveExportSettingsDialog::WaveExportSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::WaveExportSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("55466Hz", 55466);
}

WaveExportSettingsDialog::~WaveExportSettingsDialog()
{
	delete ui;
}

int WaveExportSettingsDialog::getSampleRate() const
{
	return ui->sampleRateComboBox->currentData().toInt();
}

int WaveExportSettingsDialog::getLoopCount() const
{
	return ui->loopSpinBox->value();
}
