#ifndef COLORPALETTEHANDLER_HPP
#define COLORPALETTEHANDLER_HPP

#include <QString>
#include "color_palette.hpp"

class QSettings;

class ColorPaletteHandler
{
public:
	static bool savePalette(const ColorPalette* const palette);
	static bool savePalette(QString file, const ColorPalette* const palette);
	static bool loadPalette(ColorPalette* const palette);
	static bool loadPalette(QString file, ColorPalette* const palette);

private:
	ColorPaletteHandler();
	const static QString organization;
	const static QString application;

	static void save(QSettings& settings, const ColorPalette* const palette);
	static void load(QSettings& settings, ColorPalette* const palette);
};

#endif // COLORPALETTEHANDLER_HPP
