#ifndef COLOR_PALETTE_HPP
#define COLOR_PALETTE_HPP

#include <QColor>

class ColorPalette
{
public:
	ColorPalette();

	// Instrument list
	QColor ilistTextColor, ilistBackColor;
	QColor ilistSelTextColor, ilistSelBackColor;
	QColor ilistHovTextColor, ilistHovBackColor;
	QColor ilistHovSelTextColor, ilistHovSelBackColor;

	// Instrument editor
	QColor instFMEnvLine1Color_, instFMEnvLine2Color_;
	QColor instFMEnvBackColor_, instFMEnvBorderColor_;
	QColor instFmEnvCirclePenColor, instFmEnvCircleBrushColor;
	QColor instSeqTagColor;
	QColor instSeqHovColor;
	QColor instSeqLoopBackColor, instSeqLoopColor, instSeqLoopEdgeColor;
	QColor instSeqReleaseBackColor, instSeqReleaseColor, instSeqReleaseEdgeColor;
	QColor instSeqLoopTextColor, instSeqReleaseTextColor;
	QColor instSeqCellColor, instSeqCellTextColor;
	QColor instSeqBorderColor;
	QColor instSeqMaskColor;

	// Order list
	QColor odrDefTextColor, odrDefRowColor;
	QColor odrCurTextColor, odrCurRowColor;
	QColor odrCurEditRowColor;
	QColor odrCurCellColor;
	QColor odrPlayTextColor, odrPlayRowColor;
	QColor odrSelCellColor;
	QColor odrHovCellColor;
	QColor odrRowNumColor;
	QColor odrHeaderTextColor, odrHeaderRowColor;
	QColor odrBorderColor;

	// Pattern editor
	QColor ptnDefTextColor, ptnDefRowColor, ptnMkRowColor;
	QColor ptnCurTextColor, ptnCurRowColor, ptnCurEditRowColor, ptnCurCellColor;
	QColor ptnPlayTextColor, ptnPlayRowColor;
	QColor ptnSelCellColor;
	QColor ptnHovCellColor;
	QColor ptnDefStepNumColor, ptnMkStepNumColor;
	QColor ptnToneColor, ptnInstColor, ptnVolColor, ptnEffIDColor, ptnEffValColor;
	QColor ptnErrorColor;
	QColor ptnHeaderTextColor, ptnHeaderRowColor;
	QColor ptnMaskColor;
	QColor ptnBorderColor;
	QColor ptnMuteColor, ptnUnmuteColor;
};

#endif // COLOR_PALETTE_HPP
