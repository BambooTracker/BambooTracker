#include "pattern_editor_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QPoint>
#include <QApplication>
#include <algorithm>
#include "gui/event_guard.hpp"

PatternEditorPanel::PatternEditorPanel(QWidget *parent)
	: QWidget(parent),
	  leftTrackNum_(0),
	  curSongNum_(0),
	  curTrackNum_(0),
	  curCellNumInTrack_(0),
	  curOrderNum_(0),
	  curStepNum_(0),
	  hovTrackNum_(-1),
	  hovCellNumInTrack_(-1),
	  hovOrderNum_(-1),
	  hovStepNum_(-1),
	  isIgnoreToSlider_(false),
	  isIgnoreToOrder_(false)
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
	toneColor_ = QColor::fromRgb(0, 230, 64);
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
	curTrackNum_ = bt_->getCurrentStepNumber();
	curStepNum_ = bt_->getCurrentStepNumber();
	curOrderNum_ = bt_->getCurrentOrderNumber();
	modStyle_ = bt_->getModuleStyle();
	TracksWidthFromLeftToEnd_ = calculateTracksWidthWithRowNum(0, modStyle_.trackAttribs.size() - 1);
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
	painter.setPen((curStepNum_ % mkCnt) ? defStepNumColor_ : mkStepNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curStepNum_, 2, 16, QChar('0')).toUpper());
	// Step data
	for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
		x += drawStep(painter, trackNum, curOrderNum_, curStepNum_, x, curRowBaselineY_, curRowY_);
		++trackNum;
	}

	int stepNum, odrNum;
	int rowY, baseY;

	/* Previous rows */
	for (rowY = curRowY_ - stepFontHeight_, baseY = curRowBaselineY_ - stepFontHeight_,
		 stepNum = curStepNum_ - 1, odrNum = curOrderNum_;
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
		if (odrNum != curOrderNum_)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, patternMaskColor_);
	}

	int stepEnd = bt_->getPatternSizeFromOrderNumber(curSongNum_, curOrderNum_);

	/* Next rows */
	for (rowY = curRowY_ + stepFontHeight_, baseY = curRowBaselineY_ + stepFontHeight_,
		 stepNum = curStepNum_ + 1, odrNum = curOrderNum_;
		 rowY <= geometry().height();
		 rowY += stepFontHeight_, baseY += stepFontHeight_, ++stepNum) {
		if (stepNum == stepEnd) {
			if (odrNum == bt_->getOrderList(curSongNum_, curTrackNum_).size() - 1) {
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
		if (odrNum != curOrderNum_)	// Mask
			painter.fillRect(0, rowY, maxWidth, stepFontHeight_, patternMaskColor_);
	}
}

int PatternEditorPanel::drawStep(QPainter &painter, int trackNum, int orderNum, int stepNum, int x, int baseY, int rowY)
{
	int offset = x + widthSpace_;
	QColor textColor = (stepNum == curStepNum_) ? curTextColor_ : defTextColor_;

	switch (modStyle_.trackAttribs[trackNum].source) {
	case SoundSource::FM:
	case SoundSource::PSG:
		/* Tone name */
		if (trackNum == curTrackNum_ && stepNum == curStepNum_ && curCellNumInTrack_ == 0)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (orderNum == hovOrderNum_ && stepNum == hovStepNum_
				&& trackNum == hovTrackNum_ && hovCellNumInTrack_ == 0)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
		int noteNum = bt_->getStepNoteNumber(curSongNum_, trackNum, orderNum, stepNum);
		switch (noteNum) {
		case -1:	// None
			painter.setPen(textColor);
			painter.drawText(offset, baseY, "---");
			break;
		case -2:	// Key on
			painter.setPen(toneColor_);
			painter.drawRect(offset, rowY + stepFontHeight_ * 2 / 5,  toneNameWidth_, stepFontHeight_ / 5);
			break;
		case -3:	// Key off
			painter.fillRect(offset, rowY + stepFontHeight_ * 2 / 5,
							 toneNameWidth_, stepFontHeight_ / 5, toneColor_);
			break;
		case -4:	// Key release
			painter.setPen(toneColor_);
			painter.drawLine(offset, rowY + stepFontHeight_ * 2 / 5,
							 offset + toneNameWidth_, rowY + stepFontHeight_ * 2 / 5);
			painter.drawLine(offset, rowY + stepFontHeight_ * 3 / 5,
							 offset + toneNameWidth_, rowY + stepFontHeight_ * 3 / 5);
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

		/* Instrument */
		if (trackNum == curTrackNum_ && stepNum == curStepNum_ && curCellNumInTrack_ == 1)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (orderNum == hovOrderNum_ && stepNum == hovStepNum_
				&& trackNum == hovTrackNum_ && hovCellNumInTrack_ == 1)	// Paint hover
			painter.fillRect(offset - widthSpace_, rowY, instWidth_ + stepFontWidth_, stepFontHeight_, hovCellColor_);
		int instNum = bt_->getStepInstrumentNumber(curSongNum_, trackNum, orderNum, stepNum);
		if (instNum == -1) {
			painter.setPen(textColor);
			painter.drawText(offset, baseY, "--");
		}
		else {
			painter.setPen(instColor_);
			painter.drawText(offset, baseY, QString("%1").arg(instNum, 2, 16, QChar('0')).toUpper());
		}
		offset += instWidth_ +  stepFontWidth_;

		/* Volume */
		if (trackNum == curTrackNum_ && stepNum == curStepNum_ && curCellNumInTrack_ == 2)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, volWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (orderNum == hovOrderNum_ && stepNum == hovStepNum_
				&& trackNum == hovTrackNum_ && hovCellNumInTrack_ == 2)	// Paint hover
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

		/* Effect */
		if (trackNum == curTrackNum_ && stepNum == curStepNum_ && curCellNumInTrack_ == 3)	// Paint current cell
			painter.fillRect(offset - widthSpace_, rowY, effWidth_ + stepFontWidth_, stepFontHeight_, curCellColor_);
		if (orderNum == hovOrderNum_ && stepNum == hovStepNum_
				&& trackNum == hovTrackNum_ && hovCellNumInTrack_ == 3)	// Paint hover
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
		case SoundSource::PSG:	str = " PSG";	break;
		}
		painter.drawText(x,
						 stepFontLeading_ + stepFontAscend_,
						 str + QString::number(modStyle_.trackAttribs[trackNum].channelInSource + 1));

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::PSG:
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
		case SoundSource::PSG:
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
		case SoundSource::PSG:
			width +=  trackWidth_;
			break;
		}
	}
	return width;
}

