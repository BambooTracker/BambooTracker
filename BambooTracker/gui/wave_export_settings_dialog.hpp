#ifndef WAVE_EXPORT_SETTINGS_DIALOG_HPP
#define WAVE_EXPORT_SETTINGS_DIALOG_HPP

#include <QDialog>

namespace Ui {
	class WaveExportSettingsDialog;
}

class WaveExportSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit WaveExportSettingsDialog(QWidget *parent = nullptr);
	~WaveExportSettingsDialog();

	int getSampleRate() const;
	int getLoopCount() const;

private:
	Ui::WaveExportSettingsDialog *ui;
};

#endif // WAVE_EXPORT_SETTINGS_DIALOG_HPP
