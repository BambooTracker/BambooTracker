#ifndef LABELED_SLIDER_HPP
#define LABELED_SLIDER_HPP

#include <QFrame>
#include <QString>

namespace Ui {
	class LabeledSlider;
}

class LabeledSlider : public QFrame
{
	Q_OBJECT

public:
	explicit LabeledSlider(QWidget *parent = 0);
	LabeledSlider(QString text, QWidget *parent = 0);
	~LabeledSlider();

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
	Ui::LabeledSlider *ui;
	void init(QString text);
};

#endif // LABELED_SLIDER_HPP
