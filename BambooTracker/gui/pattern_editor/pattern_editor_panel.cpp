#include "pattern_editor_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QPoint>
#include <QApplication>
#include <algorithm>
#include "gui/event_guard.hpp"
#include "gui/command/pattern/pattern_commands_qt.hpp"

PatternEditorPanel::PatternEditorPanel(QWidget *parent)
	: QWidget(parent),
	  leftTrackNum_(0),
	  curSongNum_(0),
	  curPos_{ 0, 0, 0, 0, },
	  hovPos_{ -1, -1, -1, -1 },
	  editPos_{ -1, -1, -1, -1 },
	  isIgnoreToSlider_(false),
	  isIgnoreToOrder_(false),
	  entryCnt_(0)
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

	/* Width & height */
	widthSpace_ = stepFontWidth_ / 2;
	stepNumWidth_ = stepFontWidth_ * 2 + widthSpace_;
	toneNameWidth_ = stepFontWidth_ * 3;
	instWidth_ = stepFontWidth_ * 2;
	volWidth_ = stepFontWidth_ * 2;
	effWidth_ = stepFontWidth_ * 3;
	trackWidth_ = toneNameWidth_ + instWidth_ + volWidth_ + effWidth_ + stepFontWidth_ * 4;
	headerHeight_ = stepFontHeight_ * 2;

	/* Color */
	defTextColor_ = QColor::fromRgb(180, 180, 180);
	defRowColor_ = QColor::fromRgb(0, 0, 40);
	mkRowColor_ = QColor::fromRgb(40, 40, 80);
	curTextColor_ = QColor::fromRgb(255, 255, 255);
	curRowColor_ = QColor::fromRgb(110, 90, 140);
	curRowColorEditable_ = QColor::fromRgb(140, 90, 110);
	curCellColor_ = QColor::fromRgb(255, 255, 255, 127);
	selTextColor_ = defTextColor_;
	selCellColor_ = QColor::fromRgb(100, 100, 200);
	hovCellColor_ = QColor::fromRgb(255, 255, 255, 64);
	defStepNumColor_ = QColor::fromRgb(255, 200, 180);
	mkStepNumColor_ = QColor::fromRgb(255, 140, 160);
	toneColor_ = QColor::fromRgb(210, 230, 64);
	instColor_ = QColor::fromRgb(82, 179, 217);
	volColor_ = QColor::fromRgb(246, 36, 89);
	effColor_ = QColor::fromRgb(42, 187, 155);
	headerTextColor_ = QColor::fromRgb(240, 240, 200);
	headerRowColor_ = QColor::fromRgb(60, 60, 60);
	patternMaskColor_ = QColor::fromRgb(0, 0, 0, 128);
	borderColor_ = QColor::fromRgb(120, 120, 120);


	initDisplay();

	setAttribute(Qt::WA_Hover);
}

void PatternEditorPanel::initDisplay()
{
	pixmap_ = std::make_unique<QPixmap>(geometry().size());
}

void PatternEditorPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
	curSongNum_ = bt_->getCurrentSongNumber();
	curPos_ = PatternPosition{ 0, 0, bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber() };
	modStyle_ = bt_->getModuleStyle();
	TracksWidthFromLeftToEnd_ = calculateTracksWidthWithRowNum(0, modStyle_.trackAttribs.size() - 1);
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
	int mkCnt = 8;	// dummy set

	/* Current row */
	// Fill row
	painter.fillRect(0, curRowY_, maxWidth, stepFontHeight_,
					 bt_->isJamMode() ? curRowColor_ : curRowColorEditable_);
	// Step number
	painter.setPen((curPos_.step % mkCnt) ? defStepNumColor_ : mkStepNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curPos_.step, 2, 16, QChar('0')).toUpper());
	// Step data
	for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
		x += drawStep(painter, trackNum, curPos_.order, curPos_.step, x, curRowBaselineY_, curRowY_);
		++trackNum;
	}

	int stepNum, odrNum;
	int rowY, baseY;

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

		// Fill row
		painter.fillRect(0, rowY, maxWidth, stepFontHeight_, (stepNum % mkCnt) ? defRowColor_ : mkRowColor_);
		// Step number
		painter.setPen((stepNum % mkCnt) ? defStepNumColor_ : mkStepNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(stepNum, 2, 16, QChar('0')).toUpper());
		// Step data
		painter.setPen(defTextColor_);
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			x += drawStep(painter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (odrNum != curPos_.order)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, patternMaskColor_);
	}

	int stepEnd = bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order);

	/* Next rows */
	for (rowY = curRowY_ + stepFontHeight_, baseY = curRowBaselineY_ + stepFontHeight_,
		 stepNum = curPos_.step + 1, odrNum = curPos_.order;
		 rowY <= geometry().height();
		 rowY += stepFontHeight_, baseY += stepFontHeight_, ++stepNum) {
		if (stepNum == stepEnd) {
			if (odrNum == bt_->getOrderList(curSongNum_, curPos_.track).size() - 1) {
				break;
			}
			else {
				++odrNum;
				stepNum = 0;
				stepEnd = bt_->getPatternSizeFromOrderNumber(curSongNum_, odrNum);
			}
		}

		// Fill row
		painter.fillRect(0, rowY, maxWidth, stepFontHeight_, (stepNum % mkCnt) ? defRowColor_ : mkRowColor_);
		// Step number
		painter.setPen((stepNum % mkCnt) ? defStepNumColor_ : mkStepNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(stepNum, 2, 16, QChar('0')).toUpper());
		// Step data
		painter.setPen(defTextColor_);
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			x += drawStep(painter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (odrNum != curPos_.order)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, patternMaskColor_);
	}
}

int PatternEditorPanel::drawStep(QPainter &painter, int trackNum, int orderNum, int stepNum, int x, int baseY, int rowY)
{
	int offset = x + widthSpace_;
	PatternPosition pos{ trackNum, 0, orderNum, stepNum };
	QColor textColor = (stepNum == curPos_.step) ? curTextColor_ : defTextColor_;

	switch (modStyle_.trackAttribs[trackNum].source) {
	case SoundSource::FM:
	case SoundSource::SSG:
		/* Tone name */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (pos == hovPos_)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
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
		offset += toneNameWidth_ +  stepFontWidth_;
		pos.colInTrack = 1;

		/* Instrument */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (pos == hovPos_)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
		int instNum = bt_->getStepInstrument(curSongNum_, trackNum, orderNum, stepNum);
		if (instNum == -1) {
			painter.setPen(textColor);
			painter.drawText(offset, baseY, "--");
		}
		else {
			painter.setPen(instColor_);
			painter.drawText(offset, baseY, QString("%1").arg(instNum, 2, 16, QChar('0')).toUpper());
		}
		offset += instWidth_ +  stepFontWidth_;
		pos.colInTrack = 2;

		/* Volume */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, volWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (pos == hovPos_)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, volWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
		int vol = bt_->getStepVolume(curSongNum_, trackNum, orderNum, stepNum);
		if (vol == -1) {
			painter.setPen(textColor);
			painter.drawText(offset, baseY, "--");
		}
		else {
			painter.setPen(volColor_);
			painter.drawText(offset, baseY, QString("%1").arg(vol, 2, 16, QChar('0')).toUpper());
		}
		offset += volWidth_ +  stepFontWidth_;
		pos.colInTrack = 3;

		/* Effect */
		if (pos == curPos_)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, effWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (pos == hovPos_)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, effWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
		auto tmpStr = bt_->getStepEffectString(curSongNum_, trackNum, orderNum, stepNum);
		QString effStr = QString::fromUtf8(tmpStr.c_str(), tmpStr.length());
		painter.setPen((effStr == "---") ? textColor : effColor_);
		painter.drawText(offset, baseY, effStr);

		return trackWidth_;
	}
}

