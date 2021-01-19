/*
 * Copyright (C) 2018-2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "vgm_export_settings_dialog.hpp"
#include "ui_vgm_export_settings_dialog.h"
#include <QTextCodec>
#include "io/export_io.hpp"

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

io::GD3Tag VgmExportSettingsDialog::getGD3Tag() const
{
	io::GD3Tag tag;
	QTextCodec* sjis = QTextCodec::codecForName("Shift-JIS");
	std::string endNull = "";
	endNull += '\0';
	endNull += '\0';

	const QByteArray trackNameEn = getTrackNameEnglish().toLatin1();
	tag.trackNameEn = "";
	for (const auto& c : trackNameEn) {
		tag.trackNameEn += c;
		tag.trackNameEn += '\0';
	}
	tag.trackNameEn += endNull;

	const QByteArray trackNameJp = sjis->fromUnicode(getTrackNameJapanese());
	tag.trackNameJp = "";
	for (const auto& c : trackNameJp) {
		tag.trackNameJp += c;
	}
	tag.trackNameJp += endNull;

	const QByteArray gameNameEn = getGameNameEnglish().toLatin1();
	tag.gameNameEn = "";
	for (const auto& c : gameNameEn) {
		tag.gameNameEn += c;
		tag.gameNameEn += '\0';
	}
	tag.gameNameEn += endNull;

	const QByteArray gameNameJp = sjis->fromUnicode(getGameNameJapanese());
	tag.gameNameJp = "";
	for (const auto& c : gameNameJp) {
		tag.gameNameJp += c;
	}
	tag.gameNameJp += endNull;

	const QByteArray systemNameEn = getSystemNameEnglish().toLatin1();
	tag.systemNameEn = "";
	for (const auto& c : systemNameEn) {
		tag.systemNameEn += c;
		tag.systemNameEn += '\0';
	}
	tag.systemNameEn += endNull;

	const QByteArray systemNameJp = sjis->fromUnicode(getSystemNameJapanese());
	tag.systemNameJp = "";
	for (const auto& c : systemNameJp) {
		tag.systemNameJp += c;
	}
	tag.systemNameJp += endNull;

	const QByteArray authorEn = getTrackAuthorEnglish().toLatin1();
	tag.authorEn = "";
	for (const auto& c : authorEn) {
		tag.authorEn += c;
		tag.authorEn += '\0';
	}
	tag.authorEn += endNull;

	const QByteArray authorJp = sjis->fromUnicode(getTrackAuthorJapanese());
	tag.authorJp = "";
	for (const auto& c : authorJp) {
		tag.authorJp += c;
	}
	tag.authorJp += endNull;

	const QByteArray releaseDate = getReleaseDate().toLatin1();
	tag.releaseDate = "";
	for (const auto& c : releaseDate) {
		tag.releaseDate += c;
		tag.releaseDate += '\0';
	}
	tag.releaseDate += endNull;

	const QByteArray vgmCreator = getVgmCreator().toLatin1();
	tag.vgmCreator = "";
	for (const auto& c : vgmCreator) {
		tag.vgmCreator += c;
		tag.vgmCreator += '\0';
	}
	tag.vgmCreator += endNull;

	const QByteArray notes = getNotes().toLatin1();
	tag.notes = "";
	for (const auto& c : notes) {
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
		target |= io::Export_YM2608;
	else if (ui->ym2612RadioButton->isChecked())
		target |= io::Export_YM2612;
	else if (ui->ym2203RadioButton->isChecked())
		target |= io::Export_YM2203;

	if (ui->ay8910PsgRadioButton->isChecked())
		target |= io::Export_AY8910Psg;
	else if (ui->ym2149PsgRadioButton->isChecked())
		target |= io::Export_YM2149Psg;

	return target;
}

void VgmExportSettingsDialog::updateSupportInformation()
{
	int target = getExportTarget();
	int channels;

	int fm = target & io::Export_FmMask;
	int ssg = target & io::Export_SsgMask;

	switch (fm) {
	default:
		channels = 6;
		break;
	case io::Export_YM2203:
		channels = 3;
		break;
	case io::Export_NoneFm:
		channels = 0;
		break;
	}

	bool haveSsg = fm == io::Export_YM2608 || fm == io::Export_YM2203 || ssg != io::Export_InternalSsg;
	bool haveRhythm = fm == io::Export_YM2608;
	bool haveAdpcm = fm == io::Export_YM2608;

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
