#include "wave_export_setting_dialog.hpp"
#include "ui_wave_export_setting_dialog.h"

WaveExportSettingDialog::WaveExportSettingDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::WaveExportSettingDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
}

WaveExportSettingDialog::~WaveExportSettingDialog()
{
	delete ui;
}

int WaveExportSettingDialog::getLoopCount() const
{
	return ui->loopSpinBox->value();
}