void PatternEditorPanel::drawHeaders(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, headerRowColor_);
	painter.setPen(headerTextColor_);
	int x, trackNum;
	for (x = stepNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
		QString str;
		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:	str = " FM";	break;
		case SoundSource::SSG:	str = " SSG";	break;
		}
		painter.drawText(x,
						 stepFontLeading_ + stepFontAscend_,
						 str + QString::number(modStyle_.trackAttribs[trackNum].channelInSource + 1));

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			x += trackWidth_;
			break;
		}
		++trackNum;
	}
}

void PatternEditorPanel::drawBorders(int maxWidth)
{
	QPainter painter(pixmap_.get());

	painter.drawLine(0, headerHeight_, geometry().width(), headerHeight_);
	painter.drawLine(stepNumWidth_, 0, stepNumWidth_, geometry().height());
	int x, trackNum;
	for (x = stepNumWidth_ + trackWidth_, trackNum = leftTrackNum_; x <= maxWidth; ) {
		painter.drawLine(x, 0, x, geometry().height());

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			x += trackWidth_;
			break;
		}
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
		switch (modStyle_.trackAttribs.at(i).source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			width +=  trackWidth_;
			break;
		}
	}
	return width;
}

void PatternEditorPanel::moveCursorToRight(int n)
{
	int oldTrackNum = curPos_.track;

	curPos_.colInTrack += n;
	if (n > 0) {
		for (bool flag = true; flag; ) {
			switch (modStyle_.trackAttribs[curPos_.track].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				if (curPos_.colInTrack < 4) {
					flag = false;
				}
				else if (curPos_.track == modStyle_.trackAttribs.size() - 1) {
					curPos_.colInTrack = 3;
					flag = false;
				}
				else {
					curPos_.colInTrack -= 4;
					++curPos_.track;
				}
				break;
			}
		}
		while (calculateTracksWidthWithRowNum(leftTrackNum_, curPos_.track) > geometry().width())
			++leftTrackNum_;
	}
	else {
		for (bool flag = true; flag; ) {
			if (curPos_.colInTrack >= 0) {
				flag = false;
			}
			else if (!curPos_.track) {
				curPos_.colInTrack = 0;
				flag = false;
			}
			else {
				--curPos_.track;
				switch (modStyle_.trackAttribs[curPos_.track].source) {
				case SoundSource::FM:
				case SoundSource::SSG:
					curPos_.colInTrack += 4;
					break;
				}
			}
		}
		if (curPos_.track < leftTrackNum_) leftTrackNum_ = curPos_.track;
	}

	TracksWidthFromLeftToEnd_
			= calculateTracksWidthWithRowNum(leftTrackNum_, modStyle_.trackAttribs.size() - 1);

	if (curPos_.track != oldTrackNum)
		bt_->setCurrentTrack(curPos_.track);

	if (!isIgnoreToSlider_)
		emit currentCellInRowChanged(calculateCellNumInRow(curPos_.track, curPos_.colInTrack));

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
				if (curPos_.order == bt_->getOrderList(curSongNum_, curPos_.track).size() - 1) {
					curPos_.step = tmp - dif - 1;	// Last step
					break;
				}
				else {
					++curPos_.order;
					tmp = dif;
				}
			}
		}
	}
	else {
		while (true) {
			if (tmp < 0) {
				if (curPos_.order == 0) {
					curPos_.step = 0;
					break;
				}
				else {
					tmp += bt_->getPatternSizeFromOrderNumber(curSongNum_, --curPos_.order);
				}
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
		emit currentOrderChanged(curPos_.order);

	update();
}

int PatternEditorPanel::calculateCellNumInRow(int trackNum, int cellNumInTrack) const
{
	int i, cnt = 0;
	for (i = 0; i < trackNum; ++i) {
		switch (modStyle_.trackAttribs[i].source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			cnt += 4;
			break;
		}
	}
	cnt += cellNumInTrack;

	return cnt;
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
	switch (modStyle_.trackAttribs.back().source) {
	case SoundSource::FM:
	case SoundSource::SSG:
		return calculateCellNumInRow(modStyle_.trackAttribs.size() - 1, 3);
	}
}

void PatternEditorPanel::updatePosition()
{
	curPos_.setRows(bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());

	emit currentOrderChanged(curPos_.order);
	emit currentStepChanged(curPos_.step, bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order) - 1);

	update();
}

void PatternEditorPanel::enterToneDataFMSSG(int key)
{
	int baseOct = bt_->getCurrentOctave();

	switch (key) {
	case Qt::Key_Z:			setStepKeyOn(Note::C, baseOct);			break;
	case Qt::Key_S:			setStepKeyOn(Note::CS, baseOct);		break;
	case Qt::Key_X:			setStepKeyOn(Note::D, baseOct);			break;
	case Qt::Key_D:			setStepKeyOn(Note::DS, baseOct);		break;
	case Qt::Key_C:			setStepKeyOn(Note::E, baseOct);			break;
	case Qt::Key_V:			setStepKeyOn(Note::F, baseOct);			break;
	case Qt::Key_G:			setStepKeyOn(Note::FS, baseOct);		break;
	case Qt::Key_B:			setStepKeyOn(Note::G, baseOct);			break;
	case Qt::Key_H:			setStepKeyOn(Note::GS, baseOct);		break;
	case Qt::Key_N:			setStepKeyOn(Note::A, baseOct);			break;
	case Qt::Key_J:			setStepKeyOn(Note::AS, baseOct);		break;
	case Qt::Key_M:			setStepKeyOn(Note::B, baseOct);			break;
	case Qt::Key_Comma:		setStepKeyOn(Note::C, baseOct + 1);		break;
	case Qt::Key_L:			setStepKeyOn(Note::CS, baseOct + 1);	break;
	case Qt::Key_Period:	setStepKeyOn(Note::D, baseOct + 1);		break;
	case Qt::Key_Q:			setStepKeyOn(Note::C, baseOct + 1);		break;
	case Qt::Key_2:			setStepKeyOn(Note::CS, baseOct + 1);	break;
	case Qt::Key_W:			setStepKeyOn(Note::D, baseOct + 1);		break;
	case Qt::Key_3:			setStepKeyOn(Note::DS, baseOct + 1);	break;
	case Qt::Key_E:			setStepKeyOn(Note::E, baseOct + 1);		break;
	case Qt::Key_R:			setStepKeyOn(Note::F, baseOct + 1);		break;
	case Qt::Key_5:			setStepKeyOn(Note::FS, baseOct + 1);	break;
	case Qt::Key_T:			setStepKeyOn(Note::G, baseOct + 1);		break;
	case Qt::Key_6:			setStepKeyOn(Note::GS, baseOct + 1);	break;
	case Qt::Key_Y:			setStepKeyOn(Note::A, baseOct + 1);		break;
	case Qt::Key_7:			setStepKeyOn(Note::AS, baseOct + 1);	break;
	case Qt::Key_U:			setStepKeyOn(Note::B, baseOct + 1);		break;
	case Qt::Key_I:			setStepKeyOn(Note::C, baseOct + 2);		break;
	case Qt::Key_9:			setStepKeyOn(Note::CS, baseOct + 2);	break;
	case Qt::Key_O:			setStepKeyOn(Note::D, baseOct + 2);		break;
	case Qt::Key_Minus:
		bt_->setStepKeyOff(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
		comStack_.lock()->push(new SetKeyOffToStepQtCommand(this));
		moveCursorToDown(1);
		break;
	case Qt::Key_Backspace:
		bt_->eraseStepNote(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
		comStack_.lock()->push(new EraseNoteInStepQtCommand(this));
		moveCursorToDown(1);
		break;
	default:
		break;
	}
}

void PatternEditorPanel::setStepKeyOn(Note note, int octave)
{
	if (octave < 7) {
		bt_->setStepNote(curSongNum_, curPos_.track, curPos_.order, curPos_.step, octave, note);
		comStack_.lock()->push(new SetKeyOnToStepQtCommand(this));
		moveCursorToDown(1);
	}
}

bool PatternEditorPanel::enterInstrumentDataFMSSG(int key)
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
	case Qt::Key_Backspace:
		bt_->eraseStepInstrument(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
		comStack_.lock()->push(new EraseInstrumentInStepQtCommand(this));
		moveCursorToDown(1);
		return false;
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

/********** Slots **********/
void PatternEditorPanel::setCurrentCellInRow(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - calculateCellNumInRow(curPos_.track, curPos_.colInTrack))
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

	int dif = calculateCellNumInRow(num, 0)
			  - calculateCellNumInRow(curPos_.track, curPos_.colInTrack);
	moveCursorToRight(dif);
}

void PatternEditorPanel::setCurrentOrder(int num)
{
	Ui::EventGuard eg(isIgnoreToOrder_);

	int dif = calculateStepDistance(curPos_.order, curPos_.step, num, 0);
	moveCursorToDown(dif);
}

/********** Events **********/
bool PatternEditorPanel::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::KeyPress:
		return keyPressed(dynamic_cast<QKeyEvent*>(event));
	case QEvent::HoverMove:
		return mouseHoverd(dynamic_cast<QHoverEvent*>(event));
	default:
		return QWidget::event(event);
	}
}

bool PatternEditorPanel::keyPressed(QKeyEvent *event)
{
	/* General Keys (with Ctrl) */
	if (event->modifiers().testFlag(Qt::ControlModifier)) return false;

	/* General Keys */
	switch (event->key()) {
	case Qt::Key_Left:
		moveCursorToRight(-1);
		return true;
	case Qt::Key_Right:
		moveCursorToRight(1);
		return true;
	case Qt::Key_Up:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-1);
			return true;
		}
	case Qt::Key_Down:
		if (bt_->isPlaySong()) {
			return  false;
		}
		else {
			moveCursorToDown(1);
			return true;
		}
	default:
		if (!bt_->isJamMode()) {
			// Pattern edit
			switch (modStyle_.trackAttribs[curPos_.track].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				switch (curPos_.colInTrack) {
				case 0:
					enterToneDataFMSSG(event->key());
					return false;
				case 1:
					return enterInstrumentDataFMSSG(event->key());
				case 2:
					// UNDONE: volume number
					break;
				case 3:
					// UNDONE: effect
					break;
				}
				break;;
			}
		}
		return false;
	}
}

