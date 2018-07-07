#include "custom_slider.hpp"

CustomSlider::CustomSlider(QWidget *parent) : QSlider(parent) {}

CustomSlider::CustomSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent) {}

void CustomSlider::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton)
	{
		if (orientation() == Qt::Vertical) {
			setValue(minimum() + (maximum() - minimum()) * (height() - ev->y()) / height());
		}
		else /* horizontal */ {
			setValue(maximum() - (maximum() - minimum()) * (width() - ev->x()) / width());
		}
	}
	QSlider::mousePressEvent(ev);
}
