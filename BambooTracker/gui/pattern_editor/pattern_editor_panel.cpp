#include "pattern_editor_panel.hpp"
#include <algorithm>
#include <vector>
#include <utility>
#include <stdexcept>
#include <QPainter>
#include <QFontMetrics>
#include <QPoint>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMetaMethod>
#include "gui/event_guard.hpp"
#include "gui/command/pattern/pattern_commands_qt.hpp"
#include "midi/midi.hpp"
#include "jam_manager.hpp"

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
	  doubleClickPos_{ -1, -1, -1, -1 },
	  isIgnoreToSlider_(false),
	  isIgnoreToOrder_(false),
	  isPressedPlus_(false),
	  isPressedMinus_(false),
	  entryCnt_(0),
	  selectAllState_(-1),
	  isMuteElse_(false),
	  hl1Cnt_(4),
	  hl2Cnt_(16),
	  editableStepCnt_(1)
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
	stepFontLeading_ = metrics.descent() / 2;
	stepFontHeight_ = stepFontAscend_ + stepFontLeading_;
	QFontMetrics m(headerFont_);

	/* Width & height */
	widthSpace_ = stepFontWidth_ / 5 * 2;
	widthSpaceDbl_ = widthSpace_ * 2;
	stepNumWidth_ = stepFontWidth_ * 2 + widthSpace_;
	toneNameWidth_ = stepFontWidth_ * 3;
	instWidth_ = stepFontWidth_ * 2;
	volWidth_ = stepFontWidth_ * 2;
	effIDWidth_ = stepFontWidth_ * 2;
	effValWidth_ = stepFontWidth_ * 2;
	effWidth_ = effIDWidth_ + effValWidth_ + widthSpaceDbl_;
	baseTrackWidth_ = toneNameWidth_ + instWidth_ + volWidth_
					  + effIDWidth_ + effValWidth_ + widthSpaceDbl_ * 4;
	hdEffCompandButtonWidth_ = m.width("+");
	hdMuteToggleWidth_ = baseTrackWidth_ - hdEffCompandButtonWidth_ - stepFontWidth_ / 2 * 3;
	headerHeight_ = stepFontHeight_ * 2;
	hdPlusY_ = headerHeight_ / 4 + m.height() / 2 - m.leading() / 2 - m.descent();
	hdMinusY_ = headerHeight_ / 2 + hdPlusY_;

	rightEffn_ = std::vector<int>(15);

	initDisplay();

	setAttribute(Qt::WA_Hover);
	setContextMenuPolicy(Qt::CustomContextMenu);

	midiKeyEventMethod_ = metaObject()->indexOfSlot("midiKeyEvent(uchar,uchar,uchar)");
	Q_ASSERT(midiKeyEventMethod_ != -1);
	MidiInterface::instance().installInputHandler(&midiThreadReceivedEvent, this);
}

PatternEditorPanel::~PatternEditorPanel()
{
	MidiInterface::instance().uninstallInputHandler(&midiThreadReceivedEvent, this);
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

void PatternEditorPanel::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
}

void PatternEditorPanel::setColorPallete(std::weak_ptr<ColorPalette> palette)
{
	palette_ = palette;
}

int PatternEditorPanel::getCurrentTrack() const
{
	return curPos_.track;
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
					 bt_->isJamMode() ? palette_.lock()->ptnCurRowColor : palette_.lock()->ptnCurEditRowColor);
	// Step number
	if (hovPos_.track == -2 && hovPos_.order == curPos_.order && hovPos_.step == curPos_.step)
		painter.fillRect(0, curRowY_, stepNumWidth_, stepFontHeight_, palette_.lock()->ptnHovCellColor);	// Paint hover
	if (curPos_.step % hl2Cnt_) {
		painter.setPen(!(curPos_.step % hl2Cnt_) ? palette_.lock()->ptnHl1StepNumColor
												 : !(curPos_.step % hl1Cnt_) ? palette_.lock()->ptnHl1StepNumColor
																			 : palette_.lock()->ptnDefStepNumColor);
	}
	else {
		painter.setPen(palette_.lock()->ptnHl2StepNumColor);
	}
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
			else if (config_.lock()->getShowPreviousNextOrders()) {
				--odrNum;
				stepNum = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, odrNum)) - 1;
			}
			else {
				break;
			}
		}

		QColor rowColor, textColor;
		if (!config_.lock()->getFollowMode() && odrNum == playOdrNum && stepNum == playStepNum) {
			rowColor = palette_.lock()->ptnPlayRowColor;
			textColor = palette_.lock()->ptnPlayTextColor;
		}
		else {
			rowColor = !(stepNum % hl2Cnt_) ? palette_.lock()->ptnHl2RowColor
											: !(stepNum % hl1Cnt_) ? palette_.lock()->ptnHl1RowColor
																   : palette_.lock()->ptnDefRowColor;
			textColor = palette_.lock()->ptnDefTextColor;
		}

		// Fill row
		painter.fillRect(0, rowY, maxWidth, stepFontHeight_, rowColor);
		// Step number
		if (hovPos_.track == -2 && hovPos_.order == odrNum && hovPos_.step == stepNum)
			painter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, palette_.lock()->ptnHovCellColor);	// Paint hover
		painter.setPen(!(stepNum % hl2Cnt_) ? palette_.lock()->ptnHl2StepNumColor
											: !(stepNum % hl1Cnt_) ? palette_.lock()->ptnHl1StepNumColor
																   : palette_.lock()->ptnDefStepNumColor);
		painter.drawText(1, baseY, QString("%1").arg(stepNum, 2, 16, QChar('0')).toUpper());
		// Step data
		painter.setPen(textColor);
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			x += drawStep(painter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (odrNum != curPos_.order)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, palette_.lock()->ptnMaskColor);
	}

	int stepEnd = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order));

	/* Next rows */
	for (rowY = curRowY_ + stepFontHeight_, baseY = curRowBaselineY_ + stepFontHeight_,
		 stepNum = curPos_.step + 1, odrNum = curPos_.order;
		 rowY <= geometry().height();
		 rowY += stepFontHeight_, baseY += stepFontHeight_, ++stepNum) {
		if (stepNum == stepEnd) {
			if (odrNum == static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1) {
				break;
			}
			else if (config_.lock()->getShowPreviousNextOrders()) {
				++odrNum;
				stepNum = 0;
				stepEnd = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, odrNum));
			}
			else {
				break;
			}
		}

		QColor rowColor, textColor;
		if (!config_.lock()->getFollowMode() && odrNum == playOdrNum && stepNum == playStepNum) {
			rowColor = palette_.lock()->ptnPlayRowColor;
			textColor = palette_.lock()->ptnPlayTextColor;
		}
		else {
			rowColor = !(stepNum % hl2Cnt_) ? palette_.lock()->ptnHl2RowColor
											: !(stepNum % hl1Cnt_) ? palette_.lock()->ptnHl1RowColor
																   : palette_.lock()->ptnDefRowColor;
			textColor = palette_.lock()->ptnDefTextColor;
		}

		// Fill row
		painter.fillRect(0, rowY, maxWidth, stepFontHeight_, rowColor);
		// Step number
		if (hovPos_.track == -2 && hovPos_.order == odrNum && hovPos_.step == stepNum)
			painter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, palette_.lock()->ptnHovCellColor);	// Paint hover
		painter.setPen(!(stepNum % hl2Cnt_) ? palette_.lock()->ptnHl2StepNumColor
											: !(stepNum % hl1Cnt_) ? palette_.lock()->ptnHl1StepNumColor
																   : palette_.lock()->ptnDefStepNumColor);
		painter.drawText(1, baseY, QString("%1").arg(stepNum, 2, 16, QChar('0')).toUpper());
		// Step data
		painter.setPen(textColor);
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			x += drawStep(painter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (odrNum != curPos_.order)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, palette_.lock()->ptnMaskColor);
	}
}