void PatternEditorPanel::moveCursorToRight(int n)
{
	int oldTrackNum = curTrackNum_;

	curCellNumInTrack_ += n;
	if (n > 0) {
		for (bool flag = true; flag; ) {
			switch (modStyle_.trackAttribs[curTrackNum_].source) {
			case SoundSource::FM:
			case SoundSource::PSG:
				if (curCellNumInTrack_ < 4) {
					flag = false;
				}
				else if (curTrackNum_ == modStyle_.trackAttribs.size() - 1) {
					curCellNumInTrack_ = 3;
					flag = false;
				}
				else {
					curCellNumInTrack_ -= 4;
					++curTrackNum_;
				}
				break;
			}
		}
		while (calculateTracksWidthWithRowNum(leftTrackNum_, curTrackNum_) > geometry().width())
			++leftTrackNum_;
	}
	else {
		for (bool flag = true; flag; ) {
			if (curCellNumInTrack_ >= 0) {
				flag = false;
			}
			else if (!curTrackNum_) {
				curCellNumInTrack_ = 0;
				flag = false;
			}
			else {
				--curTrackNum_;
				switch (modStyle_.trackAttribs[curTrackNum_].source) {
				case SoundSource::FM:
				case SoundSource::PSG:
					curCellNumInTrack_ += 4;
					break;
				}
			}
		}
		if (curTrackNum_ < leftTrackNum_) leftTrackNum_ = curTrackNum_;
	}
	TracksWidthFromLeftToEnd_
			= calculateTracksWidthWithRowNum(leftTrackNum_, modStyle_.trackAttribs.size() - 1);

	if (curTrackNum_ != oldTrackNum)
		bt_->setCurrentTrack(curTrackNum_);

	if (!isIgnoreToSlider_)
		emit currentCellInRowChanged(calculateCellNumInRow(curTrackNum_, curCellNumInTrack_));

	if (!isIgnoreToOrder_ && curTrackNum_ != oldTrackNum)	// Send to order list
		emit currentTrackChanged(curTrackNum_);

	update();
}