void PatternEditorPanel::paintEvent(QPaintEvent *event)
{
	if (bt_ != nullptr) drawPattern(event->rect());
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

	if (hovPos_.order >= 0 && hovPos_.step >= 0
			&& hovPos_.track >= 0 && hovPos_.colInTrack >= 0) {
		switch (event->button()) {
		case Qt::LeftButton:
		{
			int horDif = calculateCellNumInRow(hovPos_.track, hovPos_.colInTrack)
						 - calculateCellNumInRow(curPos_.track, curPos_.colInTrack);
			int verDif = calculateStepDistance(curPos_.order, curPos_.step, hovPos_.order, hovPos_.step);
			moveCursorToRight(horDif);
			moveCursorToDown(verDif);
			update();
			break;
		}
		default: break;
		}
	}
}

bool PatternEditorPanel::mouseHoverd(QHoverEvent *event)
{
	QPoint pos = event->pos();
	PatternPosition oldPos = hovPos_;

	// Detect Step
	if (pos.y() <= headerHeight_) {
		// Track header
		hovPos_.setRows(-1, -1);
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
					if (tmpOdr == bt_->getOrderList(curSongNum_, 0).size() - 1) {
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
		hovPos_.setCols(-1, -1);
	}
	else {
		int flag = true;
		int tmpWidth = stepNumWidth_;
		for (int i = leftTrackNum_; flag; ) {
			switch (modStyle_.trackAttribs[i].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				tmpWidth += (toneNameWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovPos_.setCols(i, 0);
					flag = false;
					break;
				}
				tmpWidth += (instWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovPos_.setCols(i, 1);
					flag = false;
					break;
				}
				tmpWidth += (volWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovPos_.setCols(i, 2);
					flag = false;
					break;
				}
				tmpWidth += (effWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovPos_.setCols(i, 3);
					flag = false;
					break;
				}
				++i;
				break;
			}

			if (i == modStyle_.trackAttribs.size()) {
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
