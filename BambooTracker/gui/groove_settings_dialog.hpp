/*
 * Copyright (C) 2018-2019 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
