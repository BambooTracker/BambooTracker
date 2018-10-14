#include "configuration_dialog.hpp"
#include "ui_configuration_dialog.h"
#include "slider_style.hpp"

ConfigurationDialog::ConfigurationDialog(std::weak_ptr<BambooTracker> core, std::weak_ptr<AudioStream> stream, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ConfigurationDialog),
	core_(core),
	stream_(stream)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	// Sound
	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("110933Hz", 110933);
	switch (core_.lock()->getStreamRate()) {
	case 44100:		ui->sampleRateComboBox->setCurrentIndex(0);	break;
	case 48000:		ui->sampleRateComboBox->setCurrentIndex(1);	break;
	case 110933:	ui->sampleRateComboBox->setCurrentIndex(2);	break;
	}
	ui->bufferLengthHorizontalSlider->setStyle(new SliderStyle());
	QObject::connect(ui->bufferLengthHorizontalSlider, &QSlider::valueChanged,
					 this, [&](int value) {
		ui->bufferLengthLabel->setText(QString::number(value) + "ms");
	});
	ui->bufferLengthHorizontalSlider->setValue(core_.lock()->getStreamDuration());
}

ConfigurationDialog::~ConfigurationDialog()
{
	delete ui;
}

void ConfigurationDialog::on_ConfigurationDialog_accepted()
{
	// Sound
	int rate = ui->sampleRateComboBox->currentData(Qt::UserRole).toInt();
	stream_.lock()->setRate(rate);
	core_.lock()->setStreamRate(rate);
	int bufLen = ui->bufferLengthHorizontalSlider->value();
	stream_.lock()->setDuration(bufLen);
	core_.lock()->setStreamDuration(bufLen);
}
