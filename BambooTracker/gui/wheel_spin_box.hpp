#pragma once

#include <QSpinBox>

class WheelSpinBox : public QSpinBox
{
public:
	explicit WheelSpinBox(QWidget * parent = nullptr);

	void stepBy(int steps) override;
};
