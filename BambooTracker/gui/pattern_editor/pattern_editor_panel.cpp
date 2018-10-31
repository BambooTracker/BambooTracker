#include "pattern_editor_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QPoint>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <algorithm>
#include <vector>
#include <utility>
#include "gui/event_guard.hpp"
#include "gui/command/pattern/pattern_commands_qt.hpp"

PatternEditorPanel::PatternEditorPanel(QWidget *parent)
	: QWidget(parent),
	  leftTrackNum_(0),
	  curSongNum_(0),
	  curPos_{ 0, 0, 0, 0, },
	  hovPos_{ -1, -1, -1, -1 },
	  editPos_{ -1, -1, -1, -1 },
	  mousePressPos_{ -1, -1, -1, -1 },
	  mouseReleasePos_{ -1, -1, -1, -1 },
	  selLeftAbovePos_{ -1, -1, -1, -1 },
	  selRightBelowPos_{ -1, -1, -1, -1 },
	  shiftPressedPos_{ -1, -1, -1, -1 },
	  isIgnoreToSlider_(false),
	  isIgnoreToOrder_(false),
	  isPressedPlus_(false),
	  isPressedMinus_(false),
	  entryCnt_(0),
	  selectAllState_(-1),
	  isMuteElse_(false),
	  hlCnt_(8)
{	
	/* Font */
	headerFont_ = QApplication::font();
	headerFont_.setPointSize(10);
	stepFont_ = QFont("Monospace", 10);
	stepFont_.setStyleHint(QFont::TypeWriter);
	stepFont_.setStyleStrategy(QFont::ForceIntegerMetrics);
	// Check font size
	QFontMetrics metrics(stepFont_);
	stepFontWidth_ = metrics.width('0');
	stepFontAscend_ = metrics.ascent();
	stepFontHeight_ = metrics.height();
	stepFontLeading_ = metrics.leading();
	QFontMetrics m(headerFont_);

	/* Width & height */
	widthSpace_ = stepFontWidth_ / 2;
	stepNumWidth_ = stepFontWidth_ * 2 + widthSpace_;
	toneNameWidth_ = stepFontWidth_ * 3;
	instWidth_ = stepFontWidth_ * 2;
	volWidth_ = stepFontWidth_ * 2;
	effIDWidth_ = stepFontWidth_ * 2;
	effValWidth_ = stepFontWidth_ * 2;
	effWidth_ = effIDWidth_ + effValWidth_ + stepFontWidth_;
	baseTrackWidth_ = toneNameWidth_ + instWidth_ + volWidth_
					  + effIDWidth_ + effValWidth_ + stepFontWidth_ * 4;
	hdEffCompandButtonWidth_ = m.width("+");
	hdMuteToggleWidth_ = baseTrackWidth_ - hdEffCompandButtonWidth_ - widthSpace_ * 3;
	headerHeight_ = stepFontHeight_ * 2;
	hdPlusY_ = headerHeight_ / 4 + m.height() / 2 - m.leading() / 2 - m.descent();
	hdMinusY_ = headerHeight_ / 2 + hdPlusY_;

	/* Color */
	defTextColor_ = QColor::fromRgb(180, 180, 180);
	defRowColor_ = QColor::fromRgb(0, 0, 40);
	mkRowColor_ = QColor::fromRgb(40, 40, 80);
	curTextColor_ = QColor::fromRgb(255, 255, 255);
	curRowColor_ = QColor::fromRgb(110, 90, 140);
	curRowColorEditable_ = QColor::fromRgb(140, 90, 110);
	curCellColor_ = QColor::fromRgb(255, 255, 255, 127);
	playTextColor_ = curTextColor_;
	playRowColor_ = QColor::fromRgb(90, 90, 140);
	selCellColor_ = QColor::fromRgb(100, 100, 200, 192);
	hovCellColor_ = QColor::fromRgb(255, 255, 255, 64);
	defStepNumColor_ = QColor::fromRgb(255, 200, 180);
	mkStepNumColor_ = QColor::fromRgb(255, 140, 160);
	toneColor_ = QColor::fromRgb(210, 230, 64);
	instColor_ = QColor::fromRgb(82, 179, 217);
	volColor_ = QColor::fromRgb(226, 156, 80);
	effIDColor_ = QColor::fromRgb(42, 187, 155);
	effValColor_ = QColor::fromRgb(42, 187, 155);
	errorColor_ = QColor::fromRgb(255, 0, 0);
	headerTextColor_ = QColor::fromRgb(240, 240, 200);
	headerRowColor_ = QColor::fromRgb(60, 60, 60);
	maskColor_ = QColor::fromRgb(0, 0, 0, 128);
	borderColor_ = QColor::fromRgb(120, 120, 120);
	muteColor_ = QColor::fromRgb(255, 0, 0);
	unmuteColor_ = QColor::fromRgb(0, 255, 0);

	rightEffn_ = std::vector<int>(15);

	initDisplay();

	setAttribute(Qt::WA_Hover);
	setContextMenuPolicy(Qt::CustomContextMenu);
}

void PatternEditorPanel::initDisplay()
{
	pixmap_ = std::make_unique<QPixmap>(geometry().size());
}

void PatternEditorPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
}

void PatternEditorPanel::setCommandStack(std::weak_ptr<QUndoStack> stack)
{
	comStack_ = stack;
}

void PatternEditorPanel::drawPattern(const QRect &rect)
{
	int maxWidth = std::min(geometry().width(), TracksWidthFromLeftToEnd_);

	pixmap_->fill(Qt::black);
	drawRows(maxWidth);
	drawHeaders(maxWidth);
	drawBorders(maxWidth);
	if (!hasFocus()) drawShadow();

	QPainter painter(this);
	painter.drawPixmap(rect, *pixmap_.get());
}

void PatternEditorPanel::drawRows(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(stepFont_);

	int x, trackNum;

	/* Current row */
	// Fill row
	painter.fillRect(0, curRowY_, maxWidth, stepFontHeight_,
					 bt_->isJamMode() ? curRowColor_ : curRowColorEditable_);
	// Step number
	if (hovPos_.track == -2 && hovPos_.order == curPos_.order && hovPos_.step == curPos_.step)
		painter.fillRect(0, curRowY_, stepNumWidth_, stepFontHeight_, hovCellColor_);	// Paint hover
	painter.setPen((curPos_.step % hlCnt_) ? defStepNumColor_ : mkStepNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curPos_.step, 2, 16, QChar('0')).toUpper());
	// Step data
	for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
		x += drawStep(painter, trackNum, curPos_.order, curPos_.step, x, curRowBaselineY_, curRowY_);
		++trackNum;
	}

	int stepNum, odrNum;
	int rowY, baseY;
	int playOdrNum = bt_->getPlayingOrderNumber();
	int playStepNum = bt_->getPlayingStepNumber();

	/* Previous rows */
	for (rowY = curRowY_ - stepFontHeight_, baseY = curRowBaselineY_ - stepFontHeight_,
		 stepNum = curPos_.step - 1, odrNum = curPos_.order;
		 rowY >= headerHeight_ - stepFontHeight_;
		 rowY -= stepFontHeight_, baseY -= stepFontHeight_, --stepNum) {
		if (stepNum == -1) {
			if (odrNum == 0) {
				break;
			}
			else {
				--odrNum;
				stepNum = bt_->getPatternSizeFromOrderNumber(curSongNum_, odrNum) - 1;
			}
		}

		QColor rowColor, textColor;
		if (odrNum == playOdrNum && stepNum == playStepNum) {
			rowColor = playRowColor_;
			textColor = playTextColor_;
		}
		else {
			rowColor = (stepNum % hlCnt_) ? defRowColor_ : mkRowColor_;
			textColor = defTextColor_;
		}

		// Fill row
		painter.fillRect(0, rowY, maxWidth, stepFontHeight_, rowColor);
		// Step number
		if (hovPos_.track == -2 && hovPos_.order == odrNum && hovPos_.step == stepNum)
			painter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, hovCellColor_);	// Paint hover
		painter.setPen((stepNum % hlCnt_) ? defStepNumColor_ : mkStepNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(stepNum, 2, 16, QChar('0')).toUpper());
		// Step data
		painter.setPen(textColor);
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			x += drawStep(painter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (odrNum != curPos_.order)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, maskColor_);
	}

	int stepEnd = bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order);

	/* Next rows */
	for (rowY = curRowY_ + stepFontHeight_, baseY = curRowBaselineY_ + stepFontHeight_,
		 stepNum = curPos_.step + 1, odrNum = curPos_.order;
		 rowY <= geometry().height();
		 rowY += stepFontHeight_, baseY += stepFontHeight_, ++stepNum) {
		if (stepNum == stepEnd) {
			if (odrNum == bt_->getOrderSize(curSongNum_) - 1) {
				break;
			}
			else {
				++odrNum;
				stepNum = 0;
				stepEnd = bt_->getPatternSizeFromOrderNumber(curSongNum_, odrNum);
			}
		}

		QColor rowColor, textColor;
		if (odrNum == playOdrNum && stepNum == playStepNum) {
			rowColor = playRowColor_;
			textColor = playTextColor_;
		}
		else {
			rowColor = (stepNum % hlCnt_) ? defRowColor_ : mkRowColor_;
			textColor = defTextColor_;
		}

		// Fill row
		painter.fillRect(0, rowY, maxWidth, stepFontHeight_, rowColor);
		// Step number
		if (hovPos_.track == -2 && hovPos_.order == odrNum && hovPos_.step == stepNum)
			painter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, hovCellColor_);	// Paint hover
		painter.setPen((stepNum % hlCnt_) ? defStepNumColor_ : mkStepNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(stepNum, 2, 16, QChar('0')).toUpper());
		// Step data
		painter.setPen(textColor);
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			x += drawStep(painter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (odrNum != curPos_.order)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, maskColor_);
	}
}

