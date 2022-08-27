/*
 * Copyright (C) 2022 Rerrah
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

#pragma once

#include <QWidget>
#include <QFont>

namespace Ui {
class FontInfoWidget;
}

/**
 * @brief Widget that has a label displaying a font descripton
 * and a button opening the font setting dialog.
 */
class FontInfoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit FontInfoWidget(QWidget *parent = nullptr);
	~FontInfoWidget();

	/**
	 * @brief Set font information.
	 * @param font font.
	 */
	void setFontInfo(const QFont& font);

	/**
	 * @brief get font.
	 * @return Font.
	 */
	QFont getFontInfo() const { return font_; }

private slots:
	void on_pushButton_clicked();

private:
	Ui::FontInfoWidget *ui;
	QFont font_;
};

