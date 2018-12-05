#include "configuration_dialog.hpp"
#include "ui_configuration_dialog.h"
#include <QPushButton>
#include <QAudio>
#include <QAudioDeviceInfo>
#include "slider_style.hpp"

ConfigurationDialog::ConfigurationDialog(std::weak_ptr<Configuration> config, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ConfigurationDialog),
	  config_(config)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	QObject::connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
					 this, [&] {
		on_ConfigurationDialog_accepted();
		emit applyPressed();
	});

	// General //
	// General settings
	ui->generalSettingsListWidget->item(0)->setCheckState(toCheckState(config.lock()->getWarpCursor()));
	ui->generalSettingsListWidget->item(1)->setCheckState(toCheckState(config.lock()->getWarpAcrossOrders()));
	ui->generalSettingsListWidget->item(2)->setCheckState(toCheckState(config.lock()->getShowRowNumberInHex()));
	ui->generalSettingsListWidget->item(3)->setCheckState(toCheckState(config.lock()->getShowPreviousNextOrders()));
	ui->generalSettingsListWidget->item(4)->setCheckState(toCheckState(config.lock()->getBackupModules()));
	ui->generalSettingsListWidget->item(5)->setCheckState(toCheckState(config.lock()->getDontSelectOnDoubleClick()));
	ui->generalSettingsListWidget->item(6)->setCheckState(toCheckState(config.lock()->getReverseFMVolumeOrder()));

	// Edit settings
	ui->pageJumpLengthSpinBox->setValue(config.lock()->getPageJumpLength());

	// Sound //
	int devRow = -1;
	int defDevRow = 0;
	for (auto& info : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
		ui->soundDeviceComboBox->addItem(info.deviceName());
		if (info.deviceName() == QString::fromUtf8(config.lock()->getSoundDevice().c_str(),
												   config.lock()->getSoundDevice().length()))
			devRow = ui->soundDeviceComboBox->count() - 1;
		if (info.deviceName() == QAudioDeviceInfo::defaultOutputDevice().deviceName()) {
			defDevRow = ui->soundDeviceComboBox->count() - 1;
		}
	}
	ui->soundDeviceComboBox->setCurrentIndex((devRow == -1) ? defDevRow : devRow);

	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("55466Hz", 55466);
	switch (config.lock()->getSampleRate()) {
	case 44100:	ui->sampleRateComboBox->setCurrentIndex(0);	break;
	case 48000:	ui->sampleRateComboBox->setCurrentIndex(1);	break;
	case 55466:	ui->sampleRateComboBox->setCurrentIndex(2);	break;
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
	config_.lock()->setWarpCursor(fromCheckState(ui->generalSettingsListWidget->item(0)->checkState()));
	config_.lock()->setWarpAcrossOrders(fromCheckState(ui->generalSettingsListWidget->item(1)->checkState()));
	config_.lock()->setShowRowNumberInHex(fromCheckState(ui->generalSettingsListWidget->item(2)->checkState()));
	config_.lock()->setShowPreviousNextOrders(fromCheckState(ui->generalSettingsListWidget->item(3)->checkState()));
	config_.lock()->setBackupModules(fromCheckState(ui->generalSettingsListWidget->item(4)->checkState()));
	config_.lock()->setDontSelectOnDoubleClick(fromCheckState(ui->generalSettingsListWidget->item(5)->checkState()));
	config_.lock()->setReverseFMVolumeOrder(fromCheckState(ui->generalSettingsListWidget->item(6)->checkState()));

	// Edit settings
	config_.lock()->setPageJumpLength(ui->pageJumpLengthSpinBox->value());

	// Sound //
	config_.lock()->setSoundDevice(ui->soundDeviceComboBox->currentText().toUtf8().toStdString());
	config_.lock()->setSampleRate(ui->sampleRateComboBox->currentData(Qt::UserRole).toInt());
	config_.lock()->setBufferLength(ui->bufferLengthHorizontalSlider->value());
}
