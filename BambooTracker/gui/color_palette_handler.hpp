/*
 * Copyright (C) 2018-2020 Rerrah
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
