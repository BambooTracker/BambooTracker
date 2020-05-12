#ifndef GRID_SETTINGS_DIALOG_HPP
#define GRID_SETTINGS_DIALOG_HPP

#include <QDialog>

namespace Ui {
	class GridSettingsDialog;
}

class GridSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit GridSettingsDialog(int interval = 1, QWidget *parent = nullptr);
	~GridSettingsDialog() override;

	int getInterval() const;

private:
	Ui::GridSettingsDialog *ui;
};

#endif // GRID_SETTINGS_DIALOG_HPP
