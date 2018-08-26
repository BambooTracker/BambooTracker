#ifndef MODULE_SETTINGS_DIALOG_HPP
#define MODULE_SETTINGS_DIALOG_HPP

#include <QDialog>
#include <QString>
#include <QTreeWidgetItem>
#include <memory>
#include "bamboo_tracker.hpp"
#include "misc.hpp"

namespace Ui {
	class ModuleSettingsDialog;
}

class ModuleSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	ModuleSettingsDialog(std::weak_ptr<BambooTracker> core, QWidget *parent = nullptr);
	~ModuleSettingsDialog() override;

private slots:
	void on_upToolButton_clicked();
	void on_downToolButton_clicked();
	void on_removePushButton_clicked();
	void on_insertPushButton_clicked();
	void on_songTreeWidget_itemSelectionChanged();
	void on_editTitleLineEdit_textEdited(const QString &arg1);

	void on_buttonBox_accepted();

private:
	Ui::ModuleSettingsDialog *ui;
	std::weak_ptr<BambooTracker> bt_;

	void insertSong(int row, QString title, SongType type, int prevNum = -1);
	void checkButtonsEnabled();
	void swapset(int aboveRow, int belowRow);
};

#endif // MODULE_SETTINGS_DIALOG_HPP
