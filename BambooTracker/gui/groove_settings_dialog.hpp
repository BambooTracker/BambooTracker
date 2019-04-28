#ifndef GROOVE_SETTINGS_DIALOG_HPP
#define GROOVE_SETTINGS_DIALOG_HPP

#include <QDialog>
#include <QKeyEvent>
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

protected:
	void keyPressEvent(QKeyEvent* event) override;

private slots:
	void on_addButton_clicked();
	void on_removeButton_clicked();
	void on_lineEdit_editingFinished();
	void on_grooveListWidget_currentRowChanged(int currentRow);
	void on_upToolButton_clicked();
	void on_downToolButton_clicked();
	void on_expandPushButton_clicked();
	void on_shrinkPushButton_clicked();
	void on_genPushButton_clicked();
	void on_padPushButton_clicked();
	void on_copyPushButton_clicked();

private:
	Ui::GrooveSettingsDialog *ui;
	std::vector<std::vector<int>> seqs_;

	void changeSequence(int seqNum);
	QString updateSequence(size_t seqNum);
	void swapSequenceItem(size_t seqNum, int index1, int index2);
};

#endif // GROOVE_SETTINGS_DIALOG_HPP
