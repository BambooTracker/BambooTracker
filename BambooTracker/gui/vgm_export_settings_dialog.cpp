#include "vgm_export_settings_dialog.hpp"
#include "ui_vgm_export_settings_dialog.h"
#include <QTextCodec>

VgmExportSettingsDialog::VgmExportSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VgmExportSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
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
	for (auto c : getTrackNameEnglish().toLatin1()) {
		tag.trackNameEn += c;
		tag.trackNameEn += '\0';
	}
	tag.trackNameEn += endNull;

	tag.trackNameJp = "";
	for (auto c : sjis->fromUnicode(getTrackNameJapanese())) {
		tag.trackNameJp += c;
	}
	tag.trackNameJp += endNull;

	tag.gameNameEn = "";
	for (auto c : getGameNameEnglish().toLatin1()) {
		tag.gameNameEn += c;
		tag.gameNameEn += '\0';
	}
	tag.gameNameEn += endNull;

	tag.gameNameJp = "";
	for (auto c : sjis->fromUnicode(getGameNameJapanese())) {
		tag.gameNameJp += c;
	}
	tag.gameNameJp += endNull;

	tag.systemNameEn = "";
	for (auto c : getSystemNameEnglish().toLatin1()) {
		tag.systemNameEn += c;
		tag.systemNameEn += '\0';
	}
	tag.systemNameEn += endNull;

	tag.systemNameJp = "";
	for (auto c : sjis->fromUnicode(getSystemNameJapanese())) {
		tag.systemNameJp += c;
	}
	tag.systemNameJp += endNull;

	tag.authorEn = "";
	for (auto c : getTrackAuthorEnglish().toLatin1()) {
		tag.authorEn += c;
		tag.authorEn += '\0';
	}
	tag.authorEn += endNull;

	tag.authorJp = "";
	for (auto c : sjis->fromUnicode(getTrackAuthorJapanese())) {
		tag.authorJp += c;
	}
	tag.authorJp += endNull;

	tag.releaseDate = "";
	for (auto c : getReleaseDate().toLatin1()) {
		tag.releaseDate += c;
		tag.releaseDate += '\0';
	}
	tag.releaseDate += endNull;

	tag.vgmCreator = "";
	for (auto c : getVgmCreator().toLatin1()) {
		tag.vgmCreator += c;
		tag.vgmCreator += '\0';
	}
	tag.vgmCreator += endNull;

	tag.notes = "";
	for (auto c : getNotes().toLatin1()) {
		tag.notes += c;
		tag.notes += '\0';
	}
	tag.notes += endNull;

	return tag;
}
