#ifndef FM_OPERATOR_TABLE_HPP
#define FM_OPERATOR_TABLE_HPP

#include <map>
#include <memory>
#include <QFrame>
#include <QPoint>
#include "gui/labeled_slider.hpp"

namespace Ui {
	class FMOperatorTable;
	enum class FMOperatorParameter;
}

class FMOperatorTable : public QFrame
{
	Q_OBJECT

public:
	explicit FMOperatorTable(QWidget *parent = 0);
	~FMOperatorTable();

	void setOperatorNumber(int n);
	int operatorNumber() const;

	void setValue(Ui::FMOperatorParameter param, int value);

signals:
	void operatorValueChanged(Ui::FMOperatorParameter param, int value);

private:
	Ui::FMOperatorTable *ui;
	int number_;
	std::map<Ui::FMOperatorParameter, LabeledSlider*> sliderMap_;

	void rewriteGraph();
};

namespace Ui {
	enum class FMOperatorParameter
	{
		AR, DR, SR, RR, SL, TL, KS, ML, DT, AM
	};
}

#endif // FM_OPERATOR_TABLE_HPP