int PatternEditorPanel::drawStep(QPainter &painter, int trackNum, int orderNum, int stepNum, int x, int baseY, int rowY)
{
	int offset = x + widthSpace_;
	PatternPosition pos{ trackNum, 0, orderNum, stepNum };
	QColor textColor = (stepNum == curPos_.step) ? curTextColor_ : defTextColor_;
	bool isHovTrack = (hovPos_.order == -2 && hovPos_.track == trackNum);
	bool isHovStep = (hovPos_.track == -2 && hovPos_.order == orderNum && hovPos_.step == stepNum);
	bool isMuteTrack = bt_->isMute(trackNum);
	SoundSource src = songStyle_.trackAttribs[trackNum].source;


	/* Tone name */
	if (pos == curPos_)	// Paint current cell
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 0, orderNum, stepNum))	// Paint selected
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, selCellColor_);
	int noteNum = bt_->getStepNoteNumber(curSongNum_, trackNum, orderNum, stepNum);
	switch (noteNum) {
	case -1:	// None
		painter.setPen(textColor);
		painter.drawText(offset, baseY, "---");
		break;
	case -2:	// Key off
		painter.fillRect(offset, rowY + stepFontHeight_ * 2 / 5,
						 toneNameWidth_, stepFontHeight_ / 5, toneColor_);
		break;
	case -3:	// Echo 0
		painter.setPen(toneColor_);
		painter.drawText(offset + widthSpace_, baseY, "^0");
		break;
	case -4:	// Echo 1
		painter.setPen(toneColor_);
		painter.drawText(offset + widthSpace_, baseY, "^1");
		break;
	case -5:	// Echo 2
		painter.setPen(toneColor_);
		painter.drawText(offset + widthSpace_, baseY, "^2");
		break;
	case -6:	// Echo 3
		painter.setPen(toneColor_);
		painter.drawText(offset + widthSpace_, baseY, "^3");
		break;
	default:	// Convert tone name
	{
		QString toneStr;
		switch (noteNum % 12) {
		case 0:		toneStr = "C-";	break;
		case 1:		toneStr = "C#";	break;
		case 2:		toneStr = "D-";	break;
		case 3:		toneStr = "D#";	break;
		case 4:		toneStr = "E-";	break;
		case 5:		toneStr = "F-";	break;
		case 6:		toneStr = "F#";	break;
		case 7:		toneStr = "G-";	break;
		case 8:		toneStr = "G#";	break;
		case 9:		toneStr = "A-";	break;
		case 10:	toneStr = "A#";	break;
		case 11:	toneStr = "B-";	break;
		}
		painter.setPen(toneColor_);
		painter.drawText(offset, baseY, toneStr + QString::number(noteNum / 12));
		break;
	}
	}
	if (isMuteTrack)	// Paint mute mask
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, maskColor_);
	offset += toneNameWidth_ +  stepFontWidth_;
	pos.colInTrack = 1;

	/* Instrument */
	if (pos == curPos_)	// Paint current cell
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 1, orderNum, stepNum))	// Paint selected
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, selCellColor_);
	int instNum = bt_->getStepInstrument(curSongNum_, trackNum, orderNum, stepNum);
	if (instNum == -1) {
		painter.setPen(textColor);
		painter.drawText(offset, baseY, "--");
	}
	else {
		std::unique_ptr<AbstractInstrument> inst = bt_->getInstrument(instNum);
		painter.setPen((inst != nullptr && src == inst->getSoundSource())
					   ? instColor_
					   : errorColor_);
		painter.drawText(offset, baseY, QString("%1").arg(instNum, 2, 16, QChar('0')).toUpper());
	}
	if (isMuteTrack)	// Paint mute mask
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, maskColor_);
	offset += instWidth_ +  stepFontWidth_;
	pos.colInTrack = 2;

	/* Volume */
	if (pos == curPos_)	// Paint current cell
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 2, orderNum, stepNum))	// Paint selected
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + stepFontWidth_, stepFontHeight_, selCellColor_);
	int vol = bt_->getStepVolume(curSongNum_, trackNum, orderNum, stepNum);
	if (vol == -1) {
		painter.setPen(textColor);
		painter.drawText(offset, baseY, "--");
	}
	else {
		QColor color;
		switch (src) {
		case SoundSource::FM:
			color = (vol < 0x80) ? volColor_ : errorColor_;
			break;
		case SoundSource::SSG:
			color = (vol < 0x10) ? volColor_ : errorColor_;
			break;
		case SoundSource::DRUM:
			color = (vol < 0x20) ? volColor_ : errorColor_;
			break;
		}
		painter.setPen(color);
		painter.drawText(offset, baseY, QString("%1").arg(vol, 2, 16, QChar('0')).toUpper());
	}
	if (isMuteTrack)	// Paint mute mask
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + stepFontWidth_, stepFontHeight_, maskColor_);
	offset += volWidth_ +  stepFontWidth_;
	pos.colInTrack = 3;

	/* Effect */
	for (int i = 0; i <= rightEffn_.at(trackNum); ++i) {
		/* Effect ID */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, curCellColor_);
		if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, hovCellColor_);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
				&& isSelectedCell(trackNum, pos.colInTrack, orderNum, stepNum))	// Paint selected
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, selCellColor_);
		QString effStr = QString::fromStdString(bt_->getStepEffectID(curSongNum_, trackNum, orderNum, stepNum, i));
		if (effStr == "--") {
			painter.setPen(textColor);
			painter.drawText(offset, baseY, effStr);
		}
		else {
			painter.setPen(effIDColor_);
			painter.drawText(offset, baseY, effStr);
		}
		if (isMuteTrack)	// Paint mute mask
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, maskColor_);
		offset += effIDWidth_;
		++pos.colInTrack;

		/* Effect Value */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, curCellColor_);
		if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, hovCellColor_);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
				&& isSelectedCell(trackNum, pos.colInTrack, orderNum, stepNum))	// Paint selected
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, selCellColor_);
		int effVal = bt_->getStepEffectValue(curSongNum_, trackNum, orderNum, stepNum, i);
		if (effVal == -1) {
			painter.setPen(textColor);
			painter.drawText(offset, baseY, "--");
		}
		else {
			painter.setPen(effValColor_);
			painter.drawText(offset, baseY, QString("%1").arg(effVal, 2, 16, QChar('0')).toUpper());
		}
		if (isMuteTrack)	// Paint mute mask
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, maskColor_);
		offset += effValWidth_ + stepFontWidth_;
		++pos.colInTrack;
	}

	return baseTrackWidth_ + effWidth_ * rightEffn_[trackNum];
}

void PatternEditorPanel::drawHeaders(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, headerRowColor_);
	int x, trackNum;
	for (x = stepNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
		int tw = baseTrackWidth_ + effWidth_ * rightEffn_.at(trackNum);
		if (hovPos_.order == -2 && hovPos_.track == trackNum)
			painter.fillRect(x - widthSpace_, 0, tw, headerHeight_, hovCellColor_);

		painter.setPen(headerTextColor_);
		QString srcName;
		switch (songStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
			srcName = "FM" + QString::number(songStyle_.trackAttribs[trackNum].channelInSource + 1);
			break;
		case SoundSource::SSG:
			srcName = "SSG" + QString::number(songStyle_.trackAttribs[trackNum].channelInSource + 1);
			break;
		case SoundSource::DRUM:
			switch (songStyle_.trackAttribs[trackNum].channelInSource) {
			case 0:	srcName = "Bass drum";	break;
			case 1:	srcName = "Snare drum";	break;
			case 2:	srcName = "Top cymbal";	break;
			case 3:	srcName = "Hi-hat";		break;
			case 4:	srcName = "Tom";		break;
			case 5:	srcName = "Rim shot";	break;
			}
			break;
		}
		painter.drawText(x, stepFontLeading_ + stepFontAscend_, srcName);

		painter.fillRect(x, headerHeight_ - 4, hdMuteToggleWidth_, 2,
						 bt_->isMute(trackNum) ? muteColor_ : unmuteColor_);

		painter.drawText(x + hdMuteToggleWidth_ + widthSpace_, hdPlusY_, "+");
		painter.drawText(x + hdMuteToggleWidth_ + widthSpace_, hdMinusY_, "-");

		x += tw;
		++trackNum;
	}
}

