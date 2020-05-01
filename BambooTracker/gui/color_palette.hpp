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
	QColor odrBorderColor;
	QColor odrBackColor;

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
	QColor ptnBorderColor;
	QColor ptnMuteColor, ptnUnmuteColor;
	QColor ptnBackColor;
	QColor ptnMarkerColor;

	// Wave visual
	QColor wavBackColor;
	QColor wavDrawColor;
};

#endif // COLOR_PALETTE_HPP
