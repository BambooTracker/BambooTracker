/*
 * Copyright (C) 2020 Rerrah
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

#ifndef GO_TO_DIALOG_HPP
#define GO_TO_DIALOG_HPP

#include <memory>
#include <QDialog>
#include "bamboo_tracker.hpp"

namespace Ui {
	class GoToDialog;
}

class GoToDialog : public QDialog
{
	Q_OBJECT

public:
	GoToDialog(std::weak_ptr<BambooTracker> bt, QWidget *parent = nullptr);
	~GoToDialog() override;

	int getOrder() const;
	int getStep() const;
	int getTrack() const;

private slots:
	void on_orderSpinBox_valueChanged(int arg1);

private:
	Ui::GoToDialog *ui;

	std::weak_ptr<BambooTracker> bt_;
	int song_;
};

#endif // GO_TO_DIALOG_HPP