void PatternEditorPanel::drawBorders(int maxWidth)
{
	QPainter painter(pixmap_.get());

	painter.drawLine(0, headerHeight_, geometry().width(), headerHeight_);
	painter.drawLine(stepNumWidth_, 0, stepNumWidth_, geometry().height());
	for (int x = stepNumWidth_, trackNum = leftTrackNum_; trackNum < rightEffn_.size(); ) {
		x += (baseTrackWidth_ + effWidth_ * rightEffn_.at(trackNum));
		if (x > maxWidth) break;
		painter.drawLine(x, 0, x, geometry().height());
		++trackNum;
	}
}

void PatternEditorPanel::drawShadow()
{
	QPainter painter(pixmap_.get());
	painter.fillRect(0, 0, geometry().width(), geometry().height(), QColor::fromRgb(0, 0, 0, 47));
}

int PatternEditorPanel::calculateTracksWidthWithRowNum(int begin, int end) const
{
	int width = stepNumWidth_;
	for (int i = begin; i <= end; ++i) {
		width += (baseTrackWidth_ + effWidth_ * rightEffn_.at(i));
	}
	return width;
}

int PatternEditorPanel::calculateColNumInRow(int trackNum, int colNumInTrack, bool isExpanded) const
{
	if (isExpanded) {
		return trackNum * 11 + colNumInTrack;
	}
	else {
		int ret = 0;
		for (int i = 0; i < trackNum; ++i) {
			ret += (5 + 2 * rightEffn_.at(i));
		}
		return ret + colNumInTrack;
	}
}

void PatternEditorPanel::moveCursorToRight(int n)
{
	int oldTrackNum = curPos_.track;

	curPos_.colInTrack += n;
	if (n > 0) {
		while (true) {
			int lim = 5 + 2 * rightEffn_.at(curPos_.track);
			if (curPos_.colInTrack < lim) {
				break;
			}
			else {
				if (curPos_.track == songStyle_.trackAttribs.size() - 1) {
					curPos_.track = 0;
				}
				else {
					++curPos_.track;
				}
				curPos_.colInTrack -= lim;
			}
		}
	}
	else {
		while (true) {
			if (curPos_.colInTrack >= 0) {
				break;
			}
			else {
				if (!curPos_.track) {
					curPos_.track = songStyle_.trackAttribs.size() - 1;
				}
				else {
					--curPos_.track;
				}
				curPos_.colInTrack += (5 + 2 * rightEffn_.at(curPos_.track));
			}
		}
	}
	if (oldTrackNum < curPos_.track) {
		while (calculateTracksWidthWithRowNum(leftTrackNum_, curPos_.track) > geometry().width())
			++leftTrackNum_;
	}
	else {
		if (curPos_.track < leftTrackNum_) leftTrackNum_ = curPos_.track;
	}

	TracksWidthFromLeftToEnd_
			= calculateTracksWidthWithRowNum(leftTrackNum_, songStyle_.trackAttribs.size() - 1);

	if (curPos_.track != oldTrackNum)
		bt_->setCurrentTrack(curPos_.track);

	if (!isIgnoreToSlider_)
		emit currentCellInRowChanged(calculateColNumInRow(curPos_.track, curPos_.colInTrack));

	if (!isIgnoreToOrder_ && curPos_.track != oldTrackNum)	// Send to order list
		emit currentTrackChanged(curPos_.track);

	update();
}

void PatternEditorPanel::moveCursorToDown(int n)
{
	int oldOdr = curPos_.order;
	int tmp = curPos_.step + n;

	if (n > 0) {
		while (true) {
			int dif = tmp - bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order);
			if (dif < 0) {
				curPos_.step = tmp;
				break;
			}
			else {
				if (curPos_.order == bt_->getOrderSize(curSongNum_) - 1) {
					curPos_.order = 0;
				}
				else {
					++curPos_.order;
				}
				tmp = dif;
			}
		}
	}
	else {
		while (true) {
			if (tmp < 0) {
				if (curPos_.order == 0) {
					curPos_.order = bt_->getOrderSize(curSongNum_) - 1;
				}
				else {
					--curPos_.order;
				}
				tmp += bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order);
			}
			else {
				curPos_.step = tmp;
				break;
			}
		}
	}

	if (curPos_.order != oldOdr)
		bt_->setCurrentOrderNumber(curPos_.order);
	bt_->setCurrentStepNumber(curPos_.step);

	if (!isIgnoreToSlider_)
		emit currentStepChanged(curPos_.step, bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - 1);

	if (!isIgnoreToOrder_ && curPos_.order != oldOdr)	// Send to order list
		emit currentOrderChanged(curPos_.order, bt_->getOrderSize(curSongNum_) - 1);

	update();
}

void PatternEditorPanel::expandEffect(int trackNum)
{
	if (rightEffn_.at(trackNum) == 3) return;
	++rightEffn_[trackNum];
	TracksWidthFromLeftToEnd_
			= calculateTracksWidthWithRowNum(leftTrackNum_, songStyle_.trackAttribs.size() - 1);

	emit effectColsCompanded(calculateColNumInRow(curPos_.track, curPos_.colInTrack), getFullColmunSize());
}

void PatternEditorPanel::shrinkEffect(int trackNum)
{
	if (rightEffn_.at(trackNum) == 0) return;
	--rightEffn_[trackNum];
	TracksWidthFromLeftToEnd_
			= calculateTracksWidthWithRowNum(leftTrackNum_, songStyle_.trackAttribs.size() - 1);

	emit effectColsCompanded(calculateColNumInRow(curPos_.track, curPos_.colInTrack), getFullColmunSize());
}

int PatternEditorPanel::calculateColumnDistance(int beginTrack, int beginColumn, int endTrack, int endColumn, bool isExpanded) const
{
	return (calculateColNumInRow(endTrack, endColumn, isExpanded)
			- calculateColNumInRow(beginTrack, beginColumn, isExpanded));
}

int PatternEditorPanel::calculateStepDistance(int beginOrder, int beginStep, int endOrder, int endStep) const
{
	int d = 0;
	int startOrder, startStep, stopOrder, stopStep;
	bool flag;

	if (endOrder >= beginOrder) {
		startOrder = endOrder;
		startStep = endStep;
		stopOrder = beginOrder;
		stopStep = beginStep;
		flag = true;
	}
	else {
		startOrder = beginOrder;
		startStep = beginStep;
		stopOrder = endOrder;
		stopStep = endStep;
		flag = false;
	}

	while (true) {
		if (startOrder == stopOrder) {
			d += (startStep - stopStep);
			break;
		}
		else {
			d += startStep;
			startStep = bt_->getPatternSizeFromOrderNumber(curSongNum_, --startOrder);
		}
	}

	return flag ? d : -d;
}

void PatternEditorPanel::changeEditable()
{
	update();
}

int PatternEditorPanel::getFullColmunSize() const
{
	return calculateColNumInRow(songStyle_.trackAttribs.size() - 1, 4 + 2 * rightEffn_.back());
}

void PatternEditorPanel::updatePosition()
{
	curPos_.setRows(bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());

	emit currentOrderChanged(curPos_.order, bt_->getOrderSize(curSongNum_) - 1);
	emit currentStepChanged(curPos_.step, bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - 1);

	update();
}

