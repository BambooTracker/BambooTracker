/*
 * Copyright (C) 2019 Rerrah
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

#ifndef FM_ENVELOPE_SET_EDIT_DIALOG_HPP
#define FM_ENVELOPE_SET_EDIT_DIALOG_HPP

#include <QDialog>
#include <QComboBox>
#include <vector>
#include "misc.hpp"

namespace Ui {
	class FMEnvelopeSetEditDialog;
}

class FMEnvelopeSetEditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FMEnvelopeSetEditDialog(std::vector<FMEnvelopeTextType> set, QWidget *parent = nullptr);
	~FMEnvelopeSetEditDialog();
	std::vector<FMEnvelopeTextType> getSet();

private slots:
	void on_upToolButton_clicked();
	void on_downToolButton_clicked();
	void on_addPushButton_clicked();
	void on_removePushButton_clicked();
	void on_treeWidget_itemSelectionChanged();

private:
	Ui::FMEnvelopeSetEditDialog *ui;

	void swapset(int aboveRow, int belowRow);
	void insertRow(int row, FMEnvelopeTextType type);
	QComboBox* makeCombobox();
	void alignTreeOn1stItemChanged();
};

#endif // FM_ENVELOPE_SET_EDIT_DIALOG_HPP
