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

#include "color_palette_handler.hpp"
#include <QSettings>
#include <QColor>

// config path (*nix): ~/.config/<organization>/<application>.ini
const QString ColorPaletteHandler::organization = "BambooTracker";
const QString ColorPaletteHandler::application = "BambooTracker";

ColorPaletteHandler::ColorPaletteHandler() {}

bool ColorPaletteHandler::savePalette(const ColorPalette* const palette)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ColorPaletteHandler::organization, ColorPaletteHandler::application);
		save(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

bool ColorPaletteHandler::savePalette(QString file, const ColorPalette* const palette)
{
	try {
		QSettings settings(file, QSettings::IniFormat);
		save(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

void ColorPaletteHandler::save(QSettings& settings, const ColorPalette* const palette)
{
	settings.beginGroup("PatternEditor");
	settings.setValue("defaultStepText", palette->ptnDefTextColor.name(QColor::HexArgb));
	settings.setValue("defaultStepBackground", palette->ptnDefStepColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep1Background", palette->ptnHl1StepColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep2Background", palette->ptnHl2StepColor.name(QColor::HexArgb));
	settings.setValue("currentStepText", palette->ptnCurTextColor.name(QColor::HexArgb));
	settings.setValue("currentStepBackground", palette->ptnCurStepColor.name(QColor::HexArgb));
	settings.setValue("currentEditingStepBackground", palette->ptnCurEditStepColor.name(QColor::HexArgb));
	settings.setValue("currentCellBackground", palette->ptnCurCellColor.name(QColor::HexArgb));
	settings.setValue("currentPlayingStepBackground", palette->ptnPlayStepColor.name(QColor::HexArgb));
	settings.setValue("selectionBackground", palette->ptnSelCellColor.name(QColor::HexArgb));
	settings.setValue("hoveredCellBackground", palette->ptnHovCellColor.name(QColor::HexArgb));
	settings.setValue("defaultStepNumber", palette->ptnDefStepNumColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep1Number", palette->ptnHl1StepNumColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep2Number", palette->ptnHl2StepNumColor.name(QColor::HexArgb));
	settings.setValue("noteText", palette->ptnNoteColor.name(QColor::HexArgb));
	settings.setValue("instrumentText", palette->ptnInstColor.name(QColor::HexArgb));
	settings.setValue("volumeText", palette->ptnVolColor.name(QColor::HexArgb));
	settings.setValue("effectText", palette->ptnEffColor.name(QColor::HexArgb));
	settings.setValue("errorText", palette->ptnErrorColor.name(QColor::HexArgb));
	settings.setValue("headerText", palette->ptnHeaderTextColor.name(QColor::HexArgb));
	settings.setValue("headerBackground", palette->ptnHeaderRowColor.name(QColor::HexArgb));
	settings.setValue("mask", palette->ptnMaskColor.name(QColor::HexArgb));
	settings.setValue("border", palette->ptnBorderColor.name(QColor::HexArgb));
	settings.setValue("headerBorder", palette->ptnHeaderBorderColor.name(QColor::HexArgb));
	settings.setValue("mute", palette->ptnMuteColor.name(QColor::HexArgb));
	settings.setValue("unmute", palette->ptnUnmuteColor.name(QColor::HexArgb));
	settings.setValue("background", palette->ptnBackColor.name(QColor::HexArgb));
	settings.setValue("marker", palette->ptnMarkerColor.name(QColor::HexArgb));
	settings.setValue("unfocusedShadow", palette->ptnUnfocusedShadowColor.name(QColor::HexArgb));
	settings.endGroup();

	settings.beginGroup("OrderList");
	settings.setValue("defaultRowText", palette->odrDefTextColor.name(QColor::HexArgb));
	settings.setValue("defaultRowBackground", palette->odrDefRowColor.name(QColor::HexArgb));
	settings.setValue("currentRowText", palette->odrCurTextColor.name(QColor::HexArgb));
	settings.setValue("currentRowBackground", palette->odrCurRowColor.name(QColor::HexArgb));
	settings.setValue("currentEditingRowBackground", palette->odrCurEditRowColor.name(QColor::HexArgb));
	settings.setValue("currentCellBackground", palette->odrCurCellColor.name(QColor::HexArgb));
	settings.setValue("currentPlayingRowBackground", palette->odrPlayRowColor.name(QColor::HexArgb));
	settings.setValue("selectionBackground", palette->odrSelCellColor.name(QColor::HexArgb));
	settings.setValue("hoveredCellBackground", palette->odrHovCellColor.name(QColor::HexArgb));
	settings.setValue("rowNumber", palette->odrRowNumColor.name(QColor::HexArgb));
	settings.setValue("headerText", palette->odrHeaderTextColor.name(QColor::HexArgb));
	settings.setValue("headerBackground", palette->odrHeaderRowColor.name(QColor::HexArgb));
	settings.setValue("border", palette->odrBorderColor.name(QColor::HexArgb));
	settings.setValue("headerBorder", palette->odrHeaderBorderColor.name(QColor::HexArgb));
	settings.setValue("background", palette->odrBackColor.name(QColor::HexArgb));
	settings.setValue("unfocusedShadow", palette->odrUnfocusedShadowColor.name(QColor::HexArgb));
	settings.endGroup();

	settings.beginGroup("InstrumentList");
	settings.setValue("defaultText", palette->ilistTextColor.name(QColor::HexArgb));
	settings.setValue("background", palette->ilistBackColor.name(QColor::HexArgb));
	settings.setValue("selectedBackground", palette->ilistSelBackColor.name(QColor::HexArgb));
	settings.setValue("hoveredBackground", palette->ilistHovBackColor.name(QColor::HexArgb));
	settings.setValue("selectedHoveredBackground", palette->ilistHovSelBackColor.name(QColor::HexArgb));
	settings.endGroup();

	settings.beginGroup("Oscilloscope");
	settings.setValue("background", palette->wavBackColor.name(QColor::HexArgb));
	settings.setValue("foreground", palette->wavDrawColor.name(QColor::HexArgb));
	settings.endGroup();
}

bool ColorPaletteHandler::loadPalette(ColorPalette* const palette)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ColorPaletteHandler::organization, ColorPaletteHandler::application);
		load(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

bool ColorPaletteHandler::loadPalette(QString file, ColorPalette* const palette)
{
	try {
		QSettings settings(file, QSettings::IniFormat);
		load(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

void ColorPaletteHandler::load(QSettings& settings, ColorPalette* const palette)
{
	settings.beginGroup("PatternEditor");
	palette->ptnDefTextColor = settings.value("defaultStepText", palette->ptnDefTextColor).value<QColor>();
	palette->ptnDefStepColor = settings.value("defaultStepBackground", palette->ptnDefStepColor).value<QColor>();
	palette->ptnHl1StepColor = settings.value("highlightedStep1Background", palette->ptnHl1StepColor).value<QColor>();
	palette->ptnHl2StepColor = settings.value("highlightedStep2Background", palette->ptnHl2StepColor).value<QColor>();
	palette->ptnCurTextColor = settings.value("currentStepText", palette->ptnCurTextColor).value<QColor>();
	palette->ptnCurStepColor = settings.value("currentStepBackground", palette->ptnCurStepColor).value<QColor>();
	palette->ptnCurEditStepColor = settings.value("currentEditingStepBackground", palette->ptnCurEditStepColor).value<QColor>();
	palette->ptnCurCellColor = settings.value("currentCellBackground", palette->ptnCurCellColor).value<QColor>();
	palette->ptnPlayStepColor = settings.value("currentPlayingStepBackground", palette->ptnPlayStepColor).value<QColor>();
	palette->ptnSelCellColor = settings.value("selectionBackground", palette->ptnSelCellColor).value<QColor>();
	palette->ptnHovCellColor = settings.value("hoveredCellBackground", palette->ptnHovCellColor).value<QColor>();
	palette->ptnDefStepNumColor = settings.value("defaultStepNumber", palette->ptnDefStepNumColor).value<QColor>();
	palette->ptnHl1StepNumColor = settings.value("highlightedStep1Number", palette->ptnHl1StepNumColor).value<QColor>();
	palette->ptnHl2StepNumColor = settings.value("highlightedStep2Number", palette->ptnHl2StepNumColor).value<QColor>();
	palette->ptnNoteColor = settings.value("noteText", palette->ptnNoteColor).value<QColor>();
	palette->ptnInstColor = settings.value("instrumentText", palette->ptnInstColor).value<QColor>();
	palette->ptnVolColor = settings.value("volumeText", palette->ptnVolColor).value<QColor>();
	palette->ptnEffColor = settings.value("effectText", palette->ptnEffColor).value<QColor>();
	palette->ptnErrorColor = settings.value("errorText", palette->ptnErrorColor).value<QColor>();
	palette->ptnHeaderTextColor = settings.value("headerText", palette->ptnHeaderTextColor).value<QColor>();
	palette->ptnHeaderRowColor = settings.value("headerBackground", palette->ptnHeaderRowColor).value<QColor>();
	palette->ptnMaskColor = settings.value("mask", palette->ptnMaskColor).value<QColor>();
	palette->ptnBorderColor = settings.value("border", palette->ptnBorderColor).value<QColor>();
	palette->ptnHeaderBorderColor = settings.value("headerBorder", palette->ptnHeaderBorderColor).value<QColor>();
	palette->ptnMuteColor = settings.value("mute", palette->ptnMuteColor).value<QColor>();
	palette->ptnUnmuteColor = settings.value("unmute", palette->ptnUnmuteColor).value<QColor>();
	palette->ptnBackColor = settings.value("background", palette->ptnBackColor).value<QColor>();
	palette->ptnMarkerColor = settings.value("marker", palette->ptnMarkerColor).value<QColor>();
	palette->ptnUnfocusedShadowColor = settings.value("unfocusedShadow", palette->ptnUnfocusedShadowColor).value<QColor>();
	settings.endGroup();

	settings.beginGroup("OrderList");
	palette->odrDefTextColor = settings.value("defaultRowText", palette->odrDefTextColor).value<QColor>();
	palette->odrDefRowColor = settings.value("defaultRowBackground", palette->odrDefRowColor).value<QColor>();
	palette->odrCurTextColor = settings.value("currentRowText", palette->odrCurTextColor).value<QColor>();
	palette->odrCurRowColor = settings.value("currentRowBackground", palette->odrCurRowColor).value<QColor>();
	palette->odrCurEditRowColor = settings.value("currentEditingRowBackground", palette->odrCurEditRowColor).value<QColor>();
	palette->odrCurCellColor = settings.value("currentCellBackground", palette->odrCurCellColor).value<QColor>();
	palette->odrPlayRowColor = settings.value("currentPlayingRowBackground", palette->odrPlayRowColor).value<QColor>();
	palette->odrSelCellColor = settings.value("selectionBackground", palette->odrSelCellColor).value<QColor>();
	palette->odrHovCellColor = settings.value("hoveredCellBackground", palette->odrHovCellColor).value<QColor>();
	palette->odrRowNumColor = settings.value("rowNumber", palette->odrRowNumColor).value<QColor>();
	palette->odrHeaderTextColor = settings.value("headerText", palette->odrHeaderTextColor).value<QColor>();
	palette->odrHeaderRowColor = settings.value("headerBackground", palette->odrHeaderRowColor).value<QColor>();
	palette->odrBorderColor = settings.value("border", palette->odrBorderColor).value<QColor>();
	palette->odrHeaderBorderColor = settings.value("headerBorder", palette->odrHeaderBorderColor).value<QColor>();
	palette->odrBackColor = settings.value("background", palette->odrBackColor).value<QColor>();
	palette->odrUnfocusedShadowColor = settings.value("unfocusedShadow", palette->odrUnfocusedShadowColor).value<QColor>();
	settings.endGroup();

	settings.beginGroup("InstrumentList");
	palette->ilistTextColor = settings.value("defaultText", palette->ilistTextColor).value<QColor>();
	palette->ilistBackColor = settings.value("background", palette->ilistBackColor).value<QColor>();
	palette->ilistSelBackColor = settings.value("selectedBackground", palette->ilistSelBackColor).value<QColor>();
	palette->ilistHovBackColor = settings.value("hoveredBackground", palette->ilistHovBackColor).value<QColor>();
	palette->ilistHovSelBackColor = settings.value("selectedHoveredBackground", palette->ilistHovSelBackColor).value<QColor>();
	settings.endGroup();

	settings.beginGroup("Oscilloscope");
	palette->wavBackColor = settings.value("background", palette->wavBackColor).value<QColor>();
	palette->wavDrawColor = settings.value("foreground", palette->wavDrawColor).value<QColor>();
	settings.endGroup();
}