bool PatternEditorPanel::enterToneData(int key)
{
	int baseOct = bt_->getCurrentOctave();

	switch (key) {
	case Qt::Key_Z:			setStepKeyOn(Note::C, baseOct);			return false;
	case Qt::Key_S:			setStepKeyOn(Note::CS, baseOct);		return false;
	case Qt::Key_X:			setStepKeyOn(Note::D, baseOct);			return false;
	case Qt::Key_D:			setStepKeyOn(Note::DS, baseOct);		return false;
	case Qt::Key_C:			setStepKeyOn(Note::E, baseOct);			return false;
	case Qt::Key_V:			setStepKeyOn(Note::F, baseOct);			return false;
	case Qt::Key_G:			setStepKeyOn(Note::FS, baseOct);		return false;
	case Qt::Key_B:			setStepKeyOn(Note::G, baseOct);			return false;
	case Qt::Key_H:			setStepKeyOn(Note::GS, baseOct);		return false;
	case Qt::Key_N:			setStepKeyOn(Note::A, baseOct);			return false;
	case Qt::Key_J:			setStepKeyOn(Note::AS, baseOct);		return false;
	case Qt::Key_M:			setStepKeyOn(Note::B, baseOct);			return false;
	case Qt::Key_Comma:		setStepKeyOn(Note::C, baseOct + 1);		return false;
	case Qt::Key_L:			setStepKeyOn(Note::CS, baseOct + 1);	return false;
	case Qt::Key_Period:	setStepKeyOn(Note::D, baseOct + 1);		return false;
	case Qt::Key_Q:			setStepKeyOn(Note::C, baseOct + 1);		return false;
	case Qt::Key_2:			setStepKeyOn(Note::CS, baseOct + 1);	return false;
	case Qt::Key_W:			setStepKeyOn(Note::D, baseOct + 1);		return false;
	case Qt::Key_3:			setStepKeyOn(Note::DS, baseOct + 1);	return false;
	case Qt::Key_E:			setStepKeyOn(Note::E, baseOct + 1);		return false;
	case Qt::Key_R:			setStepKeyOn(Note::F, baseOct + 1);		return false;
	case Qt::Key_5:			setStepKeyOn(Note::FS, baseOct + 1);	return false;
	case Qt::Key_T:			setStepKeyOn(Note::G, baseOct + 1);		return false;
	case Qt::Key_6:			setStepKeyOn(Note::GS, baseOct + 1);	return false;
	case Qt::Key_Y:			setStepKeyOn(Note::A, baseOct + 1);		return false;
	case Qt::Key_7:			setStepKeyOn(Note::AS, baseOct + 1);	return false;
	case Qt::Key_U:			setStepKeyOn(Note::B, baseOct + 1);		return false;
	case Qt::Key_I:			setStepKeyOn(Note::C, baseOct + 2);		return false;
	case Qt::Key_9:			setStepKeyOn(Note::CS, baseOct + 2);	return false;
	case Qt::Key_O:			setStepKeyOn(Note::D, baseOct + 2);		return false;
	case Qt::Key_Minus:
		bt_->setStepKeyOff(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
		comStack_.lock()->push(new SetKeyOffToStepQtCommand(this));
		moveCursorToDown(1);
		return true;
	case Qt::Key_AsciiCircum:
	{
		int n = bt_->getCurrentOctave();
		if (n > 3) n = 3;
		bt_->setEchoBufferAccess(curSongNum_, curPos_.track, curPos_.order, curPos_.step, n);
		comStack_.lock()->push(new SetEchoBufferAccessQtCommand(this));
		moveCursorToDown(1);
		return true;
	}
	default:
		return false;
	}
}

void PatternEditorPanel::setStepKeyOn(Note note, int octave)
{
	if (octave < 8) {
		bt_->setStepNote(curSongNum_, curPos_.track, curPos_.order, curPos_.step, octave, note);
		comStack_.lock()->push(new SetKeyOnToStepQtCommand(this));
		moveCursorToDown(1);
	}
}

bool PatternEditorPanel::enterInstrumentData(int key)
{
	switch (key) {
	case Qt::Key_0:	setStepInstrument(0x0);	return true;
	case Qt::Key_1:	setStepInstrument(0x1);	return true;
	case Qt::Key_2:	setStepInstrument(0x2);	return true;
	case Qt::Key_3:	setStepInstrument(0x3);	return true;
	case Qt::Key_4:	setStepInstrument(0x4);	return true;
	case Qt::Key_5:	setStepInstrument(0x5);	return true;
	case Qt::Key_6:	setStepInstrument(0x6);	return true;
	case Qt::Key_7:	setStepInstrument(0x7);	return true;
	case Qt::Key_8:	setStepInstrument(0x8);	return true;
	case Qt::Key_9:	setStepInstrument(0x9);	return true;
	case Qt::Key_A:	setStepInstrument(0xa);	return true;
	case Qt::Key_B:	setStepInstrument(0xb);	return true;
	case Qt::Key_C:	setStepInstrument(0xc);	return true;
	case Qt::Key_D:	setStepInstrument(0xd);	return true;
	case Qt::Key_E:	setStepInstrument(0xe);	return true;
	case Qt::Key_F:	setStepInstrument(0xf);	return true;
	default:	return false;
	}
}

void PatternEditorPanel::setStepInstrument(int num)
{
	entryCnt_ = (entryCnt_ == 1 && curPos_ == editPos_) ? 0 : 1;
	editPos_ = curPos_;
	bt_->setStepInstrument(curSongNum_, editPos_.track, editPos_.order, editPos_.step, num);
	comStack_.lock()->push(new SetInstrumentToStepQtCommand(this, editPos_));

	if (!entryCnt_) moveCursorToDown(1);
}

bool PatternEditorPanel::enterVolumeData(int key)
{
	switch (key) {
	case Qt::Key_0:	setStepVolume(0x0);	return true;
	case Qt::Key_1:	setStepVolume(0x1);	return true;
	case Qt::Key_2:	setStepVolume(0x2);	return true;
	case Qt::Key_3:	setStepVolume(0x3);	return true;
	case Qt::Key_4:	setStepVolume(0x4);	return true;
	case Qt::Key_5:	setStepVolume(0x5);	return true;
	case Qt::Key_6:	setStepVolume(0x6);	return true;
	case Qt::Key_7:	setStepVolume(0x7);	return true;
	case Qt::Key_8:	setStepVolume(0x8);	return true;
	case Qt::Key_9:	setStepVolume(0x9);	return true;
	case Qt::Key_A:	setStepVolume(0xa);	return true;
	case Qt::Key_B:	setStepVolume(0xb);	return true;
	case Qt::Key_C:	setStepVolume(0xc);	return true;
	case Qt::Key_D:	setStepVolume(0xd);	return true;
	case Qt::Key_E:	setStepVolume(0xe);	return true;
	case Qt::Key_F:	setStepVolume(0xf);	return true;
	default:	return false;
	}
}

void PatternEditorPanel::setStepVolume(int volume)
{
	entryCnt_ = (entryCnt_ == 1 && curPos_ == editPos_) ? 0 : 1;
	editPos_ = curPos_;
	bt_->setStepVolume(curSongNum_, editPos_.track, editPos_.order, editPos_.step, volume);
	comStack_.lock()->push(new SetVolumeToStepQtCommand(this, editPos_));

	if (!entryCnt_) moveCursorToDown(1);
}

bool PatternEditorPanel::enterEffectID(int key)
{
	switch (key) {
	case Qt::Key_0:			setStepEffectID("0");	return true;
	case Qt::Key_1:			setStepEffectID("1");	return true;
	case Qt::Key_2:			setStepEffectID("2");	return true;
	case Qt::Key_3:			setStepEffectID("3");	return true;
	case Qt::Key_4:			setStepEffectID("4");	return true;
	case Qt::Key_5:			setStepEffectID("5");	return true;
	case Qt::Key_6:			setStepEffectID("6");	return true;
	case Qt::Key_7:			setStepEffectID("7");	return true;
	case Qt::Key_8:			setStepEffectID("8");	return true;
	case Qt::Key_9:			setStepEffectID("9");	return true;
	case Qt::Key_A:			setStepEffectID("A");	return true;
	case Qt::Key_B:			setStepEffectID("B");	return true;
	case Qt::Key_C:			setStepEffectID("C");	return true;
	case Qt::Key_D:			setStepEffectID("D");	return true;
	case Qt::Key_E:			setStepEffectID("E");	return true;
	case Qt::Key_F:			setStepEffectID("F");	return true;
	case Qt::Key_G:			setStepEffectID("G");	return true;
	case Qt::Key_H:			setStepEffectID("H");	return true;
	case Qt::Key_I:			setStepEffectID("I");	return true;
	case Qt::Key_J:			setStepEffectID("J");	return true;
	case Qt::Key_K:			setStepEffectID("K");	return true;
	case Qt::Key_L:			setStepEffectID("L");	return true;
	case Qt::Key_M:			setStepEffectID("M");	return true;
	case Qt::Key_N:			setStepEffectID("N");	return true;
	case Qt::Key_O:			setStepEffectID("O");	return true;
	case Qt::Key_P:			setStepEffectID("P");	return true;
	case Qt::Key_Q:			setStepEffectID("Q");	return true;
	case Qt::Key_R:			setStepEffectID("R");	return true;
	case Qt::Key_S:			setStepEffectID("S");	return true;
	case Qt::Key_T:			setStepEffectID("T");	return true;
	case Qt::Key_U:			setStepEffectID("U");	return true;
	case Qt::Key_V:			setStepEffectID("V");	return true;
	case Qt::Key_W:			setStepEffectID("W");	return true;
	case Qt::Key_X:			setStepEffectID("X");	return true;
	case Qt::Key_Y:			setStepEffectID("Y");	return true;
	case Qt::Key_Z:			setStepEffectID("Z");	return true;
	default:										return false;
	}
}

void PatternEditorPanel::setStepEffectID(QString str)
{
	entryCnt_ = (entryCnt_ == 1 && curPos_ == editPos_) ? 0 : 1;
	editPos_ = curPos_;
	bt_->setStepEffectID(curSongNum_, editPos_.track, editPos_.order, editPos_.step,
						 (editPos_.colInTrack - 3) / 2, str.toStdString());
	comStack_.lock()->push(new SetEffectIDToStepQtCommand(this, editPos_));

	if (!entryCnt_) moveCursorToDown(1);
}

bool PatternEditorPanel::enterEffectValue(int key)
{
	switch (key) {
	case Qt::Key_0:	setStepEffectValue(0x0);	return true;
	case Qt::Key_1:	setStepEffectValue(0x1);	return true;
	case Qt::Key_2:	setStepEffectValue(0x2);	return true;
	case Qt::Key_3:	setStepEffectValue(0x3);	return true;
	case Qt::Key_4:	setStepEffectValue(0x4);	return true;
	case Qt::Key_5:	setStepEffectValue(0x5);	return true;
	case Qt::Key_6:	setStepEffectValue(0x6);	return true;
	case Qt::Key_7:	setStepEffectValue(0x7);	return true;
	case Qt::Key_8:	setStepEffectValue(0x8);	return true;
	case Qt::Key_9:	setStepEffectValue(0x9);	return true;
	case Qt::Key_A:	setStepEffectValue(0xa);	return true;
	case Qt::Key_B:	setStepEffectValue(0xb);	return true;
	case Qt::Key_C:	setStepEffectValue(0xc);	return true;
	case Qt::Key_D:	setStepEffectValue(0xd);	return true;
	case Qt::Key_E:	setStepEffectValue(0xe);	return true;
	case Qt::Key_F:	setStepEffectValue(0xf);	return true;
	default:	return false;
	}
}

void PatternEditorPanel::setStepEffectValue(int value)
{
	entryCnt_ = (entryCnt_ == 1 && curPos_ == editPos_) ? 0 : 1;
	editPos_ = curPos_;
	bt_->setStepEffectValue(curSongNum_, editPos_.track, editPos_.order, editPos_.step,
							(editPos_.colInTrack - 4) / 2, value);
	comStack_.lock()->push(new SetEffectValueToStepQtCommand(this, editPos_));

	if (!entryCnt_) moveCursorToDown(1);
}

void PatternEditorPanel::insertStep()
{
	bt_->insertStep(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
	comStack_.lock()->push(new InsertStepQtCommand(this));
	moveCursorToDown(1);
}

void PatternEditorPanel::deletePreviousStep()
{
	if (curPos_.step) {
		bt_->deletePreviousStep(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
		comStack_.lock()->push(new DeletePreviousStepQtCommand(this));
		moveCursorToDown(-1);
	}
}

void PatternEditorPanel::copySelectedCells()
{
	if (selLeftAbovePos_.order == -1) return;

	int w = 1 + calculateColumnDistance(selLeftAbovePos_.track, selLeftAbovePos_.colInTrack,
										selRightBelowPos_.track, selRightBelowPos_.colInTrack, true);
	int h = 1 + calculateStepDistance(selLeftAbovePos_.order, selLeftAbovePos_.step,
									  selRightBelowPos_.order, selRightBelowPos_.step);
	QString str = QString("PATTERN_COPY:%1,%2,%3,")
				  .arg(QString::number(selLeftAbovePos_.colInTrack),
					   QString::number(w),
					   QString::number(h));
	PatternPosition pos = selLeftAbovePos_;
	while (true) {
		switch (pos.colInTrack) {
		case 0:
			str += QString::number(bt_->getStepNoteNumber(curSongNum_, pos.track, pos.order, pos.step));
			break;
		case 1:
			str += QString::number(bt_->getStepInstrument(curSongNum_, pos.track, pos.order, pos.step));
			break;
		case 2:
			str += QString::number(bt_->getStepVolume(curSongNum_, pos.track, pos.order, pos.step));
			break;
		case 3:
			str += QString::fromStdString(bt_->getStepEffectID(curSongNum_, pos.track, pos.order, pos.step, 0));
			break;
		case 4:
			str += QString::number(bt_->getStepEffectValue(curSongNum_, pos.track, pos.order, pos.step, 0));
			break;
		case 5:
			str += QString::fromStdString(bt_->getStepEffectID(curSongNum_, pos.track, pos.order, pos.step, 1));
			break;
		case 6:
			str += QString::number(bt_->getStepEffectValue(curSongNum_, pos.track, pos.order, pos.step, 1));
			break;
		case 7:
			str += QString::fromStdString(bt_->getStepEffectID(curSongNum_, pos.track, pos.order, pos.step, 2));
			break;
		case 8:
			str += QString::number(bt_->getStepEffectValue(curSongNum_, pos.track, pos.order, pos.step, 2));
			break;
		case 9:
			str += QString::fromStdString(bt_->getStepEffectID(curSongNum_, pos.track, pos.order, pos.step, 3));
			break;
		case 10:
			str += QString::number(bt_->getStepEffectValue(curSongNum_, pos.track, pos.order, pos.step, 3));
			break;
		}

		if (pos.track == selRightBelowPos_.track && pos.colInTrack == selRightBelowPos_.colInTrack) {
			if (pos.order == selRightBelowPos_.order && pos.step == selRightBelowPos_.step) {
				break;
			}
			else {
				pos.setCols(selLeftAbovePos_.track, selLeftAbovePos_.colInTrack);
				++pos.step;
			}
		}
		else {
			++pos.colInTrack;
			pos.track += (pos.colInTrack / 11);
			pos.colInTrack %= 11;
		}

		str += ",";
	}

	QApplication::clipboard()->setText(str);
}

void PatternEditorPanel::eraseSelectedCells()
{
	bt_->erasePatternCells(curSongNum_, selLeftAbovePos_.track, selLeftAbovePos_.colInTrack,
						   selLeftAbovePos_.order, selLeftAbovePos_.step,
						   selRightBelowPos_.track, selRightBelowPos_.colInTrack, selRightBelowPos_.step);
	comStack_.lock()->push(new EraseCellsInPatternQtCommand(this));
}

void PatternEditorPanel::pasteCopiedCells(PatternPosition& startPos)
{
	int sCol = 0;
	std::vector<std::vector<std::string>> cells
			= instantiateCellsFromString(QApplication::clipboard()->text(), sCol);

	if (sCol > 2 && !((curPos_.colInTrack - sCol) % 2) && cells.front().size() <= 11 - curPos_.colInTrack)
		sCol = curPos_.colInTrack;

	bt_->pastePatternCells(curSongNum_, startPos.track, sCol,
						   startPos.order, startPos.step, std::move(cells));
	comStack_.lock()->push(new PasteCopiedDataToPatternQtCommand(this));
}

void PatternEditorPanel::pasteMixCopiedCells(PatternPosition& startPos)
{
	int sCol = 0;
	std::vector<std::vector<std::string>> cells
			= instantiateCellsFromString(QApplication::clipboard()->text(), sCol);

	if (sCol > 2 && !((curPos_.colInTrack - sCol) % 2) && cells.front().size() <= 11 - curPos_.colInTrack)
		sCol = curPos_.colInTrack;

	bt_->pasteMixPatternCells(curSongNum_, startPos.track, sCol,
							  startPos.order, startPos.step, std::move(cells));
	comStack_.lock()->push(new PasteMixCopiedDataToPatternQtCommand(this));
}

std::vector<std::vector<std::string>> PatternEditorPanel::instantiateCellsFromString(QString str, int& startCol)
{
	str.remove(QRegularExpression("PATTERN_(COPY|CUT):"));
	QString hdRe = "^([0-9]+),([0-9]+),([0-9]+),";
	QRegularExpression re(hdRe);
	QRegularExpressionMatch match = re.match(str);
	startCol = match.captured(1).toInt();
	int w = match.captured(2).toInt();
	int h = match.captured(3).toInt();
	str.remove(re);

	std::vector<std::vector<std::string>> cells;
	re = QRegularExpression("^([^,]+),");
	for (int i = 0; i < h; ++i) {
		cells.emplace_back();
		for (int j = 0; j < w; ++j) {
			match = re.match(str);
			if (match.hasMatch()) {
				cells.at(i).push_back(match.captured(1).toStdString());
				str.remove(re);
			}
			else {
				cells.at(i).push_back(str.toStdString());
				break;
			}
		}
	}

	return cells;
}

void PatternEditorPanel::cutSelectedCells()
{
	if (selLeftAbovePos_.order == -1) return;

	copySelectedCells();
	eraseSelectedCells();
	QString str = QApplication::clipboard()->text();
	str.replace("COPY", "CUT");
	QApplication::clipboard()->setText(str);
}

void PatternEditorPanel::increaseNoteKey(PatternPosition& startPos, PatternPosition& endPos)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->increaseNoteKeyInPattern(curSongNum_,
									  beginTrack, startPos.order, startPos.step,
									  endPos.track, endPos.step);
		comStack_.lock()->push(new IncreaseNoteKeyInPatternQtCommand(this));
	}
}

void PatternEditorPanel::decreaseNoteKey(PatternPosition& startPos, PatternPosition& endPos)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->decreaseNoteKeyInPattern(curSongNum_,
									  beginTrack, startPos.order, startPos.step,
									  endPos.track, endPos.step);
		comStack_.lock()->push(new DecreaseNoteKeyInPatternQtCommand(this));
	}
}