void PatternEditorPanel::moveCursorToDown(int n)
{
	int oldOdr = curOrderNum_;
	int tmp = curStepNum_ + n;

	if (n > 0) {
		while (true) {
			int dif = tmp - bt_->getPatternSizeFromOrderNumber(curSongNum_, curOrderNum_);
			if (dif < 0) {
				curStepNum_ = tmp;
				break;
			}
			else {
				if (curOrderNum_ == bt_->getOrderList(curSongNum_, curTrackNum_).size() - 1) {
					curStepNum_ = tmp - dif - 1;	// Last step
					break;
				}
				else {
					++curOrderNum_;
					tmp = dif;
				}
			}
		}
	}
	else {
		while (true) {
			if (tmp < 0) {
				if (curOrderNum_ == 0) {
					curStepNum_ = 0;
					break;
				}
				else {
					tmp += bt_->getPatternSizeFromOrderNumber(curSongNum_, --curOrderNum_);
				}
			}
			else {
				curStepNum_ = tmp;
				break;
			}
		}
	}

	if (curOrderNum_ != oldOdr)
		bt_->setCurrentOrderNumber(curOrderNum_);
	bt_->setCurrentStepNumber(curStepNum_);

	if (!isIgnoreToSlider_)
		emit currentStepChanged(curStepNum_, bt_->getPatternSizeFromOrderNumber(curSongNum_, curOrderNum_) - 1);

	if (!isIgnoreToOrder_ && curOrderNum_ != oldOdr)	// Send to order list
		emit currentOrderChanged(curOrderNum_);

	update();
}

int PatternEditorPanel::calculateCellNumInRow(int trackNum, int cellNumInTrack) const
{
	int i, cnt = 0;
	for (i = 0; i < trackNum; ++i) {
		switch (modStyle_.trackAttribs[i].source) {
		case SoundSource::FM:
		case SoundSource::PSG:
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
	case SoundSource::PSG:
		return calculateCellNumInRow(modStyle_.trackAttribs.size() - 1, 3);
	default: return 0;
	}
}

/********** Slots **********/
void PatternEditorPanel::setCurrentCellInRow(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - calculateCellNumInRow(curTrackNum_, curCellNumInTrack_))
		moveCursorToRight(dif);
}

void PatternEditorPanel::setCurrentStep(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - curStepNum_) moveCursorToDown(dif);
}

void PatternEditorPanel::setCurrentTrack(int num)
{
	Ui::EventGuard eg(isIgnoreToOrder_);

	int dif = calculateCellNumInRow(num, 0)
			  - calculateCellNumInRow(curTrackNum_, curCellNumInTrack_);
	moveCursorToRight(dif);
}