int PatternEditorPanel::drawStep(QPainter &painter, int trackNum, int orderNum, int stepNum, int x, int baseY, int rowY)
{
	int offset = x + widthSpace_;
	PatternPosition pos{ trackNum, 0, orderNum, stepNum };
	QColor textColor = (stepNum == curPos_.step) ? palette_.lock()->ptnCurTextColor : palette_.lock()->ptnDefTextColor;
	bool isHovTrack = (hovPos_.order == -2 && hovPos_.track == trackNum);
	bool isHovStep = (hovPos_.track == -2 && hovPos_.order == orderNum && hovPos_.step == stepNum);
	bool isMuteTrack = bt_->isMute(trackNum);
	SoundSource src = songStyle_.trackAttribs[static_cast<size_t>(trackNum)].source;


	/* Tone name */
	if (pos == curPos_)	// Paint current cell
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnCurCellColor);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnHovCellColor);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 0, orderNum, stepNum))	// Paint selected
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnSelCellColor);
	int noteNum = bt_->getStepNoteNumber(curSongNum_, trackNum, orderNum, stepNum);
	switch (noteNum) {
	case -1:	// None
		painter.setPen(textColor);
		painter.drawText(offset, baseY, "---");
		break;
	case -2:	// Key off
		painter.fillRect(offset, rowY + stepFontHeight_ * 2 / 5,
						 toneNameWidth_, stepFontHeight_ / 5, palette_.lock()->ptnToneColor);
		break;
	case -3:	// Echo 0
		painter.setPen(palette_.lock()->ptnToneColor);
		painter.drawText(offset + stepFontWidth_ / 2, baseY, "^0");
		break;
	case -4:	// Echo 1
		painter.setPen(palette_.lock()->ptnToneColor);
		painter.drawText(offset + stepFontWidth_ / 2, baseY, "^1");
		break;
	case -5:	// Echo 2
		painter.setPen(palette_.lock()->ptnToneColor);
		painter.drawText(offset + stepFontWidth_ / 2, baseY, "^2");
		break;
	case -6:	// Echo 3
		painter.setPen(palette_.lock()->ptnToneColor);
		painter.drawText(offset + stepFontWidth_ / 2, baseY, "^3");
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
		painter.setPen(palette_.lock()->ptnToneColor);
		painter.drawText(offset, baseY, toneStr + QString::number(noteNum / 12));
		break;
	}
	}
	if (isMuteTrack)	// Paint mute mask
		painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnMaskColor);
	offset += toneNameWidth_ +  widthSpaceDbl_;
	pos.colInTrack = 1;

	/* Instrument */
	if (pos == curPos_)	// Paint current cell
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnCurCellColor);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnHovCellColor);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 1, orderNum, stepNum))	// Paint selected
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnSelCellColor);
	int instNum = bt_->getStepInstrument(curSongNum_, trackNum, orderNum, stepNum);
	if (instNum == -1) {
		painter.setPen(textColor);
		painter.drawText(offset, baseY, "--");
	}
	else {
		std::unique_ptr<AbstractInstrument> inst = bt_->getInstrument(instNum);
		painter.setPen((inst != nullptr && src == inst->getSoundSource())
					   ? palette_.lock()->ptnInstColor
					   : palette_.lock()->ptnErrorColor);
		painter.drawText(offset, baseY, QString("%1").arg(instNum, 2, 16, QChar('0')).toUpper());
	}
	if (isMuteTrack)	// Paint mute mask
		painter.fillRect(offset - widthSpace_, rowY, instWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnMaskColor);
	offset += instWidth_ +  widthSpaceDbl_;
	pos.colInTrack = 2;

	/* Volume */
	if (pos == curPos_)	// Paint current cell
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnCurCellColor);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnHovCellColor);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 2, orderNum, stepNum))	// Paint selected
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnSelCellColor);
	int vol = bt_->getStepVolume(curSongNum_, trackNum, orderNum, stepNum);
	if (vol == -1) {
		painter.setPen(textColor);
		painter.drawText(offset, baseY, "--");
	}
	else {
		int volLim = 0;	// Dummy set
		switch (src) {
		case SoundSource::FM:	volLim = 0x80;	break;
		case SoundSource::SSG:	volLim = 0x10;	break;
		case SoundSource::DRUM:	volLim = 0x20;	break;
		}
		painter.setPen((vol < volLim) ? palette_.lock()->ptnVolColor : palette_.lock()->ptnErrorColor);
		if (src == SoundSource::FM && vol < volLim && config_.lock()->getReverseFMVolumeOrder()) {

			vol = volLim - vol - 1;
		}
		painter.drawText(offset, baseY, QString("%1").arg(vol, 2, 16, QChar('0')).toUpper());
	}
	if (isMuteTrack)	// Paint mute mask
		painter.fillRect(offset - widthSpace_, rowY, volWidth_ + widthSpaceDbl_, stepFontHeight_, palette_.lock()->ptnMaskColor);
	offset += volWidth_ +  widthSpaceDbl_;
	pos.colInTrack = 3;

	/* Effect */
	for (int i = 0; i <= rightEffn_.at(static_cast<size_t>(trackNum)); ++i) {
		/* Effect ID */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnCurCellColor);
		if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnHovCellColor);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
				&& isSelectedCell(trackNum, pos.colInTrack, orderNum, stepNum))	// Paint selected
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnSelCellColor);
		QString effStr = QString::fromStdString(bt_->getStepEffectID(curSongNum_, trackNum, orderNum, stepNum, i));
		if (effStr == "--") {
			painter.setPen(textColor);
			painter.drawText(offset, baseY, effStr);
		}
		else {
			painter.setPen(palette_.lock()->ptnEffIDColor);
			painter.drawText(offset, baseY, effStr);
		}
		if (isMuteTrack)	// Paint mute mask
			painter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnMaskColor);
		offset += effIDWidth_;
		++pos.colInTrack;

		/* Effect Value */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnCurCellColor);
		if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnHovCellColor);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
				&& isSelectedCell(trackNum, pos.colInTrack, orderNum, stepNum))	// Paint selected
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnSelCellColor);
		int effVal = bt_->getStepEffectValue(curSongNum_, trackNum, orderNum, stepNum, i);
		if (effVal == -1) {
			painter.setPen(textColor);
			painter.drawText(offset, baseY, "--");
		}
		else {
			painter.setPen(palette_.lock()->ptnEffValColor);
			if (src == SoundSource::FM && config_.lock()->getReverseFMVolumeOrder() && effStr.at(0) == 'M')
				effVal = 0x7f - effVal;
			painter.drawText(offset, baseY, QString("%1").arg(effVal, 2, 16, QChar('0')).toUpper());
		}
		if (isMuteTrack)	// Paint mute mask
			painter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, palette_.lock()->ptnMaskColor);
		offset += effValWidth_ + widthSpaceDbl_;
		++pos.colInTrack;
	}

	return baseTrackWidth_ + effWidth_ * rightEffn_[static_cast<size_t>(trackNum)];
}

