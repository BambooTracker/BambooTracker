#ifndef LABELED_VERTICAL_SLIDER_HPP
#define LABELED_VERTICAL_SLIDER_HPP

#include <QFrame>
#include <QString>

namespace Ui {
	class LabeledVerticalSlider;
}

class LabeledVerticalSlider : public QFrame
{
	Q_OBJECT

public:
	explicit LabeledVerticalSlider(QWidget *parent = 0);
	LabeledVerticalSlider(QString text, QWidget *parent = 0);
	~LabeledVerticalSlider();

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
	Ui::LabeledVerticalSlider *ui;
	void init(QString text);
};

#endif // LABELED_VERTICAL_SLIDER_HPP
