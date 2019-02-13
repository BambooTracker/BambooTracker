#ifndef FM_OPERATOR_TABLE_HPP
#define FM_OPERATOR_TABLE_HPP

#include <map>
#include <memory>
#include <vector>
#include <QFrame>
#include <QShowEvent>
#include <QResizeEvent>
#include <QString>
#include "gui/labeled_vertical_slider.hpp"
#include "gui/color_palette.hpp"
#include "misc.hpp"

namespace Ui {
	class FMOperatorTable;
	enum class FMOperatorParameter;
}

class FMOperatorTable : public QFrame
{
	Q_OBJECT

public:
	explicit FMOperatorTable(QWidget *parent = nullptr);
	~FMOperatorTable() override;

	void setEnvelopeSetNames(std::vector<QString> list);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

	void setOperatorNumber(int n);
	int operatorNumber() const;

	void setValue(Ui::FMOperatorParameter param, int value);

	QString toString() const;

protected:
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

signals:
	void operatorValueChanged(Ui::FMOperatorParameter param, int value);
	void operatorEnableChanged(bool enable);
	void copyEnvelopePressed();
	void pasteEnvelopePressed();
	void pasteEnvelopeFromPressed(QString type);
	void copyOperatorPressed(int num);
	void pasteOperatorPressed(int num);

private slots:
	void on_ssgegCheckBox_stateChanged(int arg1);
	void on_groupBox_toggled(bool arg1);
	void on_groupBox_customContextMenuRequested(const QPoint &pos);

private:
	Ui::FMOperatorTable *ui;
	std::shared_ptr<ColorPalette> palette_;
	int number_;
	std::map<Ui::FMOperatorParameter, LabeledVerticalSlider*> sliderMap_;
	std::vector<QString> envelopeTypes_;

	// Envelope graph
	void resizeGraph();
	void repaintGraph();
};

namespace Ui {
	enum class FMOperatorParameter
	{
		AR, DR, SR, RR, SL, TL, KS, ML, DT, AM, SSGEG
	};
}

#endif // FM_OPERATOR_TABLE_HPP
