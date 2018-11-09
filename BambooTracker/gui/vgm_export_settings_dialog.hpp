#ifndef VGM_EXPORT_SETTINGS_DIALOG_HPP
#define VGM_EXPORT_SETTINGS_DIALOG_HPP

#include <QDialog>

namespace Ui {
	class VgmExportSettingsDialog;
}

class VgmExportSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit VgmExportSettingsDialog(QWidget *parent = nullptr);
	~VgmExportSettingsDialog();

private:
	Ui::VgmExportSettingsDialog *ui;
};

#endif // VGM_EXPORT_SETTINGS_DIALOG_HPP
