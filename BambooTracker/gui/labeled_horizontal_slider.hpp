#ifndef LABALED_HORIZONTAL_SLIDER_HPP
#define LABALED_HORIZONTAL_SLIDER_HPP

#include <QFrame>
#include <QString>

namespace Ui {
	class LabeledHorizontalSlider;
}

class LabeledHorizontalSlider : public QFrame
{
	Q_OBJECT

public:
	explicit LabeledHorizontalSlider(QWidget *parent = 0);
	LabeledHorizontalSlider(QString text, QWidget *parent = 0);
	~LabeledHorizontalSlider();

	int value() const;
	void setValue(int value);

	int maximum() const;
	void setMaximum(int value);
	int minimum() const;
	void setMinimum(int value);

	QString text() const;
	void setText(QString text);

signals:
	void valueChanged(int value);

private slots:
	void on_slider_valueChanged(int value);

private:
	Ui::LabeledHorizontalSlider *ui;

	void init(QString text);
};

#endif // LABALED_HORIZONTAL_SLIDER_HPP