void PatternEditorPanel::drawHeaders(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, palette_.lock()->ptnHeaderRowColor);
	int x, trackNum;
	int lspace = stepFontWidth_ / 2;
	for (x = stepNumWidth_ + lspace, trackNum = leftTrackNum_; x < maxWidth; ) {
		int tw = baseTrackWidth_ + effWidth_ * rightEffn_.at(static_cast<size_t>(trackNum));
		if (hovPos_.order == -2 && hovPos_.track == trackNum)
			painter.fillRect(x - lspace, 0, tw, headerHeight_, palette_.lock()->ptnHovCellColor);

		painter.setPen(palette_.lock()->ptnHeaderTextColor);
		QString srcName;
		const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(trackNum)];
		switch (attrib.source) {
		case SoundSource::FM:
			switch (songStyle_.type) {
			case SongType::STD:
				srcName = "FM" + QString::number(attrib.channelInSource + 1);
				break;
			case SongType::FMEX:
				switch (attrib.channelInSource) {
				case 2:
					srcName = "FM3-OP1";
					break;
				case 6:
					srcName = "FM3-OP2";
					break;
				case 7:
					srcName = "FM3-OP3";
					break;
				case 8:
					srcName = "FM3-OP4";
					break;
				default:
					srcName = "FM" + QString::number(attrib.channelInSource + 1);
					break;
				}
				break;
			}
			break;
		case SoundSource::SSG:
			srcName = "SSG" + QString::number(attrib.channelInSource + 1);
			break;
		case SoundSource::DRUM:
			switch (attrib.channelInSource) {
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
						 bt_->isMute(trackNum) ? palette_.lock()->ptnMuteColor : palette_.lock()->ptnUnmuteColor);

		painter.drawText(x + hdMuteToggleWidth_ + lspace, hdPlusY_, "+");
		painter.drawText(x + hdMuteToggleWidth_ + lspace, hdMinusY_, "-");

		x += tw;
		++trackNum;
	}
}

void PatternEditorPanel::drawBorders(int maxWidth)
{
	QPainter painter(pixmap_.get());

	painter.drawLine(0, headerHeight_, geometry().width(), headerHeight_);
	painter.drawLine(stepNumWidth_, 0, stepNumWidth_, geometry().height());
	size_t trackNum = static_cast<size_t>(leftTrackNum_);
	for (int x = stepNumWidth_; trackNum < rightEffn_.size(); ) {
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

// NOTE: end >= -1
int PatternEditorPanel::calculateTracksWidthWithRowNum(int begin, int end) const
{
	int width = stepNumWidth_;
	for (int i = begin; i <= end; ++i) {
		width += (baseTrackWidth_ + effWidth_ * rightEffn_.at(static_cast<size_t>(i)));
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
		size_t tn = static_cast<size_t>(trackNum);
		for (size_t i = 0; i < tn; ++i) {
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
			int lim = 5 + 2 * rightEffn_.at(static_cast<size_t>(curPos_.track));
			if (curPos_.colInTrack < lim) {
				break;
			}
			else {
				if (curPos_.track == static_cast<int>(songStyle_.trackAttribs.size()) - 1) {
					if (config_.lock()->getWarpCursor()) {
						curPos_.track = 0;
					}
					else {
						curPos_.colInTrack = lim - 1;
						break;
					}
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
					if (config_.lock()->getWarpCursor()) {
						curPos_.track = static_cast<int>(songStyle_.trackAttribs.size()) - 1;
					}
					else {
						curPos_.colInTrack = 0;
						break;
					}
				}
				else {
					--curPos_.track;
				}
				curPos_.colInTrack += (5 + 2 * rightEffn_.at(static_cast<size_t>(curPos_.track)));
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
			= calculateTracksWidthWithRowNum(leftTrackNum_, static_cast<int>(songStyle_.trackAttribs.size()) - 1);

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
			int dif = tmp - static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order));
			if (dif < 0) {
				curPos_.step = tmp;
				break;
			}
			else {
				if (config_.lock()->getWarpAcrossOrders()) {
					if (curPos_.order == static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1) {
						curPos_.order = 0;
					}
					else {
						++curPos_.order;
					}
				}
				tmp = dif;
			}
		}
	}
	else {
		while (true) {
			if (tmp < 0) {
				if (config_.lock()->getWarpAcrossOrders()) {
					if (curPos_.order == 0) {
						curPos_.order = static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1;
					}
					else {
						--curPos_.order;
					}
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
		emit currentStepChanged(
				curPos_.step, static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order)) - 1);

	if (!isIgnoreToOrder_ && curPos_.order != oldOdr)	// Send to order list
		emit currentOrderChanged(
				curPos_.order, static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1);

	update();
}

void PatternEditorPanel::expandEffect(int trackNum)
{
	size_t tn = static_cast<size_t>(trackNum);
	if (rightEffn_.at(tn) == 3) return;
	++rightEffn_[tn];
	TracksWidthFromLeftToEnd_ = calculateTracksWidthWithRowNum(
									leftTrackNum_, static_cast<int>(songStyle_.trackAttribs.size()) - 1);

	emit effectColsCompanded(calculateColNumInRow(curPos_.track, curPos_.colInTrack), getFullColmunSize());
}

void PatternEditorPanel::shrinkEffect(int trackNum)
{
	size_t tn = static_cast<size_t>(trackNum);
	if (rightEffn_.at(tn) == 0) return;
	--rightEffn_[tn];
	TracksWidthFromLeftToEnd_ = calculateTracksWidthWithRowNum(
									leftTrackNum_, static_cast<int>(songStyle_.trackAttribs.size()) - 1);

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
			startStep = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, --startOrder));
		}
	}

	return flag ? d : -d;
}

QPoint PatternEditorPanel::calculateCurrentCursorPosition() const
{
	int w = calculateTracksWidthWithRowNum(leftTrackNum_, curPos_.track - 1);
	if (curPos_.colInTrack > 0) {
		w = w + toneNameWidth_ + widthSpaceDbl_;
		if (curPos_.colInTrack > 1) {
			w = w + instWidth_ + widthSpaceDbl_;
			if (curPos_.colInTrack > 2) {
				w = w + volWidth_ + widthSpaceDbl_;
				for (int i = 3; i < 11; ++i) {
					if (curPos_.colInTrack == i) break;
					w = w + widthSpace_ + ((i % 2) ? effIDWidth_ : effValWidth_);
				}
			}
		}
	}
	return QPoint(w, curRowY_);
}

