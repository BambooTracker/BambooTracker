#ifndef GROOVE_SETTINGS_DIALOG_HPP
#define GROOVE_SETTINGS_DIALOG_HPP

#include <QDialog>
#include <vector>

namespace Ui {
	class GrooveSettingsDialog;
}

class GrooveSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit GrooveSettingsDialog(QWidget *parent = nullptr);
	~GrooveSettingsDialog() override;
	void setGrooveSquences(std::vector<std::vector<int>> seqs);
	std::vector<std::vector<int>> getGrooveSequences();

private slots:
	void on_addButton_clicked();
	void on_removeButton_clicked();
	void on_lineEdit_editingFinished();
	void on_listWidget_currentRowChanged(int currentRow);

private:
	Ui::GrooveSettingsDialog *ui;
	std::vector<std::vector<int>> seqs_;
};

#endif // GROOVE_SETTINGS_DIALOG_HPP
