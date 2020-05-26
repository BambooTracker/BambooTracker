#include "vgm_export_settings_dialog.hpp"
#include "ui_vgm_export_settings_dialog.h"
#include "export_handler.hpp"
#include <QTextCodec>

VgmExportSettingsDialog::VgmExportSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VgmExportSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	for (QRadioButton *button : {
		 ui->ym2608RadioButton, ui->ym2612RadioButton, ui->ym2203RadioButton, ui->noneFmRadioButton,
		 ui->internalSsgRadioButton, ui->ay8910PsgRadioButton })
		connect(button, &QAbstractButton::toggled,
				this, &VgmExportSettingsDialog::updateSupportInformation);

	updateSupportInformation();
}

VgmExportSettingsDialog::~VgmExportSettingsDialog()
{
	delete ui;
}

bool VgmExportSettingsDialog::enabledGD3() const
{
	return ui->gd3GroupBox->isChecked();
}

QString VgmExportSettingsDialog::getTrackNameEnglish() const
{
	return ui->titleEnLineEdit->text();
}

QString VgmExportSettingsDialog::getTrackNameJapanese() const
{
	return ui->titleJpLineEdit->text();
}

QString VgmExportSettingsDialog::getGameNameEnglish() const
{
	return ui->nameEnLineEdit->text();
}

QString VgmExportSettingsDialog::getGameNameJapanese() const
{
	return ui->nameJpLineEdit->text();
}

QString VgmExportSettingsDialog::getSystemNameEnglish() const
{
	return ui->systemEnLineEdit->text();
}

QString VgmExportSettingsDialog::getSystemNameJapanese() const
{
	return ui->systemJpLineEdit->text();
}

QString VgmExportSettingsDialog::getTrackAuthorEnglish() const
{
	return ui->authorEnLineEdit->text();
}

QString VgmExportSettingsDialog::getTrackAuthorJapanese() const
{
	return ui->authorJpLineEdit->text();
}

QString VgmExportSettingsDialog::getReleaseDate() const
{
	return ui->releaseDateLineEdit->text();
}

QString VgmExportSettingsDialog::getVgmCreator() const
{
	return ui->creatorLineEdit->text();
}

QString VgmExportSettingsDialog::getNotes() const
{
	return ui->notesPlainTextEdit->toPlainText();
}

GD3Tag VgmExportSettingsDialog::getGD3Tag() const
{
	GD3Tag tag;
	QTextCodec* sjis = QTextCodec::codecForName("Shift-JIS");
	std::string endNull = "";
	endNull += '\0';
	endNull += '\0';

	tag.trackNameEn = "";
	for (const auto& c : getTrackNameEnglish().toLatin1()) {
		tag.trackNameEn += c;
		tag.trackNameEn += '\0';
	}
	tag.trackNameEn += endNull;

	tag.trackNameJp = "";
	for (const auto& c : sjis->fromUnicode(getTrackNameJapanese())) {
		tag.trackNameJp += c;
	}
	tag.trackNameJp += endNull;

	tag.gameNameEn = "";
	for (const auto& c : getGameNameEnglish().toLatin1()) {
		tag.gameNameEn += c;
		tag.gameNameEn += '\0';
	}
	tag.gameNameEn += endNull;

	tag.gameNameJp = "";
	for (const auto& c : sjis->fromUnicode(getGameNameJapanese())) {
		tag.gameNameJp += c;
	}
	tag.gameNameJp += endNull;

	tag.systemNameEn = "";
	for (const auto& c : getSystemNameEnglish().toLatin1()) {
		tag.systemNameEn += c;
		tag.systemNameEn += '\0';
	}
	tag.systemNameEn += endNull;

	tag.systemNameJp = "";
	for (const auto& c : sjis->fromUnicode(getSystemNameJapanese())) {
		tag.systemNameJp += c;
	}
	tag.systemNameJp += endNull;

	tag.authorEn = "";
	for (const auto& c : getTrackAuthorEnglish().toLatin1()) {
		tag.authorEn += c;
		tag.authorEn += '\0';
	}
	tag.authorEn += endNull;

	tag.authorJp = "";
	for (const auto& c : sjis->fromUnicode(getTrackAuthorJapanese())) {
		tag.authorJp += c;
	}
	tag.authorJp += endNull;

	tag.releaseDate = "";
	for (const auto& c : getReleaseDate().toLatin1()) {
		tag.releaseDate += c;
		tag.releaseDate += '\0';
	}
	tag.releaseDate += endNull;

	tag.vgmCreator = "";
	for (const auto& c : getVgmCreator().toLatin1()) {
		tag.vgmCreator += c;
		tag.vgmCreator += '\0';
	}
	tag.vgmCreator += endNull;

	tag.notes = "";
	for (const auto& c : getNotes().toLatin1()) {
		tag.notes += c;
		tag.notes += '\0';
	}
	tag.notes += endNull;

	return tag;
}

int VgmExportSettingsDialog::getExportTarget() const
{
	int target = 0;

	if (ui->ym2608RadioButton->isChecked())
		target |= Export_YM2608;
	else if (ui->ym2612RadioButton->isChecked())
		target |= Export_YM2612;
	else if (ui->ym2203RadioButton->isChecked())
		target |= Export_YM2203;

	if (ui->ay8910PsgRadioButton->isChecked())
		target |= Export_AY8910Psg;
	else if (ui->ym2149PsgRadioButton->isChecked())
		target |= Export_YM2149Psg;

	return target;
}

void VgmExportSettingsDialog::updateSupportInformation()
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
	case Export_NoneFm:
		channels = 0;
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
