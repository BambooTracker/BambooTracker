#ifndef FM_OPERATOR_TABLE_HPP
#define FM_OPERATOR_TABLE_HPP

#include <map>
#include <memory>
#include <QFrame>
#include <QShowEvent>
#include <QResizeEvent>
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

protected:
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

signals:
	void operatorValueChanged(Ui::FMOperatorParameter param, int value);

private slots:
	void on_ssgegCheckBox_stateChanged(int arg1);

private:
	Ui::FMOperatorTable *ui;
	int number_;
	std::map<Ui::FMOperatorParameter, LabeledSlider*> sliderMap_;

	// Envelope graph
	void resizeGraph();
	void repaintGraph();
};

namespace Ui {
	enum class FMOperatorParameter
	{
		AR, DR, SR, RR, SL, TL, KS, ML, DT, AM, IS_SSGEG, TYPE_SSGEG
	};
}

#endif // FM_OPERATOR_TABLE_HPP