void PatternEditorPanel::setCurrentOrder(int num)
{
	Ui::EventGuard eg(isIgnoreToOrder_);

	int dif = calculateStepDistance(curOrderNum_, curStepNum_, num, 0);
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
	switch (event->key()) {
	case Qt::Key_Left:	moveCursorToRight(-1);	return true;
	case Qt::Key_Right:	moveCursorToRight(1);	return true;
	case Qt::Key_Up:	moveCursorToDown(-1);	return true;
	case Qt::Key_Down:	moveCursorToDown(1);	return true;
	default: return false;
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

	if (hovOrderNum_ >= 0 && hovStepNum_ >= 0
			&& hovTrackNum_ >= 0 && hovCellNumInTrack_ >= 0) {
		int horDif = calculateCellNumInRow(hovTrackNum_, hovCellNumInTrack_)
					 - calculateCellNumInRow(curTrackNum_, curCellNumInTrack_);
		int verDif = calculateStepDistance(curOrderNum_, curStepNum_, hovOrderNum_, hovStepNum_);

		moveCursorToRight(horDif);
		moveCursorToDown(verDif);

		update();
	}
}

bool PatternEditorPanel::mouseHoverd(QHoverEvent *event)
{
	QPoint pos = event->pos();

	int oldOdr = hovOrderNum_;
	int oldStep = hovStepNum_;
	int oldTrack = hovTrackNum_;
	int oldCell = hovCellNumInTrack_;

	// Detect Step
	if (pos.y() <= headerHeight_) {
		// Track header
		hovOrderNum_ = -1;
		hovStepNum_ = -1;
	}
	else {
		if (pos.y() < curRowY_) {
			int tmpOdr = curOrderNum_;
			int tmpStep = curStepNum_ +  (pos.y() - curRowY_) / stepFontHeight_ - 1;
			while (true) {
				if (tmpStep < 0) {
					if (tmpOdr == 0) {
						hovOrderNum_ = -1;
						hovStepNum_ = -1;
						break;
					}
					else {
						tmpStep += bt_->getPatternSizeFromOrderNumber(curSongNum_, --tmpOdr);
					}
				}
				else {
					hovOrderNum_ = tmpOdr;
					hovStepNum_ = tmpStep;
					break;
				}
			}
		}
		else {
			int tmpOdr = curOrderNum_;
			int tmpStep = curStepNum_ +  (pos.y() - curRowY_) / stepFontHeight_;
			while (true) {
				int endStep = bt_->getPatternSizeFromOrderNumber(curSongNum_, tmpOdr);
				if (tmpStep < endStep) {
					hovOrderNum_ = tmpOdr;
					hovStepNum_ = tmpStep;
					break;
				}
				else {
					if (tmpOdr == bt_->getOrderList(curSongNum_, 0).size() - 1) {
						hovOrderNum_ = -1;
						hovStepNum_ = -1;
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
		hovTrackNum_ = -1;
		hovCellNumInTrack_ = -1;
	}
	else {
		int flag = true;
		int tmpWidth = stepNumWidth_;
		for (hovTrackNum_ = leftTrackNum_; flag; ) {
			switch (modStyle_.trackAttribs[hovTrackNum_].source) {
			case SoundSource::FM:
			case SoundSource::PSG:
				tmpWidth += (toneNameWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovCellNumInTrack_ = 0;
					flag = false;
					break;
				}
				tmpWidth += (instWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovCellNumInTrack_ = 1;
					flag = false;
					break;
				}
				tmpWidth += (volWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovCellNumInTrack_ = 2;
					flag = false;
					break;
				}
				tmpWidth += (effWidth_ + stepFontWidth_);
				if (pos.x() <= tmpWidth) {
					hovCellNumInTrack_ = 3;
					flag = false;
					break;
				}
				++hovTrackNum_;
				break;
			}
		}
	}

	if (hovOrderNum_ != oldOdr || hovStepNum_ != oldStep
			|| hovTrackNum_ != oldTrack || hovCellNumInTrack_ != oldCell)
		update();

	return true;
}

void PatternEditorPanel::wheelEvent(QWheelEvent *event)
{
	int degree = event->angleDelta().y() / 8;
	moveCursorToDown(-degree / 15);
}