void PatternEditorPanel::changeEditable()
{
	update();
}

int PatternEditorPanel::getFullColmunSize() const
{
	return calculateColNumInRow(static_cast<int>(songStyle_.trackAttribs.size()) - 1, 4 + 2 * rightEffn_.back());
}

void PatternEditorPanel::updatePosition()
{
	curPos_.setRows(bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());

	emit currentOrderChanged(curPos_.order, static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1);
	emit currentStepChanged(
				curPos_.step, static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order)) - 1);

	update();
}

JamKey PatternEditorPanel::getJamKeyFromLayoutMapping(Qt::Key key) {
	std::shared_ptr<Configuration> configLocked = config_.lock();
	Configuration::KeyboardLayout selectedLayout = configLocked->getNoteEntryLayout();
	if (configLocked->mappingLayouts.find (selectedLayout) != configLocked->mappingLayouts.end()) {
		std::map<Qt::Key, JamKey> selectedLayoutMapping = configLocked->mappingLayouts.at (selectedLayout);
		if (selectedLayoutMapping.find (key) != selectedLayoutMapping.end()) {
			return selectedLayoutMapping.at (key);
		} else {
			throw std::invalid_argument("Unmapped key");
		}
	} else throw std::out_of_range("Unmapped Layout");
}

bool PatternEditorPanel::enterToneData(QKeyEvent* event)
{
	QString seq = QKeySequence(static_cast<int>(event->modifiers()) | event->key()).toString();
	if (seq == QKeySequence(
				QString::fromUtf8(config_.lock()->getKeyOffKey().c_str(),
								  static_cast<int>(config_.lock()->getKeyOffKey().length()))).toString()) {
		bt_->setStepKeyOff(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
		comStack_.lock()->push(new SetKeyOffToStepQtCommand(this));
		if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
		return true;
	}
	else if (seq == QKeySequence(
				 QString::fromUtf8(config_.lock()->getEchoBufferKey().c_str(),
								   static_cast<int>(config_.lock()->getEchoBufferKey().length()))).toString()) {
		int n = bt_->getCurrentOctave();
		if (n > 3) n = 3;
		bt_->setEchoBufferAccess(curSongNum_, curPos_.track, curPos_.order, curPos_.step, n);
		comStack_.lock()->push(new SetEchoBufferAccessQtCommand(this));
		if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
		return true;
	}

	int baseOct = bt_->getCurrentOctave();

	if (event->modifiers().testFlag(Qt::NoModifier)) {
		Qt::Key qtKey = static_cast<Qt::Key> (event->key());
		JamKey possibleJamKey;
		try {
			possibleJamKey = getJamKeyFromLayoutMapping(qtKey);
			setStepKeyOn(JamManager::jamKeyToNote (possibleJamKey), baseOct + (static_cast<int>(possibleJamKey) / 12));
		} catch (std::invalid_argument) {}
	}

	return false;
}

void PatternEditorPanel::setStepKeyOn(Note note, int octave)
{
	if (octave < 8) {
		bt_->setStepNote(curSongNum_, curPos_.track, curPos_.order, curPos_.step, octave, note);
		comStack_.lock()->push(new SetKeyOnToStepQtCommand(this));
		if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
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

	emit instrumentEntered(
				bt_->getStepInstrument(curSongNum_, editPos_.track, editPos_.order, editPos_.step));

	if (!bt_->isPlaySong() && !entryCnt_) moveCursorToDown(editableStepCnt_);
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
	bool isReversed = (songStyle_.trackAttribs[static_cast<size_t>(curPos_.track)].source == SoundSource::FM
					  && config_.lock()->getReverseFMVolumeOrder());
	bt_->setStepVolume(curSongNum_, editPos_.track, editPos_.order, editPos_.step, volume, isReversed);
	comStack_.lock()->push(new SetVolumeToStepQtCommand(this, editPos_));

	if (!bt_->isPlaySong() && !entryCnt_) moveCursorToDown(editableStepCnt_);
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

	if (!bt_->isPlaySong() && !entryCnt_) {
		if (config_.lock()->getMoveCursorToRight()) moveCursorToRight(1);
		else moveCursorToDown(editableStepCnt_);
	}

	// Send effect description
	QString effDetail = tr("Invalid effect");
	std::string id = bt_->getStepEffectID(curSongNum_, editPos_.track, editPos_.order,
										  editPos_.step, (editPos_.colInTrack - 3) / 2);
	SoundSource src = songStyle_.trackAttribs.at(static_cast<size_t>(editPos_.track)).source;
	if (id == "00") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("00xy - Arpeggio, x: 2nd note (0-F), y: 3rd note (0-F)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "01") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("01xx - Portamento up, xx: depth (00-FF)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "02") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("02xx - Portamento down, xx: depth (00-FF)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "03") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("03xx - Tone portamento, xx: depth (00-FF)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "04") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("04xy - Vibrato, x: period (0-F), y: depth (0-F)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "07") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("07xx - Tremolo, x: period (0-F), y: depth (0-F)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "08") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::DRUM:
			effDetail = tr("08xx - Pan, xx: 00 = no sound, 01 = right, 02 = left, 03 = center");
			break;
		case SoundSource::SSG:
			break;
		}
	}
	else if (id == "0A") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("0Axy - Volume slide, x: up (0-F), y: down (0-F)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "0B") {
		effDetail = tr("0Bxx - Jump to begginning of order xx");
	}
	else if (id == "0C") {
		effDetail = tr("0Cxx - End of song");
	}
	else if (id == "0D") {
		effDetail = tr("0Dxx - Jump to step xx of next order");
	}
	else if (id == "0F") {
		effDetail = tr("0Fxx - Change speed (xx: 00-1F), change tempo (xx: 20-FF)");
	}
	else if (id == "0G") {
		effDetail = tr("0Gxx - Note delay, xx: count (00-FF)");
	}
	else if (id == "0O") {
		effDetail = tr("0Oxx - Set groove xx");
	}
	else if (id == "0P") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("0Pxx - Detune, xx: pitch (00-FF)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "0Q") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("0Qxy - Note slide up, x: count (0-F), y: seminote (0-F)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "0R") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("0Rxy - Note slide down, x: count (0-F), y: seminote (0-F)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "0S") {
		effDetail = tr("0Sxx - Note cut, xx: count (01-FF)");
	}
	else if (id == "0T") {
		switch (src) {
		case SoundSource::FM:
		case SoundSource::SSG:
			effDetail = tr("0Txy - Transpose delay, x: count (1-7: up, 9-F: down), y: seminote (0-F)");
			break;
		case SoundSource::DRUM:
			break;
		}
	}
	else if (id == "0V") {
		switch (src) {
		case SoundSource::DRUM:
			effDetail = tr("0Vxx - Master volume, xx: volume (00-3F)");
			break;
		case SoundSource::FM:
		case SoundSource::SSG:
			break;
		}
	}
	else if (id.front() == 'M') {
		if (ctohex(*(id.begin() + 1)) > 0)
			effDetail = tr("Mxyy - Volume delay, x: count (1-F), yy: volume (00-FF)");
	}

	emit effectEntered(effDetail);
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
	int n = (editPos_.colInTrack - 4) / 2;
	bool isReversed = (songStyle_.trackAttribs[static_cast<size_t>(editPos_.track)].source == SoundSource::FM
					  && config_.lock()->getReverseFMVolumeOrder()
					  && bt_->getStepEffectID(curSongNum_, editPos_.track, editPos_.order, editPos_.step, n)
					  .front() == 'M');
	bt_->setStepEffectValue(curSongNum_, editPos_.track, editPos_.order, editPos_.step, n, value, isReversed);
	comStack_.lock()->push(new SetEffectValueToStepQtCommand(this, editPos_));

	if (!bt_->isPlaySong() && !entryCnt_) {
		if (config_.lock()->getMoveCursorToRight()) moveCursorToRight(1);
		else moveCursorToDown(editableStepCnt_);
	}
}

