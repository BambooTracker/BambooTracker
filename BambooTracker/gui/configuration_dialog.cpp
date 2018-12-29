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
	ui->generalSettingsListWidget->item(7)->setCheckState(toCheckState(config.lock()->getMoveCursorToRight()));

	// Edit settings
	ui->pageJumpLengthSpinBox->setValue(config.lock()->getPageJumpLength());

	// Keys
	ui->keyOffKeySequenceEdit->setKeySequence(QString::fromUtf8(config.lock()->getKeyOffKey().c_str(),
																config.lock()->getKeyOffKey().length()));
	ui->octaveUpKeySequenceEdit->setKeySequence(QString::fromUtf8(config.lock()->getOctaveUpKey().c_str(),
																  config.lock()->getOctaveUpKey().length()));
	ui->octaveDownKeySequenceEdit->setKeySequence(QString::fromUtf8(config.lock()->getOctaveDownKey().c_str(),
																	config.lock()->getOctaveDownKey().length()));
	ui->echoBufferKeySequenceEdit->setKeySequence(QString::fromUtf8(config.lock()->getEchoBufferKey().c_str(),
																	config.lock()->getEchoBufferKey().length()));

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
	config_.lock()->setMoveCursorToRight(fromCheckState(ui->generalSettingsListWidget->item(7)->checkState()));

	// Edit settings
	config_.lock()->setPageJumpLength(ui->pageJumpLengthSpinBox->value());

	// Keys
	config_.lock()->setKeyOffKey(ui->keyOffKeySequenceEdit->keySequence().toString().toStdString());
	config_.lock()->setOctaveUpKey(ui->octaveUpKeySequenceEdit->keySequence().toString().toStdString());
	config_.lock()->setOctaveDownKey(ui->octaveDownKeySequenceEdit->keySequence().toString().toStdString());
	config_.lock()->setEchoBufferKey(ui->echoBufferKeySequenceEdit->keySequence().toString().toStdString());

	// Sound //
	config_.lock()->setSoundDevice(ui->soundDeviceComboBox->currentText().toUtf8().toStdString());
	config_.lock()->setSampleRate(ui->sampleRateComboBox->currentData(Qt::UserRole).toInt());
	config_.lock()->setBufferLength(ui->bufferLengthHorizontalSlider->value());
}

void ConfigurationDialog::on_generalSettingsListWidget_itemSelectionChanged()
{
	QString text;
	switch (ui->generalSettingsListWidget->currentRow()) {
	case 0:	// Warp cursor
		text = "Warp the cursor around the edges of the pattern editor.";
		break;
	case 1:	// Warp across orders
		text = "Move to previous or next order when reaching top or bottom in the pattern editor.";
		break;
	case 2:	// Show row numbers in hex
		text = "Display order numbers and the order count on the status bar in hexadecimal.";
		break;
	case 3:	// Preview previous/next orders
		text = "Preview previous and next orders in the pattern editor.";
		break;
	case 4:	// Backup modeles
		text = "Create a backup copy of the existing file when saving a module.";
		break;
	case 5:	// Don't select on double click
		text = "Don't select the whole track when double-clicking in the pattern editor.";
		break;
	case 6:	// Reverse FM volume order
		text = "Reverse the order of FM volume so that 00 is the quietest in the pattern editor";
		break;
	case 7:	// Move cursor to right
		text = "Move the cursor to right after entering effects in the pattern editor.";
		break;
	default:
		text = "";
		break;
	}
	ui->descPlainTextEdit->setPlainText(QString("Description: ") + text);
}
