/*
 * Copyright (C) 2018-2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FM_OPERATOR_TABLE_HPP
#define FM_OPERATOR_TABLE_HPP

#include <unordered_map>
#include <memory>
#include <vector>
#include <QFrame>
#include <QShowEvent>
#include <QResizeEvent>
#include <QString>
#include <QPixmap>
#include <QPainter>
#include <QPointF>
#include "gui/labeled_vertical_slider.hpp"
#include "gui/color_palette.hpp"
#include "enum_hash.hpp"

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
	void setGroupEnabled(bool enabled);

	void setDTDisplayType(bool useNegative);

	QString toString() const;

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

signals:
	void operatorValueChanged(Ui::FMOperatorParameter param, int value);
	void operatorEnableChanged(bool enable);
	void copyEnvelopePressed();
	void pasteEnvelopePressed();
	void pasteEnvelopeFromPressed(int typenum);
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
	std::unordered_map<Ui::FMOperatorParameter, LabeledVerticalSlider*> sliderMap_;
	std::vector<QString> envelopeTypes_;
	bool isDTNegative_;
	bool isIgnoreEvent_;

	// Envelope graph
	QPixmap envmap_;
	static constexpr int ENV_H_ = 127;
	static constexpr int ENV_W_ = 200;
	static constexpr int SSGEG_H_ = 35;
	static constexpr int ENV_LINE_W_ = 2;
	static constexpr int ENV_LINE_T_ = 1;
	double xr_, yr_;

	void resizeGraph();
	void repaintGraph();

	inline void drawLine(QPainter& painter, qreal x1, qreal y1, qreal x2, qreal y2)
	{
		painter.drawLine(ENV_LINE_W_ + x1 * xr_ + 1, ENV_LINE_W_ + y1 * yr_ + 1,
						 ENV_LINE_W_ + x2 * xr_ + 1, ENV_LINE_W_ + y2 * yr_ + 1);
	}

	inline void drawLine(QPainter& painter, const QPointF& p1, const QPointF& p2)
	{
		drawLine(painter, p1.x(), p1.y(), p2.x(), p2.y());
	}

	inline void drawLines(QPainter& painter, const QVector<QPointF>& ps)
	{
		for (int i = 1; i < ps.size(); ++i) {
			drawLine(painter, ps[i-1], ps[i]);
		}
	}
};

namespace Ui {
	enum class FMOperatorParameter
	{
		AR, DR, SR, RR, SL, TL, KS, ML, DT, AM, SSGEG
	};
}

#endif // FM_OPERATOR_TABLE_HPP