void PatternEditorPanel::increaseNoteOctave(PatternPosition& startPos, PatternPosition& endPos)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->increaseNoteOctaveInPattern(curSongNum_,
										 beginTrack, startPos.order, startPos.step,
										 endPos.track, endPos.step);
		comStack_.lock()->push(new IncreaseNoteOctaveInPatternQtCommand(this));
	}
}

void PatternEditorPanel::decreaseNoteOctave(PatternPosition& startPos, PatternPosition& endPos)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->decreaseNoteOctaveInPattern(curSongNum_,
										 beginTrack, startPos.order, startPos.step,
										 endPos.track, endPos.step);
		comStack_.lock()->push(new DecreaseNoteOctaveInPatternQtCommand(this));
	}
}

void PatternEditorPanel::setSelectedRectangle(const PatternPosition& start, const PatternPosition& end)
{
	if (start.compareCols(end) > 0) {
		if (start.compareRows(end) > 0) {
			selLeftAbovePos_ = end;
			selRightBelowPos_ = start;
		}
		else {
			selLeftAbovePos_ = { end.track, end.colInTrack, start.order, start.step };
			selRightBelowPos_ = { start.track, start.colInTrack, end.order, end.step };
		}
	}
	else {
		if (start.compareRows(end) > 0) {
			selLeftAbovePos_ = { start.track, start.colInTrack, end.order, end.step };
			selRightBelowPos_ = { end.track, end.colInTrack, start.order, start.step };
		}
		else {
			selLeftAbovePos_ = start;
			selRightBelowPos_ = end;
		}
	}

	emit selected(true);
}