void PatternEditorPanel::insertStep()
{
	bt_->insertStep(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
	comStack_.lock()->push(new InsertStepQtCommand(this));
}

void PatternEditorPanel::deletePreviousStep()
{
	if (curPos_.step) {
		bt_->deletePreviousStep(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
		comStack_.lock()->push(new DeletePreviousStepQtCommand(this));
		if (!bt_->isPlaySong()) moveCursorToDown(-1);
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

void PatternEditorPanel::pasteCopiedCells(const PatternPosition& startPos)
{
	int sCol = 0;
	std::vector<std::vector<std::string>> cells
			= instantiateCellsFromString(QApplication::clipboard()->text(), sCol);

	if (sCol > 2 && !((curPos_.colInTrack - sCol) % 2)
			&& static_cast<int>(cells.front().size()) <= 11 - curPos_.colInTrack)
		sCol = curPos_.colInTrack;

	bt_->pastePatternCells(curSongNum_, startPos.track, sCol,
						   startPos.order, startPos.step, std::move(cells));
	comStack_.lock()->push(new PasteCopiedDataToPatternQtCommand(this));
}

void PatternEditorPanel::pasteMixCopiedCells(const PatternPosition& startPos)
{
	int sCol = 0;
	std::vector<std::vector<std::string>> cells
			= instantiateCellsFromString(QApplication::clipboard()->text(), sCol);

	if (sCol > 2 && !((curPos_.colInTrack - sCol) % 2)
			&& static_cast<int>(cells.front().size()) <= 11 - curPos_.colInTrack)
		sCol = curPos_.colInTrack;

	bt_->pasteMixPatternCells(curSongNum_, startPos.track, sCol,
							  startPos.order, startPos.step, std::move(cells));
	comStack_.lock()->push(new PasteMixCopiedDataToPatternQtCommand(this));
}

void PatternEditorPanel::pasteOverwriteCopiedCells(const PatternPosition& startPos)
{
	int sCol = 0;
	std::vector<std::vector<std::string>> cells
			= instantiateCellsFromString(QApplication::clipboard()->text(), sCol);

	if (sCol > 2 && !((curPos_.colInTrack - sCol) % 2)
			&& static_cast<int>(cells.front().size()) <= 11 - curPos_.colInTrack)
		sCol = curPos_.colInTrack;

	bt_->pasteOverwritePatternCells(curSongNum_, startPos.track, sCol,
							  startPos.order, startPos.step, std::move(cells));
	comStack_.lock()->push(new PasteOverwriteCopiedDataToPatternQtCommand(this));
}

std::vector<std::vector<std::string>> PatternEditorPanel::instantiateCellsFromString(QString str, int& startCol)
{
	str.remove(QRegularExpression("PATTERN_(COPY|CUT):"));
	QString hdRe = "^([0-9]+),([0-9]+),([0-9]+),";
	QRegularExpression re(hdRe);
	QRegularExpressionMatch match = re.match(str);
	startCol = match.captured(1).toInt();
	int w = match.captured(2).toInt();
	size_t h = match.captured(3).toUInt();
	str.remove(re);

	std::vector<std::vector<std::string>> cells;
	re = QRegularExpression("^([^,]+),");
	for (size_t i = 0; i < h; ++i) {
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

void PatternEditorPanel::increaseNoteKey(const PatternPosition& startPos, const PatternPosition& endPos)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->increaseNoteKeyInPattern(curSongNum_,
									  beginTrack, startPos.order, startPos.step,
									  endPos.track, endPos.step);
		comStack_.lock()->push(new IncreaseNoteKeyInPatternQtCommand(this));
	}
}

void PatternEditorPanel::decreaseNoteKey(const PatternPosition& startPos, const PatternPosition& endPos)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->decreaseNoteKeyInPattern(curSongNum_,
									  beginTrack, startPos.order, startPos.step,
									  endPos.track, endPos.step);
		comStack_.lock()->push(new DecreaseNoteKeyInPatternQtCommand(this));
	}
}

void PatternEditorPanel::increaseNoteOctave(const PatternPosition& startPos, const PatternPosition& endPos)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->increaseNoteOctaveInPattern(curSongNum_,
										 beginTrack, startPos.order, startPos.step,
										 endPos.track, endPos.step);
		comStack_.lock()->push(new IncreaseNoteOctaveInPatternQtCommand(this));
	}
}

