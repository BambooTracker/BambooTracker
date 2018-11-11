#include "configuration_dialog.hpp"
#include "ui_configuration_dialog.h"
#include "slider_style.hpp"

ConfigurationDialog::ConfigurationDialog(std::weak_ptr<Configuration> config, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ConfigurationDialog),
	  config_(config)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	// General //
	// General settings

	// Edit settings
	ui->pageJumpLengthSpinBox->setValue(config.lock()->getPageJumpLength());
	// Sound //
	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("110933Hz", 110933);
	switch (config.lock()->getSampleRate()) {
	case 44100:		ui->sampleRateComboBox->setCurrentIndex(0);	break;
	case 48000:		ui->sampleRateComboBox->setCurrentIndex(1);	break;
	case 110933:	ui->sampleRateComboBox->setCurrentIndex(2);	break;
	}
	ui->bufferLengthHorizontalSlider->setStyle(new SliderStyle());
	QObject::connect(ui->bufferLengthHorizontalSlider, &QSlider::valueChanged,
					 this, [&](int value) {
		ui->bufferLengthLabel->setText(QString::number(value) + "ms");
	});
	ui->bufferLengthHorizontalSlider->setValue(config.lock()->getBufferLength());
}

ConfigurationDialog::~ConfigurationDialog()
{
	delete ui;
}

void ConfigurationDialog::on_ConfigurationDialog_accepted()
{
	// General //
	// General settings

	// Edit settings
	config_.lock()->setPageJumpLength(ui->pageJumpLengthSpinBox->value());
	// Sound //
	config_.lock()->setSampleRate(ui->sampleRateComboBox->currentData(Qt::UserRole).toInt());
	config_.lock()->setBufferLength(ui->bufferLengthHorizontalSlider->value());
}