bool PatternEditorPanel::isSelectedCell(int trackNum, int colNum, int orderNum, int stepNum)
{
	PatternPosition pos{ trackNum, colNum, orderNum, stepNum };
	return (selLeftAbovePos_.compareCols(pos) <= 0 && selRightBelowPos_.compareCols(pos) >= 0
			&& selLeftAbovePos_.compareRows(pos) <= 0 && selRightBelowPos_.compareRows(pos) >= 0);
}

/********** Slots **********/
void PatternEditorPanel::setCurrentCellInRow(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - calculateColNumInRow(curPos_.track, curPos_.colInTrack))
		moveCursorToRight(dif);
}

void PatternEditorPanel::setCurrentStep(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - curPos_.step) moveCursorToDown(dif);
}

void PatternEditorPanel::setCurrentTrack(int num)
{
	Ui::EventGuard eg(isIgnoreToOrder_);

	int dif = calculateColumnDistance(curPos_.track, curPos_.colInTrack, num, 0);
	moveCursorToRight(dif);
}

void PatternEditorPanel::setCurrentOrder(int num)
{
	Ui::EventGuard eg(isIgnoreToOrder_);

	int dif = calculateStepDistance(curPos_.order, curPos_.step, num, 0);
	moveCursorToDown(dif);
}

void PatternEditorPanel::onOrderListEdited()
{
	// Reset position memory
	hovPos_ = { -1, -1, -1, -1 };
	editPos_ = { -1, -1, -1, -1 };
	mousePressPos_ = { -1, -1, -1, -1 };
	mouseReleasePos_ = { -1, -1, -1, -1 };
	selLeftAbovePos_ = { -1, -1, -1, -1 };
	selRightBelowPos_ = { -1, -1, -1, -1 };
	shiftPressedPos_ = { -1, -1, -1, -1 };
	selectAllState_ = -1;
	emit selected(false);

	update();
}

void PatternEditorPanel::onDefaultPatternSizeChanged()
{
	// Check pattern size
	int end = bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order);
	if (curPos_.step >= end) curPos_.step = end - 1;

	update();
}

void PatternEditorPanel::setPatternHighlightCount(int count)
{
	hlCnt_ = count;
	update();
}

void PatternEditorPanel::onSongLoaded()
{
	curSongNum_ = bt_->getCurrentSongNumber();
	curPos_ = {
		bt_->getCurrentTrackAttribute().number,
		0,
		bt_->getCurrentOrderNumber(),
		bt_->getCurrentStepNumber()
	};
	songStyle_ = bt_->getSongStyle(curSongNum_);
	TracksWidthFromLeftToEnd_ = calculateTracksWidthWithRowNum(0, songStyle_.trackAttribs.size() - 1);

	hovPos_ = { -1, -1, -1, -1 };
	editPos_ = { -1, -1, -1, -1 };
	mousePressPos_ = { -1, -1, -1, -1 };
	mouseReleasePos_ = { -1, -1, -1, -1 };
	selLeftAbovePos_ = { -1, -1, -1, -1 };
	selRightBelowPos_ = { -1, -1, -1, -1 };
	shiftPressedPos_ = { -1, -1, -1, -1 };
	entryCnt_ = 0;
	selectAllState_ = -1;
	emit selected(false);

	update();
}

void PatternEditorPanel::onDeletePressed()
{
	if (bt_->isJamMode()) return;

	if (selLeftAbovePos_.order != -1) {	// Delete region
		eraseSelectedCells();
	}
	else {
		switch (curPos_.colInTrack) {
		case 0:
			bt_->eraseStepNote(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new EraseStepQtCommand(this));
			moveCursorToDown(1);
			break;
		case 1:
			bt_->eraseStepInstrument(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new EraseInstrumentInStepQtCommand(this));
			moveCursorToDown(1);
			break;
		case 2:
			bt_->eraseStepVolume(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new EraseVolumeInStepQtCommand(this));
			moveCursorToDown(1);
			break;
		case 3:
		case 5:
		case 7:
		case 9:
			bt_->eraseStepEffect(curSongNum_, curPos_.track, curPos_.order, curPos_.step,
								 (curPos_.colInTrack - 3) / 2);
			comStack_.lock()->push(new EraseEffectInStepQtCommand(this));
			moveCursorToDown(1);
			break;
		case 4:
		case 6:
		case 8:
		case 10:
			bt_->eraseStepEffectValue(curSongNum_, curPos_.track, curPos_.order, curPos_.step,
									  (curPos_.colInTrack - 4) / 2);
			comStack_.lock()->push(new EraseEffectValueInStepQtCommand(this));
			moveCursorToDown(1);
			break;
		}
	}
}

void PatternEditorPanel::onPastePressed()
{
	if (!bt_->isPlaySong()) pasteCopiedCells(curPos_);
}

void PatternEditorPanel::onPasteMixPressed()
{
	if (!bt_->isPlaySong()) pasteMixCopiedCells(curPos_);
}

void PatternEditorPanel::onSelectPressed(int type)
{
	switch (type) {
	case 0:	// None
	{
		selLeftAbovePos_ = { -1, -1, -1, -1 };
		selRightBelowPos_ = { -1, -1, -1, -1 };
		selectAllState_ = -1;
		emit selected(false);
		break;
	}
	case 1:	// All
	{
		selectAllState_ = (selectAllState_ + 1) % 2;
		int max = bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - 1;
		if (!selectAllState_) {
			PatternPosition start = { curPos_.track, 0, curPos_.order, 0 };
			PatternPosition end = { curPos_.track, 10, curPos_.order, max };
			setSelectedRectangle(start, end);
		}
		else {
			PatternPosition start = { 0, 0, curPos_.order, 0 };
			PatternPosition end = { static_cast<int>(songStyle_.trackAttribs.size() - 1), 10,
									curPos_.order, max };
			setSelectedRectangle(start, end);
		}
		break;
	}
	}

	update();
}

void PatternEditorPanel::onTransposePressed(bool isOctave, bool isIncreased)
{
	if (bt_->isJamMode()) return;

	if (isOctave) {
		if (isIncreased) {
			if (selLeftAbovePos_.order != -1)
				increaseNoteOctave(selLeftAbovePos_, selRightBelowPos_);
			else
				increaseNoteOctave(curPos_, curPos_);
		}
		else {
			if (selLeftAbovePos_.order != -1)
				decreaseNoteOctave(selLeftAbovePos_, selRightBelowPos_);
			else
				decreaseNoteOctave(curPos_, curPos_);
		}
	}
	else {
		if (isIncreased) {
			if (selLeftAbovePos_.order != -1)
				increaseNoteKey(selLeftAbovePos_, selRightBelowPos_);
			else
				increaseNoteKey(curPos_, curPos_);
		}
		else {
			if (selLeftAbovePos_.order != -1)
				decreaseNoteKey(selLeftAbovePos_, selRightBelowPos_);
			else
				decreaseNoteKey(curPos_, curPos_);
		}
	}
}

