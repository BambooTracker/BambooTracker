#ifndef COLORPALETTEHANDLER_HPP
#define COLORPALETTEHANDLER_HPP

#include <memory>
#include <QString>
#include "color_palette.hpp"

class QSettings;

class ColorPaletteHandler
{
public:
	static bool savePalette(std::weak_ptr<ColorPalette> palette);
	static bool savePalette(QString file, std::weak_ptr<ColorPalette> palette);
	static bool loadPalette(std::weak_ptr<ColorPalette> palette);
	static bool loadPalette(QString file, std::weak_ptr<ColorPalette> palette);

private:
	ColorPaletteHandler();
	const static QString organization;
	const static QString application;

	static void save(QSettings& settings, std::weak_ptr<ColorPalette> palette);
	static void load(QSettings& settings, std::weak_ptr<ColorPalette> palette);
};

#endif // COLORPALETTEHANDLER_HPP