void PatternEditorPanel::decreaseNoteOctave(const PatternPosition& startPos, const PatternPosition& endPos)
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
	int patMax = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, end.order)) - 1;
	if (start.compareCols(end) > 0) {
		if (start.step > end.step) {
			selLeftAbovePos_ = end;
			selRightBelowPos_ = {
				start.track, start.colInTrack,
				end.order, (start.step > patMax) ? patMax : start.step
			};
		}
		else {
			selLeftAbovePos_ = { end.track, end.colInTrack, end.order, start.step };
			selRightBelowPos_ = { start.track, start.colInTrack, end.order, end.step };
		}
	}
	else {
		if (start.step > end.step) {
			selLeftAbovePos_ = { start.track, start.colInTrack, end.order, end.step };
			selRightBelowPos_ = {
				end.track, end.colInTrack,
				end.order, (start.step > patMax) ? patMax : start.step
			};
		}
		else {
			selLeftAbovePos_ = { start.track, start.colInTrack, end.order, start.step };
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

void PatternEditorPanel::showPatternContextMenu(const PatternPosition& pos, const QPoint& point)
{
	QMenu menu;
	// Leave Before Qt5.7.0 style due to windows xp
	QAction* undo = menu.addAction(tr("&Undo"));
	QObject::connect(undo, &QAction::triggered, this, [&]() {
		bt_->undo();
		comStack_.lock()->undo();
	});
	QAction* redo = menu.addAction(tr("&Redo"));
	QObject::connect(redo, &QAction::triggered, this, [&]() {
		bt_->redo();
		comStack_.lock()->redo();
	});
	menu.addSeparator();
	QAction* copy = menu.addAction(tr("&Copy"));
	QObject::connect(copy, &QAction::triggered, this, &PatternEditorPanel::copySelectedCells);
	QAction* cut = menu.addAction(tr("Cu&t"));
	QObject::connect(cut, &QAction::triggered, this, &PatternEditorPanel::cutSelectedCells);
	QAction* paste = menu.addAction(tr("&Paste"));
	QObject::connect(paste, &QAction::triggered, this, [&]() { pasteCopiedCells(pos); });
	auto pasteSp = new QMenu(tr("Paste Specia&l"));
	menu.addMenu(pasteSp);
	QAction* pasteMix = pasteSp->addAction(tr("&Mix"));
	QObject::connect(pasteMix, &QAction::triggered, this, [&]() { pasteMixCopiedCells(pos); });
	QAction* pasteOver = pasteSp->addAction(tr("&Overwrite"));
	QObject::connect(pasteOver, &QAction::triggered, this, [&]() { pasteOverwriteCopiedCells(pos); });
	QAction* erase = menu.addAction(tr("&Erase"));
	QObject::connect(erase, &QAction::triggered, this, &PatternEditorPanel::eraseSelectedCells);
	QAction* select = menu.addAction(tr("Select &All"));
	QObject::connect(select, &QAction::triggered, this, [&]() { onSelectPressed(1); });
	menu.addSeparator();
	auto pattern = new QMenu(tr("Patter&n"));
	menu.addMenu(pattern);
	QAction* interpolate = pattern->addAction(tr("&Interpolate"));
	QObject::connect(interpolate, &QAction::triggered, this, &PatternEditorPanel::onInterpolatePressed);
	QAction* reverse = pattern->addAction(tr("&Reverse"));
	QObject::connect(reverse, &QAction::triggered, this, &PatternEditorPanel::onReversePressed);
	QAction* replace = pattern->addAction(tr("R&eplace Instrument"));
	QObject::connect(replace, &QAction::triggered, this, &PatternEditorPanel::onReplaceInstrumentPressed);
	pattern->addSeparator();
	QAction* expand = pattern->addAction(tr("E&xpand"));
	QObject::connect(expand, &QAction::triggered, this, &PatternEditorPanel::onExpandPressed);
	QAction* shrink = pattern->addAction(tr("S&hrink"));
	QObject::connect(shrink, &QAction::triggered, this, &PatternEditorPanel::onShrinkPressed);
	pattern->addSeparator();
	auto transpose = new QMenu(tr("&Transpose"));
	pattern->addMenu(transpose);
	QAction* deNote = transpose->addAction(tr("&Decrease Note"));
	QObject::connect(deNote, &QAction::triggered, this, [&]() { onTransposePressed(false, false); });
	QAction* inNote = transpose->addAction(tr("&Increase Note"));
	QObject::connect(inNote, &QAction::triggered, this, [&]() { onTransposePressed(false, true); });
	QAction* deOct = transpose->addAction(tr("D&ecrease Octave"));
	QObject::connect(deOct, &QAction::triggered, this, [&]() { onTransposePressed(true, false); });
	QAction* inOct = transpose->addAction(tr("I&ncrease Octave"));
	QObject::connect(inOct, &QAction::triggered, this, [&]() { onTransposePressed(true, true); });
	menu.addSeparator();
	QAction* toggle = menu.addAction(tr("To&ggle Track"));
	QObject::connect(toggle, &QAction::triggered, this, [&] { onToggleTrackPressed(pos.track); });
	QAction* solo = menu.addAction(tr("&Solo Track"));
	QObject::connect(solo, &QAction::triggered, this, [&] { onSoloTrackPressed(pos.track); });
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
	undo->setShortcutVisibleInContextMenu(true);
	redo->setShortcutVisibleInContextMenu(true);
	copy->setShortcutVisibleInContextMenu(true);
	cut->setShortcutVisibleInContextMenu(true);
	paste->setShortcutVisibleInContextMenu(true);
	pasteMix->setShortcutVisibleInContextMenu(true);
	erase->setShortcutVisibleInContextMenu(true);
	select->setShortcutVisibleInContextMenu(true);
	interpolate->setShortcutVisibleInContextMenu(true);
	reverse->setShortcutVisibleInContextMenu(true);
	replace->setShortcutVisibleInContextMenu(true);
	deNote->setShortcutVisibleInContextMenu(true);
	inNote->setShortcutVisibleInContextMenu(true);
	deOct->setShortcutVisibleInContextMenu(true);
	inOct->setShortcutVisibleInContextMenu(true);
	toggle->setShortcutVisibleInContextMenu(true);
	solo->setShortcutVisibleInContextMenu(true);
#endif
	undo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
	redo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
	undo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
	copy->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
	cut->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
	paste->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
	pasteMix->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
	erase->setShortcut(QKeySequence(Qt::Key_Delete));
	select->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
	interpolate->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	reverse->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
	replace->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
	deNote->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F1));
	inNote->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F2));
	deOct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F3));
	inOct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4));
	toggle->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F9));
	solo->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F10));

	if (bt_->isJamMode() || pos.order < 0 || pos.track < 0) {
		copy->setEnabled(false);
		cut->setEnabled(false);
		paste->setEnabled(false);
		pasteMix->setEnabled(false);
		pasteOver->setEnabled(false);
		erase->setEnabled(false);
		interpolate->setEnabled(false);
		reverse->setEnabled(false);
		replace->setEnabled(false);
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
			pasteOver->setEnabled(false);
		}
		if (selRightBelowPos_.order < 0
				|| !isSelectedCell(pos.track, pos.colInTrack,
								   pos.order, pos.step)) {
			copy->setEnabled(false);
			cut->setEnabled(false);
			erase->setEnabled(false);
			interpolate->setEnabled(false);
			reverse->setEnabled(false);
			replace->setEnabled(false);
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
	if (pos.track < 0) {
		toggle->setEnabled(false);
		solo->setEnabled(false);
	}

	menu.exec(mapToGlobal(point));
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
	int end = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order));
	if (curPos_.step >= end) curPos_.step = end - 1;

	update();
}

void PatternEditorPanel::setPatternHighlight1Count(int count)
{
	hl1Cnt_ = count;
	update();
}

void PatternEditorPanel::setPatternHighlight2Count(int count)
{
	hl2Cnt_ = count;
	update();
}

