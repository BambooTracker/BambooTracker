#ifndef S98_EXPORT_SETTINGS_DIALOG_HPP
#define S98_EXPORT_SETTINGS_DIALOG_HPP

#include <QDialog>
#include "s98_tag.hpp"

namespace Ui {
	class S98ExportSettingsDialog;
}

class S98ExportSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit S98ExportSettingsDialog(QWidget *parent = nullptr);
	~S98ExportSettingsDialog();

	bool enabledTag() const;
	S98Tag getS98Tag() const;
	int getExportTarget() const;

private slots:
	void updateSupportInformation();

private:
	Ui::S98ExportSettingsDialog *ui;
};

#endif // S98_EXPORT_SETTINGS_DIALOG_HPP
