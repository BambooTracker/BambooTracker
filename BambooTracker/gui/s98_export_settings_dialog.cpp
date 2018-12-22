#include "s98_export_settings_dialog.hpp"
#include "ui_s98_export_settings_dialog.h"

S98ExportSettingsDialog::S98ExportSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::S98ExportSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
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
