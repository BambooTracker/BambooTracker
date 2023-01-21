/*
 * Copyright (C) 2018-2023 Rerrah
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

#include "color_palette.hpp"
#include <QSettings>
#include "gui/gui_utils.hpp"

ColorPalette::ColorPalette()
{
	// Instrument list
	ilistTextColor = QColor::fromRgb(255, 255, 255, 255);
	ilistBackColor = QColor::fromRgb(0, 0, 0, 255);
	ilistSelBackColor = QColor::fromRgb(110, 90, 140, 255);
	ilistHovBackColor = QColor::fromRgb(255, 255, 255, 75);
	ilistHovSelBackColor = QColor::fromRgb(140, 120, 170, 255);

	// Instrument editor
	instFMEnvLine1Color = QColor::fromRgb(242, 38, 19, 255);
	instFMEnvLine2Color = QColor::fromRgb(46, 204, 113, 255);
	instFMEnvLine3Color = QColor::fromRgb(38, 38, 255, 255);
	instFMEnvGridColor = QColor::fromRgb(40, 40, 40, 63);
	instFMEnvBackColor = QColor::fromRgb(255, 255, 255, 0);
	instFMEnvBorderColor = QColor::fromRgb(125, 125, 125, 255);
	instFMAlForeColor = QColor::fromRgb(107, 185, 240, 255);
	instFMAlBackColor = QColor::fromRgb(255, 255, 255, 0);
	instSeqLoopBackColor = QColor::fromRgb(25, 25, 25, 255);
	instSeqReleaseBackColor = QColor::fromRgb(0, 0, 0, 255);
	instSeqLoopColor = QColor::fromRgb(210, 40, 180, 127);
	instSeqReleaseColor = QColor::fromRgb(40, 170, 200, 127);
	instSeqLoopEdgeColor = QColor::fromRgb(180, 20, 180, 127);
	instSeqReleaseEdgeColor = QColor::fromRgb(40, 170, 150, 127);
	instSeqTagColor = QColor::fromRgb(255, 255, 255, 255);
	instSeqHovColor = QColor::fromRgb(255, 255, 255, 63);
	instSeqLoopTextColor = QColor::fromRgb(24, 223, 172, 255);
	instSeqReleaseTextColor = QColor::fromRgb(24, 223, 172, 255);
	instSeqCellColor = QColor::fromRgb(38, 183, 173, 255);
	instSeqCellTextColor = QColor::fromRgb(255, 255, 255);
	instSeqBorderColor = QColor::fromRgb(50, 50, 50, 255);
	instSeqMaskColor = QColor::fromRgb(0, 0, 0, 128);
	instSeqOddColColor = QColor::fromRgb(255, 255, 255, 31);

	// ADPCM sample editor
	instADPCMMemAllColor = QColor::fromRgb(210, 40, 180, 150);
	instADPCMMemCurColor = QColor::fromRgb(210, 40, 180, 255);
	instADPCMMemBackColor = QColor::fromRgb(0, 0, 0, 255);
	instADPCMSampViewForeColor = QColor::fromRgb(38, 183, 173, 255);
	instADPCMSampViewBackColor = QColor::fromRgb(0, 0, 0, 255);
	instADPCMSampViewCenterColor = QColor::fromRgb(63, 63, 63, 255);
	instADPCMSampViewGridColor = QColor::fromRgb(63, 63, 63, 170);
	instADPCMSampViewDrawColor = QColor::fromRgb(255, 0, 0, 255);
	instADPCMSampViewDirectDrawColor = QColor::fromRgb(255, 150, 0, 255);
	instADPCMSampViewRepeatBeginColor = QColor::fromRgb(204, 102, 255, 255);
	instADPCMSampViewRepeatEndColor = QColor::fromRgb(137, 167, 255, 255);

	// Tone/Noise editor
	tnToneCellColor = QColor::fromRgb(225, 209, 47, 255);
	tnToneTextColor = QColor::fromRgb(255, 255, 126, 255);
	tnNoiseCellColor = QColor::fromRgb(210, 40, 180, 255);
	tnNoiseTextColor = QColor::fromRgb(240, 110, 220, 255);
	tnToneBackColor = QColor::fromRgb(0, 0, 0, 255);
	tnNoiseBackColor = QColor::fromRgb(25, 25, 25, 255);

	// Order list
	odrDefTextColor = QColor::fromRgb(180, 180, 180, 255);
	odrDefRowColor = QColor::fromRgb(40, 40, 80, 255);
	odrCurTextColor = QColor::fromRgb(255, 255, 255, 255);
	odrCurRowColor = QColor::fromRgb(110, 90, 140, 255);
	odrCurEditRowColor = QColor::fromRgb(140, 90, 110, 255);
	odrCurCellColor = QColor::fromRgb(255, 255, 255, 127);
	odrPlayRowColor = QColor::fromRgb(90, 90, 140, 255);
	odrSelCellColor = QColor::fromRgb(100, 100, 200, 192);
	odrHovCellColor = QColor::fromRgb(255, 255, 255, 64);
	odrRowNumColor = QColor::fromRgb(255, 200, 180, 255);
	odrHeaderTextColor = QColor::fromRgb(240, 240, 200, 255);
	odrHeaderRowColor = QColor::fromRgb(60, 60, 60, 255);
	odrBorderColor = QColor::fromRgb(0, 0, 0, 255);
	odrHeaderBorderColor = QColor::fromRgb(0, 0, 0, 255);
	odrBackColor = QColor::fromRgb(0, 0, 0, 255);
	odrUnfocusedShadowColor = QColor::fromRgb(0, 0, 0, 47);

	// Pattern editor
	ptnDefTextColor = QColor::fromRgb(180, 180, 180, 255);
	ptnDefStepColor = QColor::fromRgb(0, 0, 40, 255);
	ptnHl1StepColor = QColor::fromRgb(30, 40, 70, 255);
	ptnHl2StepColor = QColor::fromRgb(60, 60, 100, 255);
	ptnCurTextColor = QColor::fromRgb(255, 255, 255, 255);
	ptnCurStepColor = QColor::fromRgb(110, 90, 140, 255);
	ptnCurEditStepColor = QColor::fromRgb(140, 90, 110, 255);
	ptnCurCellColor = QColor::fromRgb(255, 255, 255, 127);
	ptnPlayStepColor = QColor::fromRgb(90, 90, 140, 255);
	ptnSelCellColor = QColor::fromRgb(100, 100, 200, 192);
	ptnHovCellColor = QColor::fromRgb(255, 255, 255, 64);
	ptnDefStepNumColor = QColor::fromRgb(255, 200, 180, 255);
	ptnHl1StepNumColor = QColor::fromRgb(255, 140, 160, 255);
	ptnHl2StepNumColor = QColor::fromRgb(255, 140, 160, 255);
	ptnNoteColor = QColor::fromRgb(210, 230, 64, 255);
	ptnInstColor = QColor::fromRgb(82, 179, 217, 255);
	ptnVolColor = QColor::fromRgb(226, 156, 80, 255);
	ptnEffColor = QColor::fromRgb(42, 187, 155, 255);
	ptnErrorColor = QColor::fromRgb(255, 0, 0, 255);
	ptnHeaderTextColor = QColor::fromRgb(240, 240, 200, 255);
	ptnHeaderRowColor = QColor::fromRgb(60, 60, 60, 255);
	ptnMaskColor = QColor::fromRgb(0, 0, 0, 127);
	ptnBorderColor = QColor::fromRgb(0, 0, 0, 255);
	ptnHeaderBorderColor = QColor::fromRgb(0, 0, 0, 255);
	ptnMuteColor = QColor::fromRgb(255, 0, 0, 255);
	ptnUnmuteColor = QColor::fromRgb(0, 255, 0, 255);
	ptnBackColor = QColor::fromRgb(0, 0, 0, 255);
	ptnMarkerColor = QColor::fromRgb(255, 255, 255, 128);
	ptnUnfocusedShadowColor = QColor::fromRgb(0, 0, 0, 47);

	// Wave visual
	wavBackColor = QColor::fromRgb(0, 0, 33, 255);
	wavDrawColor = QColor::fromRgb(82, 179, 217, 255);
}

namespace io
{
namespace
{
// config path (*nix): ~/.config/<organization>/<file>.ini
const QString FILE = "BambooTracker";

void save(QSettings& settings, const ColorPalette* const palette)
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

void load(QSettings& settings, ColorPalette* const palette)
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
}

bool savePalette(const ColorPalette* const palette)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, io::ORGANIZATION_NAME, FILE);
		save(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

bool savePalette(const QString &file, const ColorPalette* const palette)
{
	try {
		QSettings settings(file, QSettings::IniFormat);
		save(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

bool loadPalette(ColorPalette* const palette)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, io::ORGANIZATION_NAME, FILE);
		load(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

bool loadPalette(const QString &file, ColorPalette* const palette)
{
	try {
		QSettings settings(file, QSettings::IniFormat);
		load(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}
}
