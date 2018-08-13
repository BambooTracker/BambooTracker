#include "line_read_only_spin_box.hpp"
#include <QLineEdit>

LineReadOnlySpinBox::LineReadOnlySpinBox(QWidget* parent)
	: QSpinBox(parent)
{
	lineEdit()->setReadOnly(true);
}
