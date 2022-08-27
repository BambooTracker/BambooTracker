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

#include "font_info_widget.hpp"
#include "ui_font_info_widget.h"
#include <QFontDialog>

FontInfoWidget::FontInfoWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FontInfoWidget)
{
	ui->setupUi(this);
}

FontInfoWidget::~FontInfoWidget()
{
	delete ui;
}

void FontInfoWidget::setFontInfo(const QFont& font)
{
	font_ = font;

	auto text = QString("%1, %2pt").arg(font.family()).arg(font.pointSize());
	if (font.bold()) text += (", " + tr("Bold"));
	if (font.italic()) text += (", " + tr("Italic"));
	ui->label->setText(text);
}

void FontInfoWidget::on_pushButton_clicked()
{
	bool ok = false;
	QFont font = QFontDialog::getFont(&ok, font_, this);
	if (ok) {
		setFontInfo(font);
	}
}

