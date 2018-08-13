#ifndef LINE_READ_ONLY_SPIN_BOX_HPP
#define LINE_READ_ONLY_SPIN_BOX_HPP

#include <QSpinBox>

class LineReadOnlySpinBox : public QSpinBox
{
	Q_OBJECT
public:
	explicit LineReadOnlySpinBox(QWidget* parent = nullptr);
};

#endif // LINE_READ_ONLY_SPIN_BOX_HPP