void PatternEditorPanel::setEditableStep(int n)
{
	editableStepCnt_ = n;
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
	switch (songStyle_.type) {
	case SongType::STD:		rightEffn_ = std::vector<int>(15);	break;
	case SongType::FMEX:	rightEffn_ = std::vector<int>(18);	break;
	}
	TracksWidthFromLeftToEnd_
			= calculateTracksWidthWithRowNum(0, static_cast<int>(songStyle_.trackAttribs.size()) - 1);

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
			if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
			break;
		case 1:
			bt_->eraseStepInstrument(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new EraseInstrumentInStepQtCommand(this));
			if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
			break;
		case 2:
			bt_->eraseStepVolume(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new EraseVolumeInStepQtCommand(this));
			if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
			break;
		case 3:
		case 5:
		case 7:
		case 9:
			bt_->eraseStepEffect(curSongNum_, curPos_.track, curPos_.order, curPos_.step,
								 (curPos_.colInTrack - 3) / 2);
			comStack_.lock()->push(new EraseEffectInStepQtCommand(this));
			if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
			break;
		case 4:
		case 6:
		case 8:
		case 10:
			bt_->eraseStepEffectValue(curSongNum_, curPos_.track, curPos_.order, curPos_.step,
									  (curPos_.colInTrack - 4) / 2);
			comStack_.lock()->push(new EraseEffectValueInStepQtCommand(this));
			if (!bt_->isPlaySong()) moveCursorToDown(editableStepCnt_);
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

void PatternEditorPanel::onPasteOverwritePressed()
{
	if (!bt_->isPlaySong()) pasteOverwriteCopiedCells(curPos_);
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
		int max = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order)) - 1;
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
	case 2:	// Row
	{
		selectAllState_ = -1;
		PatternPosition start = { 0, 0, curPos_.order, curPos_.step };
		PatternPosition end = { static_cast<int>(songStyle_.trackAttribs.size() - 1), 10,
								curPos_.order, curPos_.step };
		setSelectedRectangle(start, end);
		break;
	}
	case 3:	// Column
	{
		selectAllState_ = -1;
		PatternPosition start = { curPos_.track, curPos_.colInTrack, curPos_.order, 0 };
		PatternPosition end = {
			curPos_.track,
			curPos_.colInTrack,
			curPos_.order,
			static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - 1)
		};
		setSelectedRectangle(start, end);
		break;
	}
	case 4:	// Pattern
	{
		selectAllState_ = -1;
		PatternPosition start = { curPos_.track, 0, curPos_.order, 0 };
		PatternPosition end = {
			curPos_.track,
			10,
			curPos_.order,
			static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - 1)
		};
		setSelectedRectangle(start, end);
		break;
	}
	case 5:	// Order
	{
		selectAllState_ = -1;
		PatternPosition start = { 0, 0, curPos_.order, 0 };
		PatternPosition end = {
			static_cast<int>(songStyle_.trackAttribs.size() - 1),
			10,
			curPos_.order,
			static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - 1)
		};
		setSelectedRectangle(start, end);
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

void PatternEditorPanel::onToggleTrackPressed(int track)
{
	bt_->setTrackMuteState(track, !bt_->isMute(track));
	isMuteElse_ = false;
	update();
}

void PatternEditorPanel::onSoloTrackPressed(int track)
{
	int trackCnt = static_cast<int>(songStyle_.trackAttribs.size());
	if (isMuteElse_) {
		if (bt_->isMute(track)) {
			for (int t = 0; t < trackCnt; ++t)
				bt_->setTrackMuteState(t, (t == track) ? false : true);
		}
		else {
			isMuteElse_ = false;
			for (int t = 0; t < trackCnt; ++t)
				bt_->setTrackMuteState(t, false);
		}
	}
	else {
		isMuteElse_ = true;
		for (int t = 0; t < trackCnt; ++t)
			bt_->setTrackMuteState(t, (t == track) ? false : isMuteElse_);
	}
	update();
}

void PatternEditorPanel::onUnmuteAllPressed()
{
	int trackCnt = static_cast<int>(songStyle_.trackAttribs.size());
	for (int t = 0; t < trackCnt; ++t)
		bt_->setTrackMuteState(t, false);
	isMuteElse_ = false;
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

void PatternEditorPanel::onReplaceInstrumentPressed()
{
	if (selLeftAbovePos_.order == -1) return;

	int curInst = bt_->getCurrentInstrumentNumber();
	if (curInst == -1) return;

	int beginTrack = (selLeftAbovePos_.colInTrack < 2) ? selLeftAbovePos_.track : (selLeftAbovePos_.track + 1);
	int endTrack = (selRightBelowPos_.colInTrack == 0) ? (selRightBelowPos_.track - 1) : selRightBelowPos_.track;
	if (beginTrack <= endTrack) {
		bt_->replaceInstrumentInPattern(curSongNum_,
										beginTrack, selLeftAbovePos_.order, selLeftAbovePos_.step,
										endTrack, selRightBelowPos_.step, curInst);
		comStack_.lock()->push(new ReplaceInstrumentInPatternQtCommand(this));
	}
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
	case Qt::Key_Return:
		emit returnPressed();
		return true;
	case Qt::Key_Shift:
		shiftPressedPos_ = curPos_;
		return true;
	case Qt::Key_Left:
		moveCursorToRight(-1);
		if (event->modifiers().testFlag(Qt::ShiftModifier))	setSelectedRectangle(shiftPressedPos_, curPos_);
		else onSelectPressed(0);
		return true;
	case Qt::Key_Right:
		moveCursorToRight(1);
		if (event->modifiers().testFlag(Qt::ShiftModifier))	setSelectedRectangle(shiftPressedPos_, curPos_);
		else onSelectPressed(0);
		return true;
	case Qt::Key_Up:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			if (event->modifiers().testFlag(Qt::ControlModifier)) {
				int base;
				if (curPos_.step) {
					base = curPos_.step;
				}
				else {
					base = static_cast<int>(bt_->getPatternSizeFromOrderNumber(
												curSongNum_,
												(curPos_.order) ? (curPos_.order - 1)
																: (static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1)));
				}
				moveCursorToDown((base - 1) / hl1Cnt_ * hl1Cnt_ - base);
			}
			else {
				moveCursorToDown(editableStepCnt_ ? -editableStepCnt_ : -1);
			}
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_Down:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			if (event->modifiers().testFlag(Qt::ControlModifier)) {
				int next = (curPos_.step / hl1Cnt_ + 1) * hl1Cnt_;
				int size = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order));
				if (next < size) moveCursorToDown(next - curPos_.step);
				else moveCursorToDown(size - curPos_.step);
			}
			else {
				moveCursorToDown(editableStepCnt_ ? editableStepCnt_ : 1);
			}
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_Tab:
		if (curPos_.track == static_cast<int>(songStyle_.trackAttribs.size()) - 1) {
			if (config_.lock()->getWarpCursor())
				moveCursorToRight(-calculateColNumInRow(curPos_.track, curPos_.colInTrack));
		}
		else {
			moveCursorToRight(5 + 2 * rightEffn_[static_cast<size_t>(curPos_.track)] - curPos_.colInTrack);
		}
		return true;
	case Qt::Key_Backtab:
		if (curPos_.track == 0) {
			if (config_.lock()->getWarpCursor())
				moveCursorToRight(getFullColmunSize() - 1);
		}
		else {
			moveCursorToRight(-5 - 2 * rightEffn_[static_cast<size_t>(curPos_.track) - 1] - curPos_.colInTrack);
		}
		return true;
	case Qt::Key_Home:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-curPos_.step);
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_End:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(
						static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order)) - curPos_.step - 1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_PageUp:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-static_cast<int>(config_.lock()->getPageJumpLength()));
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_PageDown:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(static_cast<int>(config_.lock()->getPageJumpLength()));
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
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
	case Qt::Key_Menu:
	{
		QPoint point = calculateCurrentCursorPosition();
		point.setX(point.x() + 24);
		point.setY(point.y() - 16);
		showPatternContextMenu(curPos_, point);
		return true;
	}
	default:
		if (!bt_->isJamMode()) {
			// Pattern edit
			if (!config_.lock()->getKeyRepetition() && event->isAutoRepeat()) return false;
			switch (curPos_.colInTrack) {
			case 0:
				return enterToneData(event);
			case 1:
				if (event->modifiers().testFlag(Qt::NoModifier)) return enterInstrumentData(event->key());
				break;
			case 2:
				if (event->modifiers().testFlag(Qt::NoModifier)) return enterVolumeData(event->key());
				break;
			case 3:
			case 5:
			case 7:
			case 9:
				if (event->modifiers().testFlag(Qt::NoModifier)) return enterEffectID(event->key());
				break;
			case 4:
			case 6:
			case 8:
			case 10:
				if (event->modifiers().testFlag(Qt::NoModifier)) return enterEffectValue(event->key());
				break;
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
		int odrSize = static_cast<int>(bt_->getOrderSize(curSongNum_));
		if (curPos_.order >= odrSize) curPos_.setRows(odrSize - 1, 0);

		drawPattern(event->rect());
	}
}

