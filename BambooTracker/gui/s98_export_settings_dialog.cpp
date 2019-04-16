#include "s98_export_settings_dialog.hpp"
#include "ui_s98_export_settings_dialog.h"
#include "export_handler.hpp"

S98ExportSettingsDialog::S98ExportSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::S98ExportSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	for (QRadioButton *button : {
			ui->ym2608RadioButton, ui->ym2612RadioButton, ui->ym2203RadioButton,
			ui->internalSsgRadioButton, ui->ay8910PsgRadioButton })
		connect(button, &QAbstractButton::toggled,
				this, &S98ExportSettingsDialog::updateSupportInformation);

	updateSupportInformation();
}

S98ExportSettingsDialog::~S98ExportSettingsDialog()
{
	delete ui;
}

bool S98ExportSettingsDialog::enabledTag() const
{
	return ui->tagGroupBox->isChecked();
}

S98Tag S98ExportSettingsDialog::getS98Tag() const
{
	S98Tag tag;
	tag.title = ui->titleLineEdit->text().toUtf8().toStdString();
	tag.artist = ui->artistLineEdit->text().toUtf8().toStdString();
	tag.game = ui->gameLineEdit->text().toUtf8().toStdString();
	tag.year = ui->yearLineEdit->text().toUtf8().toStdString();
	tag.genre = ui->genreLineEdit->text().toUtf8().toStdString();
	tag.comment = ui->commentLineEdit->text().toUtf8().toStdString();
	tag.copyright = ui->copyrightLineEdit->text().toUtf8().toStdString();
	tag.s98by = ui->s98byLineEdit->text().toUtf8().toStdString();
	tag.system = ui->systemLineEdit->text().toUtf8().toStdString();
	return tag;
}

int S98ExportSettingsDialog::getExportTarget() const
{
	int target = 0;

	if (ui->ym2612RadioButton->isChecked())
		target |= Export_YM2612;
	else if (ui->ym2203RadioButton->isChecked())
		target |= Export_YM2203;

	if (ui->ay8910PsgRadioButton->isChecked())
		target |= Export_AY8910Psg;
	else if (ui->ym2149PsgRadioButton->isChecked())
		target |= Export_YM2149Psg;

	return target;
}

void S98ExportSettingsDialog::updateSupportInformation()
{
	int target = getExportTarget();
	int channels;

	int fm = target & Export_FmMask;
	int ssg = target & Export_SsgMask;

	switch (fm) {
	default:
		channels = 6;
		break;
	case Export_YM2203:
		channels = 3;
		break;
	}

	bool haveSsg = fm == Export_YM2608 || fm == Export_YM2203 || ssg != Export_InternalSsg;
	bool haveRhythm = fm == Export_YM2608;
	bool haveAdpcm = fm == Export_YM2608;

	ui->supportFmChannelsLabel->setText(QString::number(channels));
	ui->supportSsgLabel->setText(haveSsg ? tr("Yes") : tr("No"));
	ui->supportRhythmLabel->setText(haveRhythm ? tr("Yes") : tr("No"));
	ui->supportAdpcmLabel->setText(haveAdpcm ? tr("Yes") : tr("No"));

	QPalette normalPalette = palette();
	QPalette warnPalette = normalPalette;
	warnPalette.setColor(QPalette::WindowText, QColor(0xef2929));

	ui->supportFmChannelsLabel->setPalette((channels == 6) ? normalPalette : warnPalette);
	ui->supportSsgLabel->setPalette(haveSsg ? normalPalette : warnPalette);
	ui->supportRhythmLabel->setPalette(haveRhythm ? normalPalette : warnPalette);
	ui->supportAdpcmLabel->setPalette(haveAdpcm ? normalPalette : warnPalette);
}
