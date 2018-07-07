#ifndef CUSTOM_SLIDER_H
#define CUSTOM_SLIDER_H

#include <QSlider>
#include <QMouseEvent>

class CustomSlider : public QSlider
{
public:
	CustomSlider(QWidget* parent = Q_NULLPTR);
	CustomSlider(Qt::Orientation orientation, QWidget* parent = Q_NULLPTR);

protected:
	void mousePressEvent(QMouseEvent *ev) override;
};

#endif // CUSTOM_SLIDER_H
