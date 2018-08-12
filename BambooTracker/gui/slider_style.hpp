#ifndef SLIDER_STYLE_HPP
#define SLIDER_STYLE_HPP

#include <QProxyStyle>

class SliderStyle : public QProxyStyle
{
public:
	virtual int styleHint (StyleHint hint, const QStyleOption* option = nullptr,
						   const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const;
};

#endif // SLIDER_STYLE_HPP
