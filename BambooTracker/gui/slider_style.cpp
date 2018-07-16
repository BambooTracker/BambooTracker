#include "slider_style.hpp"

int SliderStyle::styleHint (StyleHint hint, const QStyleOption* option,
							const QWidget* widget, QStyleHintReturn* returnData) const
{
	if (hint == QStyle::SH_Slider_AbsoluteSetButtons) {
		return Qt::LeftButton;
	}
	else {
		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
}