void PatternEditorPanel::onMuteTrackPressed()
{
	bt_->setTrackMuteState(curPos_.track, !bt_->isMute(curPos_.track));
	isMuteElse_ = false;
	update();
}

void PatternEditorPanel::onSoloTrackPressed()
{
	isMuteElse_ = !isMuteElse_;
	for (int track = 0; track < songStyle_.trackAttribs.size(); ++track)
		bt_->setTrackMuteState(track, (track == curPos_.track) ? false : isMuteElse_);
	update();
}

void PatternEditorPanel::onExpandPressed()
{
	if (selLeftAbovePos_.order == -1) return;

	bt_->expandPattern(curSongNum_, selLeftAbovePos_.track, selLeftAbovePos_.colInTrack,
					   selLeftAbovePos_.order, selLeftAbovePos_.step,
					   selRightBelowPos_.track, selRightBelowPos_.colInTrack, selRightBelowPos_.step);
	comStack_.lock()->push(new ExpandPatternQtCommand(this));
}

void PatternEditorPanel::onShrinkPressed()
{
	if (selLeftAbovePos_.order == -1) return;

	bt_->shrinkPattern(curSongNum_, selLeftAbovePos_.track, selLeftAbovePos_.colInTrack,
					   selLeftAbovePos_.order, selLeftAbovePos_.step,
					   selRightBelowPos_.track, selRightBelowPos_.colInTrack, selRightBelowPos_.step);
	comStack_.lock()->push(new ShrinkPatternQtCommand(this));
}

void PatternEditorPanel::onInterpolatePressed()
{
	if (selLeftAbovePos_.order == -1) return;

	bt_->interpolatePattern(curSongNum_, selLeftAbovePos_.track, selLeftAbovePos_.colInTrack,
							selLeftAbovePos_.order, selLeftAbovePos_.step,
							selRightBelowPos_.track, selRightBelowPos_.colInTrack, selRightBelowPos_.step);
	comStack_.lock()->push(new InterpolatePatternQtCommand(this));
}

void PatternEditorPanel::onReversePressed()
{
	if (selLeftAbovePos_.order == -1) return;

	bt_->reversePattern(curSongNum_, selLeftAbovePos_.track, selLeftAbovePos_.colInTrack,
							selLeftAbovePos_.order, selLeftAbovePos_.step,
							selRightBelowPos_.track, selRightBelowPos_.colInTrack, selRightBelowPos_.step);
	comStack_.lock()->push(new ReversePatternQtCommand(this));
}

/********** Events **********/
bool PatternEditorPanel::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::KeyPress:
		return keyPressed(dynamic_cast<QKeyEvent*>(event));
	case QEvent::KeyRelease:
		return keyReleased(dynamic_cast<QKeyEvent*>(event));
	case QEvent::HoverMove:
		return mouseHoverd(dynamic_cast<QHoverEvent*>(event));
	default:
		return QWidget::event(event);
	}
}

bool PatternEditorPanel::keyPressed(QKeyEvent *event)
{
	/* General Keys */
	switch (event->key()) {
	case Qt::Key_Shift:
		shiftPressedPos_ = curPos_;
		return true;
	case Qt::Key_Left:
		moveCursorToRight(-1);
		if (event->modifiers().testFlag(Qt::ShiftModifier))
			setSelectedRectangle(shiftPressedPos_, curPos_);
		return true;
	case Qt::Key_Right:
		moveCursorToRight(1);
		if (event->modifiers().testFlag(Qt::ShiftModifier))
			setSelectedRectangle(shiftPressedPos_, curPos_);
		return true;
	case Qt::Key_Up:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.order == curPos_.order) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_Down:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.order == curPos_.order) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_Tab:
		if (curPos_.track == songStyle_.trackAttribs.size() - 1)
			moveCursorToRight(-calculateColNumInRow(curPos_.track, curPos_.colInTrack));
		else
			moveCursorToRight(5 + 2 * rightEffn_[curPos_.track] - curPos_.colInTrack);
		return true;
	case Qt::Key_Backtab:
		if (curPos_.track == 0)
			moveCursorToRight(getFullColmunSize() - 1);
		else
			moveCursorToRight(-5 - 2 * rightEffn_[curPos_.track - 1] - curPos_.colInTrack);
		return true;
	case Qt::Key_Home:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-curPos_.step);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.order == curPos_.order) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_End:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(
						bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - curPos_.step - 1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.order == curPos_.order) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_PageUp:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-4);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.order == curPos_.order) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_PageDown:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(4);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.order == curPos_.order) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_Insert:
		if (bt_->isJamMode()) {
			return false;
		}
		else {
			insertStep();
			return true;
		}
	case Qt::Key_Backspace:
		if (bt_->isJamMode()) {
			return false;
		}
		else {
			deletePreviousStep();
			return true;
		}
	default:
		if (!bt_->isJamMode()) {
			// Pattern edit
			switch (curPos_.colInTrack) {
			case 0:
				return enterToneData(event->key());
			case 1:
				return enterInstrumentData(event->key());
			case 2:
				return enterVolumeData(event->key());
			case 3:
			case 5:
			case 7:
			case 9:
				return enterEffectID(event->key());
			case 4:
			case 6:
			case 8:
			case 10:
				return enterEffectValue(event->key());
			}
		}
		return false;
	}
}

bool PatternEditorPanel::keyReleased(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Shift:
		shiftPressedPos_ = { -1, -1, -1, -1 };
		return true;
	default:
		return false;
	}
}

void PatternEditorPanel::paintEvent(QPaintEvent *event)
{	
	if (bt_ != nullptr) {
		// Check order size
		size_t odrSize = bt_->getOrderSize(curSongNum_);
		if (curPos_.order >= odrSize) curPos_.setRows(odrSize - 1, 0);

		drawPattern(event->rect());
	}
}

void PatternEditorPanel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowBaselineY_ = (geometry().height() - headerHeight_) / 2 + headerHeight_;
	curRowY_ = curRowBaselineY_ - (stepFontAscend_ + stepFontLeading_ / 2);

	initDisplay();
}

void PatternEditorPanel::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event)

	setFocus();

	mousePressPos_ = hovPos_;
	mouseReleasePos_ = { -1, -1, -1, -1 };

	if (event->button() == Qt::LeftButton) {
		if (mousePressPos_.order == -2 && mousePressPos_.track >= 0) {
			int w = calculateTracksWidthWithRowNum(leftTrackNum_, mousePressPos_.track - 1)
					+ hdMuteToggleWidth_ + widthSpace_;
			if (w < event->pos().x() && event->pos().x() < w + hdEffCompandButtonWidth_ + widthSpace_) {
				if (event->pos().y() < headerHeight_ / 2) isPressedPlus_ = true;
				else isPressedMinus_ = true;
			}
		}
		selLeftAbovePos_ = { -1, -1, -1, -1 };
		selRightBelowPos_ = { -1, -1, -1, -1 };
		selectAllState_ = -1;
		emit selected(false);
	}
}

void PatternEditorPanel::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		if (mousePressPos_.track < 0 || mousePressPos_.order < 0) return;	// Start point is out f range

		if (mousePressPos_.order == hovPos_.order && hovPos_.track >= 0) {
			setSelectedRectangle(mousePressPos_, hovPos_);
			update();
		}

		if (event->x() < stepNumWidth_ && leftTrackNum_ > 0) {
			moveCursorToRight(-(5 + 2 * rightEffn_.at(leftTrackNum_ - 1)));
		}
		else if (event->x() > geometry().width() - stepNumWidth_ && hovPos_.track != -1) {
			moveCursorToRight(5 + 2 * rightEffn_.at(leftTrackNum_));
		}
		if (event->pos().y() < headerHeight_ + stepFontHeight_) {
			moveCursorToDown(-1);
		}
		else if (event->pos().y() > geometry().height() - stepFontHeight_) {
			moveCursorToDown(1);
		}
	}
}

