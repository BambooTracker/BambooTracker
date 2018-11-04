#ifndef WAVE_EXPORT_SETTING_DIALOG_HPP
#define WAVE_EXPORT_SETTING_DIALOG_HPP

#include <QDialog>

namespace Ui {
	class WaveExportSettingDialog;
}

class WaveExportSettingDialog : public QDialog
{
	Q_OBJECT

public:
	explicit WaveExportSettingDialog(QWidget *parent = nullptr);
	~WaveExportSettingDialog();

	int getLoopCount() const;

private:
	Ui::WaveExportSettingDialog *ui;
};

#endif // WAVE_EXPORT_SETTING_DIALOG_HPP
