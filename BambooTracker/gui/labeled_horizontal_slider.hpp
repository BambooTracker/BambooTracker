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

#ifndef LABALED_HORIZONTAL_SLIDER_HPP
#define LABALED_HORIZONTAL_SLIDER_HPP

#include <QFrame>
#include <QString>
#include <QSlider>
#include <QEvent>

namespace Ui {
	class LabeledHorizontalSlider;
}

class LabeledHorizontalSlider : public QFrame
{
	Q_OBJECT

public:
	explicit LabeledHorizontalSlider(QWidget *parent = nullptr);
	LabeledHorizontalSlider(QString text, QString prefix = "", QString suffix = "", QWidget *parent = nullptr);
	~LabeledHorizontalSlider() override;

	int value() const;
	void setValue(int value);

	int maximum() const;
	void setMaximum(int value);
	int minimum() const;
	void setMinimum(int value);

	void setValueRate(double rate, int precision = 1);
	void setSign(bool enabled);

	void setTickPosition(QSlider::TickPosition position);
	void setTickInterval(int ti);

	QString text() const;
	void setText(QString text);
	QString suffix() const;
	void setSuffix(QString suffix);
	QString prefix() const;
	void setprefix(QString prefix);

signals:
	void valueChanged(int value);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
	void on_slider_valueChanged(int value);

private:
	Ui::LabeledHorizontalSlider *ui;
	QString suffix_, prefix_;
	double rate_;
	int precision_;
	bool isSigned_;

	void init(QString text, QString prefix, QString suffix);
	void updateValueLabel();
};

#endif // LABALED_HORIZONTAL_SLIDER_HPP