void PatternEditorPanel::mouseReleaseEvent(QMouseEvent* event)
{
	mouseReleasePos_ = hovPos_;

	switch (event->button()) {
	case Qt::LeftButton:
		if (mousePressPos_ == mouseReleasePos_) {	// Jump cell
			if (hovPos_.order >= 0 && hovPos_.step >= 0
					&& hovPos_.track >= 0 && hovPos_.colInTrack >= 0) {
				int horDif = calculateColumnDistance(curPos_.track, curPos_.colInTrack,
													 hovPos_.track, hovPos_.colInTrack);
				int verDif = calculateStepDistance(curPos_.order, curPos_.step,
												   hovPos_.order, hovPos_.step);
				moveCursorToRight(horDif);
				moveCursorToDown(verDif);
				update();
			}
			else if (hovPos_.order == -2 && hovPos_.track >= 0) {	// Header
				if (isPressedPlus_) {
					expandEffect(hovPos_.track);
					isPressedPlus_ = false;
				}
				else if (isPressedMinus_) {
					shrinkEffect(hovPos_.track);
					isPressedMinus_ = false;
				}
				else {
					bt_->setTrackMuteState(hovPos_.track, !bt_->isMute(hovPos_.track));	// Toggle mute
					isMuteElse_ = false;

					int horDif = calculateColumnDistance(curPos_.track, curPos_.colInTrack,
														 hovPos_.track, 0);
					moveCursorToRight(horDif);
				}
				update();
			}
			else if (hovPos_.track == -2 && hovPos_.order >= 0 && hovPos_.step >= 0) {	// Step number
				int verDif = calculateStepDistance(curPos_.order, curPos_.step,
												   hovPos_.order, hovPos_.step);
				moveCursorToDown(verDif);
				update();
			}
		}
		break;

	case Qt::RightButton:	// Show context menu
	{
		QMenu menu;
		QAction* undo = menu.addAction("Undo", this, [&]() {
			bt_->undo();
			comStack_.lock()->undo();
		});
		QAction* redo = menu.addAction("Redo", this, [&]() {
			bt_->redo();
			comStack_.lock()->redo();
		});
		menu.addSeparator();
		QAction* copy = menu.addAction("Copy", this, &PatternEditorPanel::copySelectedCells);
		QAction* cut = menu.addAction("Cut", this, &PatternEditorPanel::cutSelectedCells);
		QAction* paste = menu.addAction("Paste", this, [&]() { pasteCopiedCells(mousePressPos_); });
		QAction* pasteMix = menu.addAction("Paste Mix", this, [&]() { pasteMixCopiedCells(mousePressPos_); });
		QAction* erase = menu.addAction("Erase", this, &PatternEditorPanel::eraseSelectedCells);
		QAction* select = menu.addAction("Select All", this, [&]() { onSelectPressed(1); });
		menu.addSeparator();
		auto pattern = new QMenu("Pattern");
		menu.addMenu(pattern);
		QAction* interpolate = pattern->addAction("Interpolate", this, &PatternEditorPanel::onInterpolatePressed);
		QAction* reverse = pattern->addAction("Reverse", this, &PatternEditorPanel::onReversePressed);
		pattern->addSeparator();
		QAction* expand = pattern->addAction("Expand", this, &PatternEditorPanel::onExpandPressed);
		QAction* shrink = pattern->addAction("Shrink", this, &PatternEditorPanel::onShrinkPressed);
		pattern->addSeparator();
		auto transpose = new QMenu("Transpose");
		pattern->addMenu(transpose);
		QAction* deNote = transpose->addAction("Decrease Note", this, [&]() { onTransposePressed(false, false); });
		QAction* inNote = transpose->addAction("Increase Note", this, [&]() { onTransposePressed(false, true); });
		QAction* deOct = transpose->addAction("Decrease Octave", this, [&]() { onTransposePressed(true, false); });
		QAction* inOct = transpose->addAction("Increase Octave", this, [&]() { onTransposePressed(true, true); });
		menu.addSeparator();
		QAction* mute = menu.addAction("Mute Track", this, &PatternEditorPanel::onMuteTrackPressed);
		QAction* solo = menu.addAction("Solo Track", this, &PatternEditorPanel::onSoloTrackPressed);

		if (bt_->isJamMode() || mousePressPos_.order < 0 || mousePressPos_.track < 0) {
			copy->setEnabled(false);
			cut->setEnabled(false);
			paste->setEnabled(false);
			pasteMix->setEnabled(false);
			erase->setEnabled(false);
			interpolate->setEnabled(false);
			reverse->setEnabled(false);
			expand->setEnabled(false);
			shrink->setEnabled(false);
			deNote->setEnabled(false);
			inNote->setEnabled(false);
			deOct->setEnabled(false);
			inOct->setEnabled(false);
		}
		else {
			QString clipText = QApplication::clipboard()->text();
			if (!clipText.startsWith("PATTERN_COPY") && !clipText.startsWith("PATTERN_CUT")) {
					paste->setEnabled(false);
					pasteMix->setEnabled(false);
			}
			if (selRightBelowPos_.order < 0
					|| !isSelectedCell(mousePressPos_.track, mousePressPos_.colInTrack,
									   mousePressPos_.order, mousePressPos_.step)) {
				copy->setEnabled(false);
				cut->setEnabled(false);
				erase->setEnabled(false);
				interpolate->setEnabled(false);
				reverse->setEnabled(false);
				expand->setEnabled(false);
				shrink->setEnabled(false);
				deNote->setEnabled(false);
				inNote->setEnabled(false);
				deOct->setEnabled(false);
				inOct->setEnabled(false);
			}
		}
		if (!comStack_.lock()->canUndo()) {
			undo->setEnabled(false);
		}
		if (!comStack_.lock()->canRedo()) {
			redo->setEnabled(false);
		}

		menu.exec(mapToGlobal(event->pos()));
		break;
	}

	default:
		break;
	}

	mousePressPos_ = { -1, -1, -1, -1 };
	mouseReleasePos_ = { -1, -1, -1, -1 };
}

bool PatternEditorPanel::mouseHoverd(QHoverEvent *event)
{
	QPoint pos = event->pos();
	PatternPosition oldPos = hovPos_;

	// Detect Step
	if (pos.y() <= headerHeight_) {
		// Track header
		hovPos_.setRows(-2, -2);
	}
	else {
		if (pos.y() < curRowY_) {
			int tmpOdr = curPos_.order;
			int tmpStep = curPos_.step +  (pos.y() - curRowY_) / stepFontHeight_ - 1;
			while (true) {
				if (tmpStep < 0) {
					if (tmpOdr == 0) {
						hovPos_.setRows(-1, -1);
						break;
					}
					else {
						tmpStep += bt_->getPatternSizeFromOrderNumber(curSongNum_, --tmpOdr);
					}
				}
				else {
					hovPos_.setRows(tmpOdr, tmpStep);
					break;
				}
			}
		}
		else {
			int tmpOdr = curPos_.order;
			int tmpStep = curPos_.step +  (pos.y() - curRowY_) / stepFontHeight_;
			while (true) {
				int endStep = bt_->getPatternSizeFromOrderNumber(curSongNum_, tmpOdr);
				if (tmpStep < endStep) {
					hovPos_.setRows(tmpOdr, tmpStep);
					break;
				}
				else {
					if (tmpOdr == bt_->getOrderSize(curSongNum_) - 1) {
						hovPos_.setRows(-1, -1);
						break;
					}
					else {
						++tmpOdr;
						tmpStep -= endStep;
					}
				}
			}
		}
	}

	// Detect column
	if (pos.x() <= stepNumWidth_) {
		// Row number
		hovPos_.setCols(-2, -2);
	}
	else {
		int tmpWidth = stepNumWidth_;
		for (int i = leftTrackNum_; ; ) {
			tmpWidth += (toneNameWidth_ + stepFontWidth_);
			if (pos.x() <= tmpWidth) {
				hovPos_.setCols(i, 0);
				break;
			}
			tmpWidth += (instWidth_ + stepFontWidth_);
			if (pos.x() <= tmpWidth) {
				hovPos_.setCols(i, 1);
				break;
			}
			tmpWidth += (volWidth_ + stepFontWidth_);
			if (pos.x() <= tmpWidth) {
				hovPos_.setCols(i, 2);
				break;
			}
			bool flag = false;
			for (int j = 0; j <= rightEffn_.at(i); ++j) {
				tmpWidth += (effIDWidth_ + widthSpace_);
				if (pos.x() <= tmpWidth) {
					hovPos_.setCols(i, 3 + 2 * j);
					flag = true;
					break;
				}
				tmpWidth += (effValWidth_ + widthSpace_);
				if (pos.x() <= tmpWidth) {
					hovPos_.setCols(i, 4 + 2 * j);
					flag = true;
					break;
				}
			}
			if (flag) break;
			++i;

			if (i == songStyle_.trackAttribs.size()) {
				hovPos_.setCols(-1, -1);
				break;
			}
		}
	}

	if (hovPos_ != oldPos) update();

	return true;
}

void PatternEditorPanel::wheelEvent(QWheelEvent *event)
{
	int degree = event->angleDelta().y() / 8;
	moveCursorToDown(-degree / 15);
}

void PatternEditorPanel::leaveEvent(QEvent* event)
{
	Q_UNUSED(event)
	// Clear mouse hover selection
	hovPos_ = { -1, -1, -1, -1 };
}
