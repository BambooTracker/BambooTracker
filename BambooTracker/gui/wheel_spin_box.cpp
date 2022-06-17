#include "wheel_spin_box.hpp"

#include <QLineEdit>

WheelSpinBox::WheelSpinBox(QWidget * parent) :
	QSpinBox(parent)
{
	// Prevent mouse scrolling from focusing the QSpinBox.
	QSpinBox::setFocusPolicy(Qt::StrongFocus);
}

void WheelSpinBox::stepBy(int steps)
{
	QSpinBox::stepBy(steps);

	// Prevent mouse scrolling from permanently selecting the QSpinBox.
	if (!QSpinBox::hasFocus()) {
		QSpinBox::lineEdit()->deselect();
	}
}
