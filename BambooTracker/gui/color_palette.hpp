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

#ifndef COLOR_PALETTE_HPP
#define COLOR_PALETTE_HPP

#include <QColor>

class ColorPalette
{
public:
	ColorPalette();

	// Instrument list
	QColor ilistTextColor, ilistBackColor;
	QColor ilistSelBackColor;
	QColor ilistHovBackColor;
	QColor ilistHovSelBackColor;

	// Instrument editor
	QColor instFMEnvLine1Color, instFMEnvLine2Color, instFMEnvLine3Color;
	QColor instFMEnvGridColor;
	QColor instFMEnvBackColor, instFMEnvBorderColor;
	QColor instFMAlForeColor, instFMAlBackColor;
	QColor instSeqTagColor;
	QColor instSeqHovColor;
	QColor instSeqLoopBackColor, instSeqLoopColor, instSeqLoopEdgeColor;
	QColor instSeqReleaseBackColor, instSeqReleaseColor, instSeqReleaseEdgeColor;
	QColor instSeqLoopTextColor, instSeqReleaseTextColor;
	QColor instSeqCellColor, instSeqCellTextColor;
	QColor instSeqBorderColor;
	QColor instSeqMaskColor;
	QColor instSeqOddColColor;

	// ADPCM sample editor
	QColor instADPCMMemAllColor, instADPCMMemCurColor, instADPCMMemBackColor;
	QColor instADPCMSampViewForeColor, instADPCMSampViewBackColor, instADPCMSampViewCenterColor;
	QColor instADPCMSampViewGridColor, instADPCMSampViewDrawColor, instADPCMSampViewDirectDrawColor;
	QColor instADPCMSampViewRepeatBeginColor, instADPCMSampViewRepeatEndColor;

	// Tone/Noise editor
	QColor tnToneCellColor, tnToneTextColor;
	QColor tnNoiseCellColor, tnNoiseTextColor;
	QColor tnToneBackColor, tnNoiseBackColor;

	// Order list
	QColor odrDefTextColor, odrDefRowColor;
	QColor odrCurTextColor, odrCurRowColor;
	QColor odrCurEditRowColor;
	QColor odrCurCellColor;
	QColor odrPlayRowColor;
	QColor odrSelCellColor;
	QColor odrHovCellColor;
	QColor odrRowNumColor;
	QColor odrHeaderTextColor, odrHeaderRowColor;
	QColor odrBorderColor, odrHeaderBorderColor;
	QColor odrBackColor;
	QColor odrUnfocusedShadowColor;

	// Pattern editor
	QColor ptnDefTextColor, ptnDefStepColor, ptnHl1StepColor, ptnHl2StepColor;
	QColor ptnCurTextColor, ptnCurStepColor, ptnCurEditStepColor, ptnCurCellColor;
	QColor ptnPlayStepColor;
	QColor ptnSelCellColor;
	QColor ptnHovCellColor;
	QColor ptnDefStepNumColor, ptnHl1StepNumColor, ptnHl2StepNumColor;
	QColor ptnNoteColor, ptnInstColor, ptnVolColor, ptnEffColor;
	QColor ptnErrorColor;
	QColor ptnHeaderTextColor, ptnHeaderRowColor;
	QColor ptnMaskColor;
	QColor ptnBorderColor, ptnHeaderBorderColor;
	QColor ptnMuteColor, ptnUnmuteColor;
	QColor ptnBackColor;
	QColor ptnMarkerColor;
	QColor ptnUnfocusedShadowColor;

	// Wave visual
	QColor wavBackColor;
	QColor wavDrawColor;
};

namespace io
{
bool savePalette(const ColorPalette* const palette);
bool savePalette(const QString& file, const ColorPalette* const palette);
bool loadPalette(ColorPalette* const palette);
bool loadPalette(const QString& file, ColorPalette* const palette);
}

#endif // COLOR_PALETTE_HPP
