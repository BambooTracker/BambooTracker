#include "color_palette.hpp"

ColorPalette::ColorPalette()
{
	// Instrument list
	ilistTextColor = QColor::fromRgb(255, 255, 255, 255);
	ilistBackColor = QColor::fromRgb(0, 0, 0, 255);
	ilistSelTextColor = QColor::fromRgb(255, 255, 255, 255);
	ilistSelBackColor = QColor::fromRgb(110, 90, 140, 255);
	ilistHovTextColor = QColor::fromRgb(255, 255, 255, 255);
	ilistHovBackColor = QColor::fromRgb(255, 255, 255, 75);
	ilistHovSelTextColor = QColor::fromRgb(255, 255, 255, 255);
	ilistHovSelBackColor = QColor::fromRgb(140, 120, 170, 255);

	// Instrument editor
	instFMEnvLine1Color = QColor::fromRgb(242, 38, 19, 255);
	instFMEnvLine2Color = QColor::fromRgb(46, 204, 113, 255);
	instFMEnvBackColor = QColor::fromRgb(255, 255, 255, 0);
	instFMEnvBorderColor = QColor::fromRgb(125, 125, 125, 255);
	instFMEnvCirclePenColor = QColor::fromRgb(0, 0, 0, 255);
	instFMEnvCircleBrushColor = QColor::fromRgb(255, 255, 255, 255);
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

	// Order list
	odrDefTextColor = QColor::fromRgb(180, 180, 180, 255);
	odrDefRowColor = QColor::fromRgb(40, 40, 80, 255);
	odrCurTextColor = QColor::fromRgb(255, 255, 255, 255);
	odrCurRowColor = QColor::fromRgb(110, 90, 140, 255);
	odrCurEditRowColor = QColor::fromRgb(140, 90, 110, 255);
	odrCurCellColor = QColor::fromRgb(255, 255, 255, 127);
	odrPlayTextColor = QColor::fromRgb(255, 255, 255, 255);
	odrPlayRowColor = QColor::fromRgb(90, 90, 140, 255);
	odrSelCellColor = QColor::fromRgb(100, 100, 200, 192);
	odrHovCellColor = QColor::fromRgb(255, 255, 255, 64);
	odrRowNumColor = QColor::fromRgb(255, 200, 180, 255);
	odrHeaderTextColor = QColor::fromRgb(240, 240, 200, 255);
	odrHeaderRowColor = QColor::fromRgb(60, 60, 60, 255);
	odrBorderColor = QColor::fromRgb(120, 120, 120, 255);

	// Pattern editor
	ptnDefTextColor = QColor::fromRgb(180, 180, 180, 255);
	ptnDefRowColor = QColor::fromRgb(0, 0, 40, 255);
	ptnHl1RowColor = QColor::fromRgb(30, 40, 70, 255);
	ptnHl2RowColor = QColor::fromRgb(60, 60, 100, 255);
	ptnCurTextColor = QColor::fromRgb(255, 255, 255, 255);
	ptnCurRowColor = QColor::fromRgb(110, 90, 140, 255);
	ptnCurEditRowColor = QColor::fromRgb(140, 90, 110, 255);
	ptnCurCellColor = QColor::fromRgb(255, 255, 255, 127);
	ptnPlayTextColor = QColor::fromRgb(255, 255, 255, 255);
	ptnPlayRowColor = QColor::fromRgb(90, 90, 140, 255);
	ptnSelCellColor = QColor::fromRgb(100, 100, 200, 192);
	ptnHovCellColor = QColor::fromRgb(255, 255, 255, 64);
	ptnDefStepNumColor = QColor::fromRgb(255, 200, 180, 255);
	ptnHl1StepNumColor = QColor::fromRgb(255, 140, 160, 255);
	ptnHl2StepNumColor = QColor::fromRgb(255, 140, 160, 255);
	ptnToneColor = QColor::fromRgb(210, 230, 64, 255);
	ptnInstColor = QColor::fromRgb(82, 179, 217, 255);
	ptnVolColor = QColor::fromRgb(226, 156, 80, 255);
	ptnEffIDColor = QColor::fromRgb(42, 187, 155, 255);
	ptnEffValColor = QColor::fromRgb(42, 187, 155, 255);
	ptnErrorColor = QColor::fromRgb(255, 0, 0, 255);
	ptnHeaderTextColor = QColor::fromRgb(240, 240, 200, 255);
	ptnHeaderRowColor = QColor::fromRgb(60, 60, 60, 255);
	ptnMaskColor = QColor::fromRgb(0, 0, 0, 128);
	ptnBorderColor = QColor::fromRgb(120, 120, 120, 255);
	ptnMuteColor = QColor::fromRgb(255, 0, 0, 255);
	ptnUnmuteColor = QColor::fromRgb(0, 255, 0, 255);
}
