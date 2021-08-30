#pragma once

#include <QWidget>
#include <QPixmap>

namespace Dpi
{

inline QPixmap scaledQPixmap(QSize size, int ratio)
{
	QPixmap out(size * ratio);
	out.setDevicePixelRatio(ratio);
	return out;
}

inline QPixmap scaledQPixmap(int width, int height, int ratio)
{
	QPixmap out(width * ratio, height * ratio);
	out.setDevicePixelRatio(ratio);
	return out;
}

inline int iRatio(QWidget const& w)
{
	// devicePixelRatio is int on Qt 5 and qreal on Qt 6.
	// This shouldn't result in *too many* behavior differences though,
	// since devicePixelRatioF is an integer on Qt 5,
	// unless KDE sets QT_SCREEN_SCALE_FACTORS (we can't workaround)
	// or we set DPI scaling to PassThrough (we don't).
	auto ratio = w.devicePixelRatio();

	// Fails on Linux KDE due to https://bugreports.qt.io/browse/QTBUG-95930.
	// Q_ASSERT((int) ratio == ratio);

	return (int) ratio;
}

inline QRect scaleRect(QRect rect, int ratio)
{
	// QRect is insane. QRectF is sane.
	// QRect(QPoint(0, 0), QSize(10, 10)).right() == 9
	// QRectF(QPointF(0., 0.), QSizeF(10., 10.)).right() == 10.
	return QRect(rect.topLeft() * ratio, rect.size() * ratio);
}

} // namespace
