#ifndef VGM_EXPORT_SETTINGS_DIALOG_HPP
#define VGM_EXPORT_SETTINGS_DIALOG_HPP

#include <QDialog>
#include <QString>
#include "gd3_tag.hpp"

namespace Ui {
	class VgmExportSettingsDialog;
}

class VgmExportSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit VgmExportSettingsDialog(QWidget *parent = nullptr);
	~VgmExportSettingsDialog();

	bool enabledGD3() const;
	QString getTrackNameEnglish() const;
	QString getTrackNameJapanese() const;
	QString getGameNameEnglish() const;
	QString getGameNameJapanese() const;
	QString getSystemNameEnglish() const;
	QString getSystemNameJapanese() const;
	QString getTrackAuthorEnglish() const;
	QString getTrackAuthorJapanese() const;
	QString getReleaseDate() const;
	QString getVgmCreator() const;
	QString getNotes() const;
	GD3Tag getGD3Tag() const;
	int getExportTarget() const;

private slots:
	void updateSupportInformation();

private:
	Ui::VgmExportSettingsDialog *ui;
};

#endif // VGM_EXPORT_SETTINGS_DIALOG_HPP
