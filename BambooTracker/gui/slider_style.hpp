#ifndef SLIDER_STYLE_HPP
#define SLIDER_STYLE_HPP

#include <QProxyStyle>

class SliderStyle : public QProxyStyle
{
public:
	virtual int styleHint (StyleHint hint, const QStyleOption* option = 0,
						   const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
};

#endif // SLIDER_STYLE_HPP