void PatternEditorPanel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowBaselineY_ = (geometry().height() + headerHeight_) / 2;
	curRowY_ = curRowBaselineY_ + stepFontLeading_ / 2 - stepFontAscend_;

	initDisplay();
}

void PatternEditorPanel::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event)

	setFocus();

	mousePressPos_ = hovPos_;
	doubleClickPos_ = mousePressPos_;
	mouseReleasePos_ = { -1, -1, -1, -1 };
	isPressedPlus_ = false;
	isPressedMinus_ = false;

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

		if (hovPos_.track >= 0 && hovPos_.order >= 0) {
			setSelectedRectangle(mousePressPos_, hovPos_);
			update();
		}

		if (event->x() < stepNumWidth_ && leftTrackNum_ > 0) {
			moveCursorToRight(-(5 + 2 * rightEffn_.at(static_cast<size_t>(leftTrackNum_) - 1)));
		}
		else if (event->x() > geometry().width() - stepNumWidth_ && hovPos_.track != -1) {
			moveCursorToRight(5 + 2 * rightEffn_.at(static_cast<size_t>(leftTrackNum_)));
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
				}
				else if (isPressedMinus_) {
					shrinkEffect(hovPos_.track);
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
		if (mousePressPos_.order == -2) {	// Header
			QMenu menu;
			// Leave Before Qt5.7.0 style due to windows xp
			QAction* toggle = menu.addAction(tr("To&ggle Track"));
			QObject::connect(toggle, &QAction::triggered, this, [&] { onToggleTrackPressed(mousePressPos_.track); });
			QAction* solo = menu.addAction(tr("&Solo Track"));
			QObject::connect(solo, &QAction::triggered, this, [&] { onSoloTrackPressed(mousePressPos_.track); });
			QAction* unmute = menu.addAction(tr("&Unmute All Tracks"));
			QObject::connect(unmute, &QAction::triggered, this, &PatternEditorPanel::onUnmuteAllPressed);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
			toggle->setShortcutVisibleInContextMenu(true);
			solo->setShortcutVisibleInContextMenu(true);
#endif
			toggle->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F9));
			solo->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F10));
			if (mousePressPos_.track < 0) {
				toggle->setEnabled(false);
				solo->setEnabled(false);
				unmute->setEnabled(false);
			}
			menu.exec(mapToGlobal(event->pos()));
		}
		else {	// Pattern
			showPatternContextMenu(mousePressPos_, event->pos());
		}

		break;
	}

	default:
		break;
	}

	mousePressPos_ = { -1, -1, -1, -1 };
	mouseReleasePos_ = { -1, -1, -1, -1 };
}

void PatternEditorPanel::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		if (doubleClickPos_.order >= 0) {
			if (!config_.lock()->getDontSelectOnDoubleClick()) {
				if (doubleClickPos_.track >= 0) {
					onSelectPressed(4);
					return;
				}
				else if (doubleClickPos_.track == -2) {
					onSelectPressed(5);
					return;
				}
			}
		}
		else if (doubleClickPos_.order == -2) {
			if (doubleClickPos_.track >= 0 && !isPressedPlus_ && !isPressedMinus_) {
				bool flag = true;
				int trackCnt = static_cast<int>(songStyle_.trackAttribs.size());
				for (int t = 0; t < trackCnt; ++t) {
					if (t != doubleClickPos_.track) flag &= bt_->isMute(t);
				}
				if (flag) onUnmuteAllPressed();
				else onSoloTrackPressed(doubleClickPos_.track);
				return;
			}
		}
	}

	// Else
	mousePressEvent(event);
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
				int endStep = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, tmpOdr));
				if (tmpStep < endStep) {
					hovPos_.setRows(tmpOdr, tmpStep);
					break;
				}
				else {
					if (tmpOdr == static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1) {
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
			tmpWidth += (toneNameWidth_ + widthSpaceDbl_);
			if (pos.x() <= tmpWidth) {
				hovPos_.setCols(i, 0);
				break;
			}
			tmpWidth += (instWidth_ + widthSpaceDbl_);
			if (pos.x() <= tmpWidth) {
				hovPos_.setCols(i, 1);
				break;
			}
			tmpWidth += (volWidth_ + widthSpaceDbl_);
			if (pos.x() <= tmpWidth) {
				hovPos_.setCols(i, 2);
				break;
			}
			bool flag = false;
			for (int j = 0; j <= rightEffn_.at(static_cast<size_t>(i)); ++j) {
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

			if (i == static_cast<int>(songStyle_.trackAttribs.size())) {
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

void PatternEditorPanel::midiThreadReceivedEvent(double delay, const uint8_t *msg, size_t len, void *userData)
{
	PatternEditorPanel *self = reinterpret_cast<PatternEditorPanel *>(userData);

	Q_UNUSED(delay);

	// Note-On/Note-Off
	if (len == 3 && (msg[0] & 0xe0) == 0x80) {
		uint8_t status = msg[0];
		uint8_t key = msg[1];
		uint8_t velocity = msg[2];
		QMetaMethod method = self->metaObject()->method(self->midiKeyEventMethod_);
		method.invoke(self, Qt::QueuedConnection,
					  Q_ARG(uchar, status), Q_ARG(uchar, key), Q_ARG(uchar, velocity));
	}
}

void PatternEditorPanel::midiKeyEvent(uchar status, uchar key, uchar velocity)
{
	if (!bt_->isJamMode()) {
		bool release = ((status & 0xf0) == 0x80) || velocity == 0;
		if (!release) {
			std::pair<int, Note> octaveAndNote = noteNumberToOctaveAndNote(static_cast<int>(key) - 12);
			setStepKeyOn(octaveAndNote.second, octaveAndNote.first);
		}
	}
}
