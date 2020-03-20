#include "pattern_editor_panel.hpp"
#include <algorithm>
#include <vector>
#include <utility>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <QPainter>
#include <QFontMetrics>
#include <QFontInfo>
#include <QPoint>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMetaMethod>
#include <QIcon>
#include "gui/event_guard.hpp"
#include "gui/command/pattern/pattern_commands_qt.hpp"
#include "midi/midi.hpp"
#include "jam_manager.hpp"
#include "gui/effect_description.hpp"
#include "gui/shortcut_util.hpp"

PatternEditorPanel::PatternEditorPanel(QWidget *parent)
	: QWidget(parent),
	  config_(std::make_shared<Configuration>()),	// Dummy
	  stepFontWidth_(0),
	  stepFontHeight_(0),
	  stepFontAscent_(0),
	  stepFontLeading_(0),
	  headerFontAscent_(0),
	  widthSpace_(0),
	  widthSpaceDbl_(0),
	  stepNumWidthCnt_(0),
	  stepNumWidth_(0),
	  stepNumBase_(0),
	  baseTrackWidth_(0),
	  toneNameWidth_(0),
	  instWidth_(0),
	  volWidth_(0),
	  effWidth_(0),
	  effIDWidth_(0),
	  effValWidth_(0),
	  tracksWidthFromLeftToEnd_(0),
	  hdMuteToggleWidth_(0),
	  hdEffCompandButtonWidth_(0),
	  headerHeight_(0),
	  hdPlusY_(0),
	  hdMinusY_(0),
	  curRowBaselineY_(0),
	  curRowY_(0),
	  leftTrackNum_(0),
	  curSongNum_(0),
	  curPos_{ 0, 0, 0, 0, },
	  hovPos_{ -1, -1, -1, -1 },
	  mousePressPos_{ -1, -1, -1, -1 },
	  mouseReleasePos_{ -1, -1, -1, -1 },
	  selLeftAbovePos_{ -1, -1, -1, -1 },
	  selRightBelowPos_{ -1, -1, -1, -1 },
	  shiftPressedPos_{ -1, -1, -1, -1 },
	  doubleClickPos_{ -1, -1, -1, -1 },
	  markerPos_{ -1, -1, -1, -1 },
	  isIgnoreToSlider_(false),
	  isIgnoreToOrder_(false),
	  isPressedPlus_(false),
	  isPressedMinus_(false),
	  entryCnt_(0),
	  selectAllState_(-1),
	  isMuteElse_(false),
	  hl1Cnt_(4),
	  hl2Cnt_(16),
	  editableStepCnt_(1),
	  viewedRowCnt_(1),
	  viewedRowsHeight_(0),
	  viewedRowOffset_(0),
	  viewedCenterY_(0),
	  viewedCenterBaseY_(0),
	  backChanged_(false),
	  textChanged_(false),
	  foreChanged_(false),
	  headerChanged_(false),
	  focusChanged_(false),
	  followModeChanged_(false),
	  hasFocussedBefore_(false),
	  stepDownCount_(0),
	  freezed_(false),
	  repaintable_(true),
	  repaintingCnt_(0)
{	
	// Initialize font
	headerFont_ = QApplication::font();
	headerFont_.setPointSize(10);
	stepFont_ = QFont("Monospace", 10);
	stepFont_.setStyleHint(QFont::TypeWriter);
	stepFont_.setStyleStrategy(QFont::ForceIntegerMetrics);

	updateSizes();

	rightEffn_ = std::vector<int>(15);

	setAttribute(Qt::WA_Hover);
	setContextMenuPolicy(Qt::CustomContextMenu);

	keyOff_ = std::make_unique<QShortcut>(this);
	keyOff_->setContext(Qt::WidgetShortcut);
	QObject::connect(keyOff_.get(), &QShortcut::activated,
					 this, [&] {
		if (!bt_->isJamMode() && curPos_.colInTrack == 0) {
			bt_->setStepKeyOff(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new SetKeyOffToStepQtCommand(this));
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
		}
	});
	echoBuf_ = std::make_unique<QShortcut>(this);
	echoBuf_->setContext(Qt::WidgetShortcut);
	QObject::connect(echoBuf_.get(), &QShortcut::activated,
					 this, [&] {
		if (!bt_->isJamMode() && curPos_.colInTrack == 0) {
			int n = bt_->getCurrentOctave();
			if (n > 3) n = 3;
			bt_->setEchoBufferAccess(curSongNum_, curPos_.track, curPos_.order, curPos_.step, n);
			comStack_.lock()->push(new SetEchoBufferAccessQtCommand(this));
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
		}
	});
	onShortcutUpdated();

	midiKeyEventMethod_ = metaObject()->indexOfSlot("midiKeyEvent(uchar,uchar,uchar)");
	Q_ASSERT(midiKeyEventMethod_ != -1);
	MidiInterface::instance().installInputHandler(&midiThreadReceivedEvent, this);
}

PatternEditorPanel::~PatternEditorPanel()
{
	MidiInterface::instance().uninstallInputHandler(&midiThreadReceivedEvent, this);
}

void PatternEditorPanel::updateSizes()
{
	QFontMetrics metrics(stepFont_);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	stepFontWidth_ = metrics.horizontalAdvance('0');
#else
	stepFontWidth_ = metrics.width('0');
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
	stepFontAscent_ = metrics.capHeight();
#else
	stepFontAscent_ = metrics.boundingRect('X').height();
#endif
	stepFontLeading_ = metrics.ascent() - stepFontAscent_ + metrics.descent() / 2;
	stepFontHeight_ = stepFontAscent_ + stepFontLeading_;

	QFontMetrics m(headerFont_);
	headerFontAscent_ = m.ascent() + 2;

	/* Width & height */
	widthSpace_ = stepFontWidth_ / 5 * 2;
	widthSpaceDbl_ = widthSpace_ * 2;
	stepNumWidthCnt_ = config_->getShowRowNumberInHex() ? 2 : 3;
	if (config_->getShowRowNumberInHex()) {
		stepNumWidthCnt_ = 2;
		stepNumBase_ = 16;
	}
	else {
		stepNumWidthCnt_ = 3;
		stepNumBase_ = 10;
	}
	stepNumWidth_ = stepFontWidth_ * stepNumWidthCnt_ + widthSpace_;
	toneNameWidth_ = stepFontWidth_ * 3;
	instWidth_ = stepFontWidth_ * 2;
	volWidth_ = stepFontWidth_ * 2;
	effIDWidth_ = stepFontWidth_ * 2;
	effValWidth_ = stepFontWidth_ * 2;
	effWidth_ = effIDWidth_ + effValWidth_ + widthSpaceDbl_;
	baseTrackWidth_ = toneNameWidth_ + instWidth_ + volWidth_
					  + effIDWidth_ + effValWidth_ + widthSpaceDbl_ * 4;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	hdEffCompandButtonWidth_ = m.horizontalAdvance("+");
#else
	hdEffCompandButtonWidth_ = m.width("+");
#endif
	hdMuteToggleWidth_ = baseTrackWidth_ - hdEffCompandButtonWidth_ - stepFontWidth_ / 2 * 3;
	headerHeight_ = m.height() * 2;
	hdPlusY_ = headerHeight_ / 4 + m.lineSpacing() / 2 - m.leading() / 2 - m.descent();
	hdMinusY_ = headerHeight_ / 2 + hdPlusY_;

	initDisplay();
}

void PatternEditorPanel::initDisplay()
{
	completePixmap_ = std::make_unique<QPixmap>(geometry().size());

	int width = geometry().width();

	// Recalculate pixmap sizes
	viewedRegionHeight_ = std::max((geometry().height() - headerHeight_), stepFontHeight_);
	int cnt = viewedRegionHeight_ / stepFontHeight_;
	viewedRowCnt_ = (cnt % 2) ? (cnt + 2) : (cnt + 1);
	viewedRowsHeight_ = viewedRowCnt_ * stepFontHeight_;

	viewedRowOffset_ = (viewedRowsHeight_ - viewedRegionHeight_) >> 1;
	viewedCenterY_ = (viewedRowsHeight_ - stepFontHeight_) >> 1;
	viewedCenterBaseY_ = viewedCenterY_ + stepFontAscent_ + (stepFontLeading_ >> 1);

	backPixmap_ = std::make_unique<QPixmap>(width, viewedRowsHeight_);
	textPixmap_ = std::make_unique<QPixmap>(width, viewedRowsHeight_);
	forePixmap_ = std::make_unique<QPixmap>(width, viewedRowsHeight_);
	headerPixmap_ = std::make_unique<QPixmap>(width, headerHeight_);
}

void PatternEditorPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
}

bool PatternEditorPanel::isReadyCore() const
{
	return (bt_ != nullptr);
}

void PatternEditorPanel::setCommandStack(std::weak_ptr<QUndoStack> stack)
{
	comStack_ = stack;
}

void PatternEditorPanel::setConfiguration(std::shared_ptr<Configuration> config)
{
	config_ = config;
}

void PatternEditorPanel::setColorPallete(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
}

void PatternEditorPanel::freeze()
{
	freezed_ = true;
	while (true) {
		if (repaintingCnt_.load())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		else {
			curPos_ = { 0, 0, 0, 0 };	// Init
			return;
		}
	}
}

void PatternEditorPanel::unfreeze()
{
	freezed_ = false;
}

QString PatternEditorPanel::getHeaderFont() const
{
	return QFontInfo(headerFont_).family();
}

int PatternEditorPanel::getHeaderFontSize() const
{
	return QFontInfo(headerFont_).pointSize();
}

QString PatternEditorPanel::getRowsFont() const
{
	return QFontInfo(stepFont_).family();
}

int PatternEditorPanel::getRowsFontSize() const
{
	return QFontInfo(stepFont_).pointSize();
}

void PatternEditorPanel::setFonts(QString headerFont, int headerSize, QString rowsFont, int rowsSize)
{
	headerFont_ = QFont(headerFont, headerSize);
	stepFont_ = QFont(rowsFont, rowsSize);

	updateSizes();
	updateTracksWidthFromLeftToEnd();

	redrawAll();
}

int PatternEditorPanel::getCurrentTrack() const
{
	return curPos_.track;
}

void PatternEditorPanel::redrawByPatternChanged(bool patternSizeChanged)
{
	textChanged_ = true;

	// When pattern size is changed, redraw all area
	if (patternSizeChanged) {
		backChanged_ = true;
		foreChanged_ = true;
	}

	repaint();
}

void PatternEditorPanel::redrawByFocusChanged()
{
	if (hasFocussedBefore_) {
		focusChanged_ = true;
		repaint();
	}
	else {
		redrawAll();
		hasFocussedBefore_ = true;
	}
}

void PatternEditorPanel::redrawByHoverChanged()
{
	headerChanged_ = true;
	backChanged_ = true;
	repaint();
}

void PatternEditorPanel::redrawByMaskChanged()
{
	foreChanged_ = true;
	headerChanged_ = true;
	repaint();
}

void PatternEditorPanel::redrawPatterns()
{
	backChanged_ = true;
	textChanged_ = true;
	foreChanged_ = true;
	repaint();
}

void PatternEditorPanel::redrawAll()
{
	headerChanged_ = true;
	redrawPatterns();
}

void PatternEditorPanel::resetEntryCount()
{
	entryCnt_ = 0;
}

void PatternEditorPanel::drawPattern(const QRect &rect)
{
	if (!freezed_ && repaintable_.load()) {
		repaintable_.store(false);
		++repaintingCnt_;	// Use module data after this line

		if (backChanged_ || textChanged_ || foreChanged_ || headerChanged_ || focusChanged_ || stepDownCount_ || followModeChanged_) {

			int maxWidth = std::min(rect.width(), tracksWidthFromLeftToEnd_);
			int trackSize = static_cast<int>(songStyle_.trackAttribs.size());
			completePixmap_->fill(palette_->ptnBackColor);

			if (!focusChanged_) {
				if (stepDownCount_ && !followModeChanged_) {
					quickDrawRows(maxWidth, trackSize);
				}
				else {
					backPixmap_->fill(Qt::transparent);
					if (textChanged_) textPixmap_->fill(Qt::transparent);
					if (foreChanged_) forePixmap_->fill(Qt::transparent);
					drawRows(maxWidth, trackSize);
				}

				if (headerChanged_) {
					// headerPixmap_->fill(Qt::transparent);
					drawHeaders(maxWidth, trackSize);
				}
			}

			{
				QPainter mergePainter(completePixmap_.get());
				QRect rowsRect(0, viewedRowOffset_, maxWidth, viewedRegionHeight_);
				QRect inViewRect(0, headerHeight_, maxWidth, viewedRegionHeight_);
				mergePainter.drawPixmap(inViewRect, *backPixmap_.get(), rowsRect);
				mergePainter.drawPixmap(inViewRect, *textPixmap_.get(), rowsRect);
				mergePainter.drawPixmap(inViewRect, *forePixmap_.get(), rowsRect);
				mergePainter.drawPixmap(headerPixmap_->rect(), *headerPixmap_.get());
			}

			drawBorders(maxWidth, trackSize);
			if (!hasFocus()) drawShadow();

			backChanged_ = false;
			textChanged_ = false;
			foreChanged_ = false;
			headerChanged_ = false;
			focusChanged_ = false;
			followModeChanged_ = false;
			stepDownCount_ = 0;
		}

		--repaintingCnt_;	// Used module data until this line
		repaintable_.store(true);
	}

	QPainter completePainter(this);
	completePainter.drawPixmap(rect, *completePixmap_.get());
}

void PatternEditorPanel::drawRows(int maxWidth, int trackSize)
{
	QPainter forePainter(forePixmap_.get());
	QPainter textPainter(textPixmap_.get());
	QPainter backPainter(backPixmap_.get());
	textPainter.setFont(stepFont_);

	int x, trackNum;

	/* Current row */
	// Fill row
	backPainter.fillRect(0, viewedCenterY_, maxWidth, stepFontHeight_,
						 bt_->isJamMode() ? palette_->ptnCurStepColor : palette_->ptnCurEditStepColor);
	// Step number
	if (markerPos_.order == curPos_.order && markerPos_.step == curPos_.step)
		backPainter.fillRect(0, viewedCenterY_, stepNumWidth_, stepFontHeight_, palette_->ptnMarkerColor);	// Paint marker
	if (hovPos_.track == -2 && hovPos_.order == curPos_.order && hovPos_.step == curPos_.step)
		backPainter.fillRect(0, viewedCenterY_, stepNumWidth_, stepFontHeight_, palette_->ptnHovCellColor);	// Paint hover
	if (textChanged_) {
		if (curPos_.step % hl2Cnt_) {
			textPainter.setPen(!(curPos_.step % hl2Cnt_) ? palette_->ptnHl1StepNumColor
														 : !(curPos_.step % hl1Cnt_) ? palette_->ptnHl1StepNumColor
																					 : palette_->ptnDefStepNumColor);
		}
		else {
			textPainter.setPen(palette_->ptnHl2StepNumColor);
		}
		textPainter.drawText(1, viewedCenterBaseY_, QString("%1").arg(curPos_.step, stepNumWidthCnt_, stepNumBase_, QChar('0')).toUpper());
	}
	// Step data
	for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
		x += drawStep(forePainter, textPainter, backPainter, trackNum, curPos_.order, curPos_.step, x, viewedCenterBaseY_, viewedCenterY_);
		++trackNum;
	}
	viewedCenterPos_ = curPos_;

	int stepNum, odrNum;
	int rowY, baseY;
	int playOdrNum = bt_->getPlayingOrderNumber();
	int playStepNum = bt_->getPlayingStepNumber();

	/* Previous rows */
	viewedFirstPos_ = curPos_;
	for (rowY = viewedCenterY_ - stepFontHeight_, baseY = viewedCenterBaseY_ - stepFontHeight_,
		 stepNum = curPos_.step - 1, odrNum = curPos_.order;
		 rowY >= 0;
		 rowY -= stepFontHeight_, baseY -= stepFontHeight_, --stepNum) {
		if (stepNum == -1) {
			if (odrNum == 0) {
				break;
			}
			else if (config_->getShowPreviousNextOrders()) {
				--odrNum;
				stepNum = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, odrNum)) - 1;
			}
			else {
				break;
			}
		}

		QColor rowColor;
		if (!config_->getFollowMode() && odrNum == playOdrNum && stepNum == playStepNum) {
			rowColor = palette_->ptnPlayStepColor;
		}
		else {
			rowColor = !(stepNum % hl2Cnt_) ? palette_->ptnHl2StepColor
											: !(stepNum % hl1Cnt_) ? palette_->ptnHl1StepColor
																   : palette_->ptnDefStepColor;
		}

		// Fill row
		backPainter.fillRect(0, rowY, maxWidth, stepFontHeight_, rowColor);
		// Step number
		if (markerPos_.order == odrNum && markerPos_.step == stepNum)
			backPainter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, palette_->ptnMarkerColor);	// Paint marker
		if (hovPos_.track == -2 && hovPos_.order == odrNum && hovPos_.step == stepNum)
			backPainter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, palette_->ptnHovCellColor);	// Paint hover
		if (textChanged_) {
			textPainter.setPen(!(stepNum % hl2Cnt_) ? palette_->ptnHl2StepNumColor
													: !(stepNum % hl1Cnt_) ? palette_->ptnHl1StepNumColor
																		   : palette_->ptnDefStepNumColor);
			textPainter.drawText(1, baseY, QString("%1").arg(stepNum, stepNumWidthCnt_, stepNumBase_, QChar('0')).toUpper());
		}
		// Step data
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
			x += drawStep(forePainter, textPainter, backPainter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (foreChanged_) {
			if (odrNum != curPos_.order)	// Mask
				forePainter.fillRect(0, rowY, maxWidth, stepFontHeight_, palette_->ptnMaskColor);
		}
		viewedFirstPos_.setRows(odrNum, stepNum);
	}

	int stepEnd = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order));

	/* Next rows */
	viewedLastPos_ = curPos_;
	for (rowY = viewedCenterY_ + stepFontHeight_, baseY = viewedCenterBaseY_ + stepFontHeight_,
		 stepNum = curPos_.step + 1, odrNum = curPos_.order;
		 rowY < viewedRowsHeight_;
		 rowY += stepFontHeight_, baseY += stepFontHeight_, ++stepNum) {
		if (stepNum == stepEnd) {
			if (odrNum == static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1) {
				break;
			}
			else if (config_->getShowPreviousNextOrders()) {
				++odrNum;
				stepNum = 0;
				stepEnd = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, odrNum));
			}
			else {
				break;
			}
		}

		QColor rowColor;
		if (!config_->getFollowMode() && odrNum == playOdrNum && stepNum == playStepNum) {
			rowColor = palette_->ptnPlayStepColor;
		}
		else {
			rowColor = !(stepNum % hl2Cnt_) ? palette_->ptnHl2StepColor
											: !(stepNum % hl1Cnt_) ? palette_->ptnHl1StepColor
																   : palette_->ptnDefStepColor;
		}

		// Fill row
		backPainter.fillRect(0, rowY, maxWidth, stepFontHeight_, rowColor);
		// Step number
		if (markerPos_.order == odrNum && markerPos_.step == stepNum)
			backPainter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, palette_->ptnMarkerColor);	// Paint marker
		if (hovPos_.track == -2 && hovPos_.order == odrNum && hovPos_.step == stepNum)
			backPainter.fillRect(0, rowY, stepNumWidth_, stepFontHeight_, palette_->ptnHovCellColor);	// Paint hover
		if (textChanged_) {
			textPainter.setPen(!(stepNum % hl2Cnt_) ? palette_->ptnHl2StepNumColor
													: !(stepNum % hl1Cnt_) ? palette_->ptnHl1StepNumColor
																		   : palette_->ptnDefStepNumColor);
			textPainter.drawText(1, baseY, QString("%1").arg(stepNum, stepNumWidthCnt_, stepNumBase_, QChar('0')).toUpper());
		}
		// Step data
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
			x += drawStep(forePainter, textPainter, backPainter, trackNum, odrNum, stepNum, x, baseY, rowY);
			++trackNum;
		}
		if (foreChanged_) {
			if (odrNum != curPos_.order)	// Mask
				forePainter.fillRect(0, rowY, maxWidth, stepFontHeight_, palette_->ptnMaskColor);
		}
		viewedLastPos_.setRows(odrNum, stepNum);
	}
}

void PatternEditorPanel::quickDrawRows(int maxWidth, int trackSize)
{
	int halfRowsCnt = viewedRowCnt_ >> 1;
	bool repaintForeAll = (curPos_.step - stepDownCount_ < 0);
	int shift = stepFontHeight_ * stepDownCount_;

	/* Move up */
	QRect srcRect(0, 0, maxWidth, viewedRowsHeight_);
	if (!repaintForeAll) forePixmap_->scroll(0, -shift, srcRect);
	textPixmap_->scroll(0, -shift, srcRect);
	backPixmap_->scroll(0, -shift, srcRect);
	{
		PatternPosition fpos = calculatePositionFrom(viewedCenterPos_.order, viewedCenterPos_.step, stepDownCount_ - halfRowsCnt);
		if (fpos.order != -1) viewedFirstPos_ = std::move(fpos);
	}

	QPainter forePainter(forePixmap_.get());
	QPainter textPainter(textPixmap_.get());
	QPainter backPainter(backPixmap_.get());
	textPainter.setFont(stepFont_);

	int x, trackNum;

	/* Clear previous cursor row, current cursor row and last rows text and foreground */
	int prevY = viewedCenterY_ - shift;
	int lastY = viewedRowsHeight_ - shift;
	textPainter.setCompositionMode(QPainter::CompositionMode_Source);
	textPainter.fillRect(0, prevY, maxWidth, stepFontHeight_, Qt::transparent);
	textPainter.fillRect(0, viewedCenterY_, maxWidth, stepFontHeight_, Qt::transparent);
	textPainter.fillRect(0, lastY, maxWidth, shift, Qt::transparent);
	textPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	if (!repaintForeAll) {
		forePainter.setCompositionMode(QPainter::CompositionMode_Source);
		forePainter.fillRect(0, prevY, maxWidth, stepFontHeight_, Qt::transparent);
		forePainter.fillRect(0, viewedCenterY_, maxWidth, stepFontHeight_, Qt::transparent);
		forePainter.fillRect(0, lastY, maxWidth, shift, Qt::transparent);
		forePainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	}

	/* Redraw previous cursor step */
	{
		int baseY = viewedCenterBaseY_ - shift;
		QColor rowColor = !(viewedCenterPos_.step % hl2Cnt_) ? palette_->ptnHl2StepColor
															 : !(viewedCenterPos_.step % hl1Cnt_) ? palette_->ptnHl1StepColor
																								  : palette_->ptnDefStepColor;
		// Fill row
		backPainter.fillRect(0, prevY, maxWidth, stepFontHeight_, rowColor);
		// Step number
		if (markerPos_.order == viewedCenterPos_.order && markerPos_.step == viewedCenterPos_.step)
			backPainter.fillRect(0, prevY, stepNumWidth_, stepFontHeight_, palette_->ptnMarkerColor);	// Paint marker
		if (hovPos_.track == -2 && hovPos_.order == viewedCenterPos_.order && hovPos_.step == viewedCenterPos_.step)
			backPainter.fillRect(0, prevY, stepNumWidth_, stepFontHeight_, palette_->ptnHovCellColor);	// Paint hover
		textPainter.setPen(!(viewedCenterPos_.step % hl2Cnt_) ? palette_->ptnHl2StepNumColor
															  : !(viewedCenterPos_.step % hl1Cnt_) ? palette_->ptnHl1StepNumColor
																								   : palette_->ptnDefStepNumColor);
		textPainter.drawText(1, baseY, QString("%1").arg(viewedCenterPos_.step, stepNumWidthCnt_, stepNumBase_, QChar('0')).toUpper());
		// Step data
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
			x += drawStep(forePainter, textPainter, backPainter, trackNum, viewedCenterPos_.order, viewedCenterPos_.step, x, baseY, prevY);
			++trackNum;
		}
	}

	/* Redraw current cursor step */
	// Fill row
	backPainter.fillRect(0, viewedCenterY_, maxWidth, stepFontHeight_,
						 bt_->isJamMode() ? palette_->ptnCurStepColor : palette_->ptnCurEditStepColor);
	// Step number
	if (markerPos_.order == curPos_.order && markerPos_.step == curPos_.step)
		backPainter.fillRect(0, viewedCenterY_, stepNumWidth_, stepFontHeight_, palette_->ptnMarkerColor);	// Paint marker
	if (hovPos_.track == -2 && hovPos_.order == curPos_.order && hovPos_.step == curPos_.step)
		backPainter.fillRect(0, viewedCenterY_, stepNumWidth_, stepFontHeight_, palette_->ptnHovCellColor);	// Paint hover
	if (curPos_.step % hl2Cnt_) {
		textPainter.setPen(!(curPos_.step % hl2Cnt_) ? palette_->ptnHl1StepNumColor
													 : !(curPos_.step % hl1Cnt_) ? palette_->ptnHl1StepNumColor
																				 : palette_->ptnDefStepNumColor);
	}
	else {
		textPainter.setPen(palette_->ptnHl2StepNumColor);
	}
	textPainter.drawText(1, viewedCenterBaseY_, QString("%1").arg(curPos_.step, stepNumWidthCnt_, stepNumBase_, QChar('0')).toUpper());
	// Step data
	for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
		x += drawStep(forePainter, textPainter, backPainter, trackNum, curPos_.order, curPos_.step, x, viewedCenterBaseY_, viewedCenterY_);
		++trackNum;
	}
	viewedCenterPos_ = curPos_;

	/* Draw new step at last if necessary */
	{
		PatternPosition bpos = calculatePositionFrom(viewedCenterPos_.order, viewedCenterPos_.step, halfRowsCnt);
		if (!config_->getShowPreviousNextOrders() && viewedCenterPos_.order != bpos.order) {
			// Clear row
			backPainter.setCompositionMode(QPainter::CompositionMode_Source);
			backPainter.fillRect(0, lastY, maxWidth, shift, Qt::transparent);
		}
		else {
			int baseY = lastY + (viewedCenterBaseY_ - viewedCenterY_);
			bpos = std::exchange(viewedLastPos_, bpos);
			while (true) {
				if (bpos.compareRows(viewedLastPos_) == 0) break;
				PatternPosition tmpBpos = calculatePositionFrom(bpos.order, bpos.step, 1);
				if (tmpBpos.order == -1) {	// when viewedLastPos_.row == -1 (viewedlastPos_.row < viewedCenterPos_.row + halRowsCnt)
					viewedLastPos_ = bpos;
					// Clear row
					backPainter.setCompositionMode(QPainter::CompositionMode_Source);
					backPainter.fillRect(0, lastY, maxWidth, shift, Qt::transparent);
					break;
				}
				else {
					bpos = tmpBpos;
				}

				QColor rowColor = !(bpos.step % hl2Cnt_) ? palette_->ptnHl2StepColor
														 : !(bpos.step % hl1Cnt_) ? palette_->ptnHl1StepColor
																				  : palette_->ptnDefStepColor;
				// Fill row
				backPainter.fillRect(0, lastY, maxWidth, stepFontHeight_, rowColor);
				// Step number
				if (markerPos_.order == bpos.order && markerPos_.step == bpos.step)
					backPainter.fillRect(0, lastY, stepNumWidth_, stepFontHeight_, palette_->ptnMarkerColor);	// Paint marker
				if (hovPos_.track == -2 && hovPos_.order == bpos.order && hovPos_.step == bpos.step)
					backPainter.fillRect(0, lastY, stepNumWidth_, stepFontHeight_, palette_->ptnHovCellColor);	// Paint hover
				textPainter.setPen(!(bpos.step % hl2Cnt_) ? palette_->ptnHl2StepNumColor
														  : !(bpos.step % hl1Cnt_) ? palette_->ptnHl1StepNumColor
																				   : palette_->ptnDefStepNumColor);
				textPainter.drawText(1, baseY, QString("%1").arg(bpos.step, stepNumWidthCnt_, stepNumBase_, QChar('0')).toUpper());
				// Step data
				for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
					x += drawStep(forePainter, textPainter, backPainter, trackNum, bpos.order, bpos.step, x, baseY, lastY);
					++trackNum;
				}
				if (bpos.order != curPos_.order)	// Mask
					forePainter.fillRect(0, lastY, maxWidth, stepFontHeight_, palette_->ptnMaskColor);

				baseY += stepFontHeight_;
				lastY += stepFontHeight_;
			}
		}
	}

	/* Redraw foreground all area if new order */
	if (repaintForeAll) {
		forePixmap_->fill(Qt::transparent);
		int y = viewedCenterY_ - viewedCenterPos_.step * stepFontHeight_;
		if (y > 0) forePainter.fillRect(0, 0, maxWidth, y, palette_->ptnMaskColor);
		if (viewedLastPos_.order != viewedCenterPos_.order) {
			y += static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, viewedCenterPos_.order)) * stepFontHeight_;
			forePainter.fillRect(0, y, maxWidth, viewedRowsHeight_ - y, palette_->ptnMaskColor);
		}
		for (x = stepNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
			int w = baseTrackWidth_ + effWidth_ * rightEffn_.at(static_cast<size_t>(trackNum));
			if (foreChanged_ && bt_->isMute(trackNum))	// Paint mute mask
				forePainter.fillRect(x, 0, w, viewedRowsHeight_, palette_->ptnMaskColor);
			x += w;
			++trackNum;
		}
	}
}

int PatternEditorPanel::drawStep(QPainter &forePainter, QPainter &textPainter, QPainter& backPainter, int trackNum, int orderNum, int stepNum, int x, int baseY, int rowY)
{
	int offset = x + widthSpace_;
	PatternPosition pos{ trackNum, 0, orderNum, stepNum };
	QColor textColor = (orderNum == curPos_.order && stepNum == curPos_.step) ? palette_->ptnCurTextColor : palette_->ptnDefTextColor;
	bool isHovTrack = (hovPos_.order == -2 && hovPos_.track == trackNum);
	bool isHovStep = (hovPos_.track == -2 && hovPos_.order == orderNum && hovPos_.step == stepNum);
	bool isMuteTrack = bt_->isMute(trackNum);
	SoundSource src = songStyle_.trackAttribs[static_cast<size_t>(trackNum)].source;


	/* Tone name */
	if (pos == curPos_)	// Paint current cell
		backPainter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnCurCellColor);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		backPainter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnHovCellColor);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 0, orderNum, stepNum))	// Paint selected
		backPainter.fillRect(offset - widthSpace_, rowY, toneNameWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnSelCellColor);
	if (textChanged_) {
		int noteNum = bt_->getStepNoteNumber(curSongNum_, trackNum, orderNum, stepNum);
		switch (noteNum) {
		case -1:	// None
			textPainter.setPen(textColor);
			textPainter.drawText(offset, baseY, "---");
			break;
		case -2:	// Key off
			textPainter.fillRect(offset, rowY + stepFontHeight_ * 2 / 5,
								 toneNameWidth_, stepFontHeight_ / 5, palette_->ptnNoteColor);
			break;
		case -3:	// Echo 0
			textPainter.setPen(palette_->ptnNoteColor);
			textPainter.drawText(offset + stepFontWidth_ / 2, baseY, "^0");
			break;
		case -4:	// Echo 1
			textPainter.setPen(palette_->ptnNoteColor);
			textPainter.drawText(offset + stepFontWidth_ / 2, baseY, "^1");
			break;
		case -5:	// Echo 2
			textPainter.setPen(palette_->ptnNoteColor);
			textPainter.drawText(offset + stepFontWidth_ / 2, baseY, "^2");
			break;
		case -6:	// Echo 3
			textPainter.setPen(palette_->ptnNoteColor);
			textPainter.drawText(offset + stepFontWidth_ / 2, baseY, "^3");
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
			textPainter.setPen(palette_->ptnNoteColor);
			textPainter.drawText(offset, baseY, toneStr + QString::number(noteNum / 12));
			break;
		}
		}
	}
	offset += toneNameWidth_ +  widthSpaceDbl_;
	pos.colInTrack = 1;

	/* Instrument */
	if (pos == curPos_)	// Paint current cell
		backPainter.fillRect(offset - widthSpace_, rowY, instWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnCurCellColor);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		backPainter.fillRect(offset - widthSpace_, rowY, instWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnHovCellColor);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 1, orderNum, stepNum))	// Paint selected
		backPainter.fillRect(offset - widthSpace_, rowY, instWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnSelCellColor);
	if (textChanged_) {
		int instNum = bt_->getStepInstrument(curSongNum_, trackNum, orderNum, stepNum);
		if (instNum == -1) {
			textPainter.setPen(textColor);
			textPainter.drawText(offset, baseY, "--");
		}
		else {
			std::unique_ptr<AbstractInstrument> inst = bt_->getInstrument(instNum);
			textPainter.setPen((inst != nullptr && src == inst->getSoundSource())
							   ? palette_->ptnInstColor
							   : palette_->ptnErrorColor);
			textPainter.drawText(offset, baseY, QString("%1").arg(instNum, 2, 16, QChar('0')).toUpper());
		}
	}
	offset += instWidth_ +  widthSpaceDbl_;
	pos.colInTrack = 2;

	/* Volume */
	if (pos == curPos_)	// Paint current cell
		backPainter.fillRect(offset - widthSpace_, rowY, volWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnCurCellColor);
	if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
		backPainter.fillRect(offset - widthSpace_, rowY, volWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnHovCellColor);
	if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
			&& isSelectedCell(trackNum, 2, orderNum, stepNum))	// Paint selected
		backPainter.fillRect(offset - widthSpace_, rowY, volWidth_ + widthSpaceDbl_, stepFontHeight_, palette_->ptnSelCellColor);
	if (textChanged_) {
		int vol = bt_->getStepVolume(curSongNum_, trackNum, orderNum, stepNum);
		if (vol == -1) {
			textPainter.setPen(textColor);
			textPainter.drawText(offset, baseY, "--");
		}
		else {
			int volLim = 0;	// Dummy set
			switch (src) {
			case SoundSource::FM:		volLim = 0x80;	break;
			case SoundSource::SSG:		volLim = 0x10;	break;
			case SoundSource::DRUM:		volLim = 0x20;	break;
			case SoundSource::ADPCM:	volLim = 0x100;	break;
			}
			textPainter.setPen((vol < volLim) ? palette_->ptnVolColor : palette_->ptnErrorColor);
			if (src == SoundSource::FM && vol < volLim && config_->getReverseFMVolumeOrder()) {

				vol = volLim - vol - 1;
			}
			textPainter.drawText(offset, baseY, QString("%1").arg(vol, 2, 16, QChar('0')).toUpper());
		}
	}
	offset += volWidth_ +  widthSpaceDbl_;
	pos.colInTrack = 3;

	/* Effect */
	for (int i = 0; i <= rightEffn_.at(static_cast<size_t>(trackNum)); ++i) {
		/* Effect ID */
		if (pos == curPos_)	// Paint current cell
			backPainter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, palette_->ptnCurCellColor);
		if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
			backPainter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, palette_->ptnHovCellColor);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
				&& isSelectedCell(trackNum, pos.colInTrack, orderNum, stepNum))	// Paint selected
			backPainter.fillRect(offset - widthSpace_, rowY, effIDWidth_ + widthSpace_, stepFontHeight_, palette_->ptnSelCellColor);
		std::string effId;
		QString effStr;
		if (textChanged_) {
			effId = bt_->getStepEffectID(curSongNum_, trackNum, orderNum, stepNum, i);
			effStr = QString::fromStdString(effId);
			if (effStr == "--") {
				textPainter.setPen(textColor);
				textPainter.drawText(offset, baseY, effStr);
			}
			else {
				textPainter.setPen(palette_->ptnEffColor);
				textPainter.drawText(offset, baseY, effStr);
			}
		}
		offset += effIDWidth_;
		++pos.colInTrack;

		/* Effect Value */
		if (pos == curPos_)	// Paint current cell
			backPainter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, palette_->ptnCurCellColor);
		if (pos == hovPos_ || isHovTrack || isHovStep)	// Paint hover
			backPainter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, palette_->ptnHovCellColor);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.order >= 0)
				&& isSelectedCell(trackNum, pos.colInTrack, orderNum, stepNum))	// Paint selected
			backPainter.fillRect(offset, rowY, effValWidth_ + widthSpace_, stepFontHeight_, palette_->ptnSelCellColor);
		if (textChanged_) {
			int effVal = bt_->getStepEffectValue(curSongNum_, trackNum, orderNum, stepNum, i);
			if (effVal == -1) {
				textPainter.setPen(textColor);
				textPainter.drawText(offset, baseY, "--");
			}
			else {
				textPainter.setPen(palette_->ptnEffColor);
				switch (Effect::toEffectType(src, effId)) {
				case EffectType::VolumeDelay:
					if (src == SoundSource::FM && config_->getReverseFMVolumeOrder() && effVal < 0x80)
						effVal = 0x7f - effVal;
					break;
				case EffectType::Brightness:
					if (config_->getReverseFMVolumeOrder() && effVal > 0)
						effVal = 0xff - effVal + 1;
					break;
				default:
					break;
				}
				if (src == SoundSource::FM && config_->getReverseFMVolumeOrder()
						&& Effect::toEffectType(SoundSource::FM, effId) == EffectType::VolumeDelay) {


				}
				textPainter.drawText(offset, baseY, QString("%1").arg(effVal, 2, 16, QChar('0')).toUpper());
			}
		}
		offset += effValWidth_ + widthSpaceDbl_;
		++pos.colInTrack;
	}

	if (foreChanged_ && isMuteTrack)	// Paint mute mask
		forePainter.fillRect(x, rowY, offset - x, stepFontHeight_, palette_->ptnMaskColor);

	return baseTrackWidth_ + effWidth_ * rightEffn_[static_cast<size_t>(trackNum)];
}

void PatternEditorPanel::drawHeaders(int maxWidth, int trackSize)
{
	QPainter painter(headerPixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, palette_->ptnHeaderRowColor);
	int x, trackNum;
	int lspace = stepFontWidth_ / 2;
	for (x = stepNumWidth_ + lspace, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
		int tw = baseTrackWidth_ + effWidth_ * rightEffn_.at(static_cast<size_t>(trackNum));
		if (hovPos_.order == -2 && hovPos_.track == trackNum)
			painter.fillRect(x - lspace, 0, tw, headerHeight_, palette_->ptnHovCellColor);

		painter.setPen(palette_->ptnHeaderTextColor);
		QString srcName;
		const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(trackNum)];
		switch (attrib.source) {
		case SoundSource::FM:
			switch (songStyle_.type) {
			case SongType::Standard:
				srcName = "FM" + QString::number(attrib.channelInSource + 1);
				break;
			case SongType::FM3chExpanded:
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
		case SoundSource::ADPCM:
			srcName = "ADPCM";
			break;
		}
		painter.drawText(x, headerFontAscent_, srcName);

		painter.fillRect(x, headerHeight_ - 4, hdMuteToggleWidth_, 2,
						 bt_->isMute(trackNum) ? palette_->ptnMuteColor : palette_->ptnUnmuteColor);

		painter.drawText(x + hdMuteToggleWidth_ + lspace, hdPlusY_, "+");
		painter.drawText(x + hdMuteToggleWidth_ + lspace, hdMinusY_, "-");

		x += tw;
		++trackNum;
	}
}

void PatternEditorPanel::drawBorders(int maxWidth, int trackSize)
{
	Q_UNUSED(trackSize)

	QPainter painter(completePixmap_.get());

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
	QPainter painter(completePixmap_.get());
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
		trackNum = std::min(trackNum, static_cast<int>(rightEffn_.size()));
		return std::accumulate(rightEffn_.begin(), rightEffn_.begin() + trackNum, colNumInTrack,
							   [](int acc, int v) { return acc + 5 + 2 * v; });
	}
}

void PatternEditorPanel::moveCursorToRight(int n)
{
	int oldTrackNum = curPos_.track;
	bool oldLeftTrack = leftTrackNum_;

	curPos_.colInTrack += n;
	if (n > 0) {
		while (true) {
			int lim = 5 + 2 * rightEffn_.at(static_cast<size_t>(curPos_.track));
			if (curPos_.colInTrack < lim) {
				break;
			}
			else {
				if (curPos_.track == static_cast<int>(songStyle_.trackAttribs.size()) - 1) {
					if (config_->getWarpCursor()) {
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
					if (config_->getWarpCursor()) {
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

	updateTracksWidthFromLeftToEnd();
	entryCnt_ = 0;

	if (curPos_.track != oldTrackNum)
		bt_->setCurrentTrack(curPos_.track);

	if (!isIgnoreToSlider_) {
		if (config_->getMoveCursorByHorizontalScroll()) {
			emit hScrollBarChangeRequested(calculateColNumInRow(curPos_.track, curPos_.colInTrack));
		}
		else if (curPos_.track != oldTrackNum) {
			emit hScrollBarChangeRequested(leftTrackNum_);
		}
	}

	if (!isIgnoreToOrder_ && curPos_.track != oldTrackNum)	// Send to order list
		emit currentTrackChanged(curPos_.track);

	// Request fore-background repaint if leftmost track is changed else request only background repaint
	if (leftTrackNum_ != oldLeftTrack) {
		headerChanged_ = true;
		foreChanged_ = true;
		textChanged_ = true;
	}
	backChanged_ = true;
	repaint();
}

void PatternEditorPanel::moveViewToRight(int n)
{
	leftTrackNum_ += n;
	updateTracksWidthFromLeftToEnd();

	// Calculate cursor position
	int track = curPos_.track + n;
	int col = std::min(curPos_.colInTrack,
					   4 + 2 * rightEffn_.at(static_cast<size_t>(track)));

	// Check visible
	int width = stepNumWidth_;
	for (int i = leftTrackNum_; i <= track; ++i) {
		width += (baseTrackWidth_ + effWidth_ * rightEffn_.at(static_cast<size_t>(i)));
		if (geometry().width() < width) {
			track = i - 1;
			col = 4 + 2 * rightEffn_.at(static_cast<size_t>(track));
			break;
		}
	}

	// Move cursor and repaint all
	headerChanged_ = true;
	foreChanged_ = true;
	textChanged_ = true;
	moveCursorToRight(calculateColumnDistance(curPos_.track, curPos_.colInTrack, track, col));
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
				if (config_->getWarpAcrossOrders()) {
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
				if (config_->getWarpAcrossOrders()) {
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

	entryCnt_ = 0;

	if (!isIgnoreToSlider_)
		emit vScrollBarChangeRequested(
				curPos_.step, static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order)) - 1);

	if (!isIgnoreToOrder_ && curPos_.order != oldOdr)	// Send to order list
		emit currentOrderChanged(
				curPos_.order, static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1);

	backChanged_ = true;
	textChanged_ = true;
	foreChanged_ = true;
	repaint();
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

PatternPosition PatternEditorPanel::calculatePositionFrom(int order, int step, int by) const
{
	PatternPosition pos{ -1, -1, order, step + by };

	if (by > 0) {
		while (true) {
			int dif = pos.step - static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, pos.order));
			if (dif < 0) {
				break;
			}
			else {
				if (pos.order == static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1) {
					return { -1, -1, -1, -1 };
				}
				else {
					++pos.order;
				}
				pos.step = dif;
			}
		}
	}
	else {
		while (true) {
			if (pos.step < 0) {
				if (pos.order == 0) {
					return { -1, -1, -1, -1 };
				}
				else {
					--pos.order;
				}
				pos.step += bt_->getPatternSizeFromOrderNumber(curSongNum_, pos.order);
			}
			else {
				break;
			}
		}
	}
	return pos;
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

int PatternEditorPanel::getScrollableCountByTrack() const
{
	int width = stepNumWidth_;
	size_t i = songStyle_.trackAttribs.size();
	do {
		--i;
		width += (baseTrackWidth_ + effWidth_ * rightEffn_.at(i));
		if (geometry().width() < width) {
			return static_cast<int>(i + 1);
		}
	} while (i);
	return 0;
}

void PatternEditorPanel::changeEditable()
{
	backChanged_ = true;
	repaint();
}

int PatternEditorPanel::getFullColmunSize() const
{
	return calculateColNumInRow(static_cast<int>(songStyle_.trackAttribs.size()) - 1, 4 + 2 * rightEffn_.back());
}

void PatternEditorPanel::updatePositionByStepUpdate(bool isFirstUpdate, bool forceJump)
{
	if (!forceJump && !config_->getFollowMode()) {	// Repaint only background
		backChanged_ = true;
		repaint();
		return;
	}

	PatternPosition tmp = curPos_;
	curPos_.setRows(bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());
	int cmp = curPos_.compareRows(tmp);
	if (!cmp && !isFirstUpdate) return;	// Delayed call, already updated.

	emit vScrollBarChangeRequested(
				curPos_.step, static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order)) - 1);

	if (isFirstUpdate || (cmp < 0) || (cmp && !config_->getShowPreviousNextOrders())) {
		stepDownCount_ = 0;	// Redraw entire area in first update
	}
	else {
		int d = calculateStepDistance(tmp.order, tmp.step, curPos_.order, curPos_.step);
		stepDownCount_ = (d < (viewedRowCnt_ >> 1)) ? d : 0;
	}
	// If stepChanged is false, repaint all pattern
	foreChanged_ = true;
	textChanged_ = true;
	backChanged_ = true;
	repaint();
}

void PatternEditorPanel::changeMarker()
{
	markerPos_.setRows(bt_->getMarkerOrder(), bt_->getMarkerStep());
	backChanged_ = true;
	repaint();
}

JamKey PatternEditorPanel::getJamKeyFromLayoutMapping(Qt::Key key) {
	Configuration::KeyboardLayout selectedLayout = config_->getNoteEntryLayout();
	if (config_->mappingLayouts.find (selectedLayout) != config_->mappingLayouts.end()) {
		std::unordered_map<std::string, JamKey> selectedLayoutMapping = config_->mappingLayouts.at (selectedLayout);
		auto it = std::find_if(selectedLayoutMapping.begin(), selectedLayoutMapping.end(),
							   [key](const std::pair<std::string, JamKey>& t) -> bool {
			return (QKeySequence(key).matches(QKeySequence(QString::fromStdString(t.first))) == QKeySequence::ExactMatch);
		});
		if (it != selectedLayoutMapping.end()) {
			return (*it).second;
		}
		else {
			throw std::invalid_argument("Unmapped key");
		}
		//something has gone wrong, current layout has no layout map
		//TODO: handle cleanly?
	} else throw std::out_of_range("Unmapped Layout");
}

bool PatternEditorPanel::enterToneData(QKeyEvent* event)
{
	int baseOct = bt_->getCurrentOctave();

	if (event->modifiers().testFlag(Qt::NoModifier)) {
		Qt::Key qtKey = static_cast<Qt::Key>(event->key());
		try {
			JamKey possibleJamKey = getJamKeyFromLayoutMapping(qtKey);
			int octaveOffset = 0;
			switch (possibleJamKey) {
			case JamKey::HighD2:
			case JamKey::HighCS2:
			case JamKey::HighC2:
				octaveOffset = 2;
				break;
			case JamKey::HighB:
			case JamKey::HighAS:
			case JamKey::HighA:
			case JamKey::HighGS:
			case JamKey::HighG:
			case JamKey::HighFS:
			case JamKey::HighF:
			case JamKey::HighE:
			case JamKey::HighDS:
			case JamKey::HighD:
			case JamKey::HighCS:
			case JamKey::HighC:
			case JamKey::LowD2:
			case JamKey::LowCS2:
			case JamKey::LowC2:
				octaveOffset = 1;
				break;
			default:
				break;
			}
			setStepKeyOn(JamManager::jamKeyToNote(possibleJamKey), baseOct + octaveOffset);
		} catch (std::invalid_argument &) {}
	}

	return false;
}

void PatternEditorPanel::setStepKeyOn(Note note, int octave)
{
	if (octave < 8) {
		bt_->setStepNote(curSongNum_, curPos_.track, curPos_.order, curPos_.step, octave, note,
						 config_->getInstrumentMask(), config_->getVolumeMask());
		comStack_.lock()->push(new SetKeyOnToStepQtCommand(this));
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
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
	bt_->setStepInstrumentDigit(curSongNum_, curPos_.track, curPos_.order, curPos_.step, num, (entryCnt_ == 1));
	comStack_.lock()->push(new SetInstrumentToStepQtCommand(this, curPos_, (entryCnt_ == 1)));

	emit instrumentEntered(
				bt_->getStepInstrument(curSongNum_, curPos_.track, curPos_.order, curPos_.step));

	if ((!bt_->isPlaySong() || !bt_->isFollowPlay()) && !updateEntryCount())
		moveCursorToDown(editableStepCnt_);
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
	bool isReversed = (songStyle_.trackAttribs[static_cast<size_t>(curPos_.track)].source == SoundSource::FM
					  && config_->getReverseFMVolumeOrder());
	bt_->setStepVolumeDigit(curSongNum_, curPos_.track, curPos_.order, curPos_.step, volume, isReversed, (entryCnt_ == 1));
	comStack_.lock()->push(new SetVolumeToStepQtCommand(this, curPos_, (entryCnt_ == 1)));

	if ((!bt_->isPlaySong() || !bt_->isFollowPlay()) && !updateEntryCount())
		moveCursorToDown(editableStepCnt_);
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
	bt_->setStepEffectIDCharacter(curSongNum_, curPos_.track, curPos_.order, curPos_.step,
								  (curPos_.colInTrack - 3) / 2, str.toStdString(),
								  config_->getFill00ToEffectValue(), (entryCnt_ == 1));
	comStack_.lock()->push(new SetEffectIDToStepQtCommand(this, curPos_, (entryCnt_ == 1)));

	PatternPosition editPos = curPos_;

	if ((!bt_->isPlaySong() || !bt_->isFollowPlay()) && !updateEntryCount()) {
		if (config_->getMoveCursorToRight()) moveCursorToRight(1);
		else moveCursorToDown(editableStepCnt_);
	}

	// Send effect description
	QString effDetail = tr("Invalid effect");
	std::string id = bt_->getStepEffectID(curSongNum_, editPos.track, editPos.order,
										  editPos.step, (editPos.colInTrack - 3) / 2);
	SoundSource src = songStyle_.trackAttribs.at(static_cast<size_t>(curPos_.track)).source;
	emit effectEntered(EffectDescription::getEffectFormatAndDetailString(Effect::toEffectType(src, id)));
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
	int n = (curPos_.colInTrack - 4) / 2;
	EffectDisplayControl ctrl = EffectDisplayControl::Unset;
	SoundSource src = songStyle_.trackAttribs[static_cast<size_t>(curPos_.track)].source;
	switch (Effect::toEffectType(
				src,
				bt_->getStepEffectID(curSongNum_, curPos_.track, curPos_.order, curPos_.step, n))) {
	case EffectType::VolumeDelay:
		if (src == SoundSource::FM && config_->getReverseFMVolumeOrder())
			ctrl = EffectDisplayControl::ReverseFMVolumeDelay;
		break;
	case EffectType::Brightness:
		if (config_->getReverseFMVolumeOrder()) ctrl = EffectDisplayControl::ReverseFMBrightness;
		break;
	default:
		break;
	}
	bt_->setStepEffectValueDigit(curSongNum_, curPos_.track, curPos_.order, curPos_.step, n, value, ctrl, (entryCnt_ == 1));
	comStack_.lock()->push(new SetEffectValueToStepQtCommand(this, curPos_, (entryCnt_ == 1)));

	if ((!bt_->isPlaySong() || !bt_->isFollowPlay()) && !updateEntryCount()) {
		if (config_->getMoveCursorToRight()) moveCursorToRight(1);
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
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(-1);
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

void PatternEditorPanel::transposeNote(const PatternPosition& startPos, const PatternPosition& endPos, int seminote)
{
	int beginTrack = (startPos.colInTrack == 0) ? startPos.track : startPos.track + 1;
	if (beginTrack <= endPos.track) {
		bt_->transposeNoteInPattern(curSongNum_, beginTrack, startPos.order, startPos.step,
									endPos.track, endPos.step, seminote);
		comStack_.lock()->push(new TransposeNoteInPatternQtCommand(this));
	}
}

void PatternEditorPanel::changeValuesInPattern(const PatternPosition& startPos, const PatternPosition& endPos, int value)
{
	if (startPos.compareCols(endPos) <= 0) {
		bt_->changeValuesInPattern(curSongNum_, startPos.track, startPos.colInTrack, startPos.order, startPos.step,
								   endPos.track, endPos.colInTrack, endPos.step, value, config_->getReverseFMVolumeOrder());
		comStack_.lock()->push(new ChangeValuesInPatternQtCommand(this));
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

	backChanged_ = true;
	repaint();
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
	undo->setIcon(QIcon(":/icon/undo"));
	QObject::connect(undo, &QAction::triggered, this, [&]() {
		bt_->undo();
		comStack_.lock()->undo();
	});
	QAction* redo = menu.addAction(tr("&Redo"));
	redo->setIcon(QIcon(":/icon/redo"));
	QObject::connect(redo, &QAction::triggered, this, [&]() {
		bt_->redo();
		comStack_.lock()->redo();
	});
	menu.addSeparator();
	QAction* copy = menu.addAction(tr("&Copy"));
	copy->setIcon(QIcon(":/icon/copy"));
	QObject::connect(copy, &QAction::triggered, this, &PatternEditorPanel::copySelectedCells);
	QAction* cut = menu.addAction(tr("Cu&t"));
	cut->setIcon(QIcon(":/icon/cut"));
	QObject::connect(cut, &QAction::triggered, this, &PatternEditorPanel::cutSelectedCells);
	QAction* paste = menu.addAction(tr("&Paste"));
	paste->setIcon(QIcon(":/icon/paste"));
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
	QObject::connect(deNote, &QAction::triggered, this, [&]() { onNoteTransposePressed(-1); });
	QAction* inNote = transpose->addAction(tr("&Increase Note"));
	QObject::connect(inNote, &QAction::triggered, this, [&]() { onNoteTransposePressed(1); });
	QAction* deOct = transpose->addAction(tr("D&ecrease Octave"));
	QObject::connect(deOct, &QAction::triggered, this, [&]() { onNoteTransposePressed(-12); });
	QAction* inOct = transpose->addAction(tr("I&ncrease Octave"));
	QObject::connect(inOct, &QAction::triggered, this, [&]() { onNoteTransposePressed(12); });
	auto changeVals = new QMenu(tr("&Change Values"));
	pattern->addMenu(changeVals);
	QAction* fdeVal = changeVals->addAction(tr("Fine &Decrease Values"));
	QObject::connect(fdeVal, &QAction::triggered, this, [&]() { onChangeValuesPressed(-1); });
	QAction* finVal = changeVals->addAction(tr("Fine &Increase Values"));
	QObject::connect(finVal, &QAction::triggered, this, [&]() { onChangeValuesPressed(1); });
	QAction* cdeVal = changeVals->addAction(tr("Coarse D&ecrease Values"));
	QObject::connect(cdeVal, &QAction::triggered, this, [&]() { onChangeValuesPressed(-16); });
	QAction* cinVal = changeVals->addAction(tr("Coarse I&ncrease Values"));
	QObject::connect(cinVal, &QAction::triggered, this, [&]() { onChangeValuesPressed(16); });
	menu.addSeparator();
	QAction* toggle = menu.addAction(tr("To&ggle Track"));
	QObject::connect(toggle, &QAction::triggered, this, [&] { onToggleTrackPressed(pos.track); });
	QAction* solo = menu.addAction(tr("&Solo Track"));
	QObject::connect(solo, &QAction::triggered, this, [&] { onSoloTrackPressed(pos.track); });
	menu.addSeparator();
	QAction* exeff = menu.addAction(tr("Expand E&ffect Column"));
	QObject::connect(exeff, &QAction::triggered, this, [&] { onExpandEffectColumnPressed(pos.track); });
	QAction* sheff = menu.addAction(tr("Shrin&k Effect Column"));
	QObject::connect(sheff, &QAction::triggered, this, [&] { onShrinkEffectColumnPressed(pos.track); });
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
	fdeVal->setShortcutVisibleInContextMenu(true);
	finVal->setShortcutVisibleInContextMenu(true);
	cdeVal->setShortcutVisibleInContextMenu(true);
	cinVal->setShortcutVisibleInContextMenu(true);
	toggle->setShortcutVisibleInContextMenu(true);
	solo->setShortcutVisibleInContextMenu(true);
	exeff->setShortcutVisibleInContextMenu(true);
	sheff->setShortcutVisibleInContextMenu(true);
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
	fdeVal->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F1));
	finVal->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F2));
	cdeVal->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F3));
	cinVal->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F4));
	toggle->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F9));
	solo->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F10));
	exeff->setShortcut(QKeySequence(Qt::ALT + Qt::Key_L));
	sheff->setShortcut(QKeySequence(Qt::ALT + Qt::Key_K));

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
		fdeVal->setEnabled(false);
		finVal->setEnabled(false);
		cdeVal->setEnabled(false);
		cinVal->setEnabled(false);
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
			fdeVal->setEnabled(false);
			finVal->setEnabled(false);
			cdeVal->setEnabled(false);
			cinVal->setEnabled(false);
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
		exeff->setEnabled(false);
		sheff->setEnabled(false);
	}

	menu.exec(mapToGlobal(point));
}

/********** Slots **********/
void PatternEditorPanel::onHScrollBarChanged(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	// Skip if position has already changed in panel
	if (config_->getMoveCursorByHorizontalScroll()) {
		if (int dif = num - calculateColNumInRow(curPos_.track, curPos_.colInTrack))
			moveCursorToRight(dif);
	}
	else {
		if (int dif = num - leftTrackNum_)
			moveViewToRight(dif);
	}
}

void PatternEditorPanel::onVScrollBarChanged(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	// Skip if position has already changed in panel
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

	int step = std::min(curPos_.step, static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, num)) - 1);
	int dif = calculateStepDistance(curPos_.order, curPos_.step, num, step);
	moveCursorToDown(dif);
}

void PatternEditorPanel::onOrderListEdited()
{
	// Reset position memory
	hovPos_ = { -1, -1, -1, -1 };
	mousePressPos_ = { -1, -1, -1, -1 };
	mouseReleasePos_ = { -1, -1, -1, -1 };
	selLeftAbovePos_ = { -1, -1, -1, -1 };
	selRightBelowPos_ = { -1, -1, -1, -1 };
	shiftPressedPos_ = { -1, -1, -1, -1 };
	selectAllState_ = -1;
	emit selected(false);

	redrawByPatternChanged(true);
}

void PatternEditorPanel::onDefaultPatternSizeChanged()
{
	// Check pattern size
	int end = static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order));
	if (curPos_.step >= end) curPos_.step = end - 1;

	redrawByPatternChanged(true);
}

void PatternEditorPanel::setPatternHighlight1Count(int count)
{
	hl1Cnt_ = count;

	backChanged_ = true;
	textChanged_ = true;
	repaint();
}

void PatternEditorPanel::setPatternHighlight2Count(int count)
{
	hl2Cnt_ = count;

	backChanged_ = true;
	textChanged_ = true;
	repaint();
}

void PatternEditorPanel::setEditableStep(int n)
{
	editableStepCnt_ = n;
}

void PatternEditorPanel::onSongLoaded()
{
	// Initialize cursor position
	curSongNum_ = bt_->getCurrentSongNumber();
	curPos_ = {
		bt_->getCurrentTrackAttribute().number,
		0,
		bt_->getCurrentOrderNumber(),
		bt_->getCurrentStepNumber()
	};
	songStyle_ = bt_->getSongStyle(curSongNum_);
	size_t trackCnt = songStyle_.trackAttribs.size();
	rightEffn_ = std::vector<int>(trackCnt);
	std::generate(rightEffn_.begin(), rightEffn_.end(), [&, i = 0]() mutable {
		return static_cast<int>(bt_->getEffectDisplayWidth(curSongNum_, i++)); });
	leftTrackNum_ = 0;
	updateTracksWidthFromLeftToEnd();

	hovPos_ = { -1, -1, -1, -1 };
	mousePressPos_ = { -1, -1, -1, -1 };
	mouseReleasePos_ = { -1, -1, -1, -1 };
	selLeftAbovePos_ = { -1, -1, -1, -1 };
	selRightBelowPos_ = { -1, -1, -1, -1 };
	shiftPressedPos_ = { -1, -1, -1, -1 };
	markerPos_ = { -1, -1, -1, -1 };
	entryCnt_ = 0;
	selectAllState_ = -1;
	emit selected(false);

	redrawAll();
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
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
			break;
		case 1:
			bt_->eraseStepInstrument(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new EraseInstrumentInStepQtCommand(this));
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
			break;
		case 2:
			bt_->eraseStepVolume(curSongNum_, curPos_.track, curPos_.order, curPos_.step);
			comStack_.lock()->push(new EraseVolumeInStepQtCommand(this));
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
			break;
		case 3:
		case 5:
		case 7:
		case 9:
			bt_->eraseStepEffect(curSongNum_, curPos_.track, curPos_.order, curPos_.step,
								 (curPos_.colInTrack - 3) / 2);
			comStack_.lock()->push(new EraseEffectInStepQtCommand(this));
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
			break;
		case 4:
		case 6:
		case 8:
		case 10:
			bt_->eraseStepEffectValue(curSongNum_, curPos_.track, curPos_.order, curPos_.step,
									  (curPos_.colInTrack - 4) / 2);
			comStack_.lock()->push(new EraseEffectValueInStepQtCommand(this));
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(editableStepCnt_);
			break;
		}
	}
}

void PatternEditorPanel::onPastePressed()
{
	if (!bt_->isJamMode()) pasteCopiedCells(curPos_);
}

void PatternEditorPanel::onPasteMixPressed()
{
	if (!bt_->isJamMode()) pasteMixCopiedCells(curPos_);
}

void PatternEditorPanel::onPasteOverwritePressed()
{
	if (!bt_->isJamMode()) pasteOverwriteCopiedCells(curPos_);
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
		backChanged_ = true;
		repaint();
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
}

void PatternEditorPanel::onNoteTransposePressed(int seminote)
{
	if (bt_->isJamMode()) return;

	if (selLeftAbovePos_.order != -1)
		transposeNote(selLeftAbovePos_, selRightBelowPos_, seminote);
	else
		transposeNote(curPos_, curPos_, seminote);
}

void PatternEditorPanel::onToggleTrackPressed(int track)
{
	bt_->setTrackMuteState(track, !bt_->isMute(track));
	isMuteElse_ = false;
	redrawByMaskChanged();
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
	redrawByMaskChanged();
}

void PatternEditorPanel::onUnmuteAllPressed()
{
	int trackCnt = static_cast<int>(songStyle_.trackAttribs.size());
	for (int t = 0; t < trackCnt; ++t)
		bt_->setTrackMuteState(t, false);
	isMuteElse_ = false;
	redrawByMaskChanged();
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

void PatternEditorPanel::onExpandEffectColumnPressed(int trackNum)
{
	size_t tn = static_cast<size_t>(trackNum);
	if (rightEffn_.at(tn) == 3) return;
	bt_->setEffectDisplayWidth(curSongNum_, trackNum, static_cast<size_t>(++rightEffn_[tn]));
	updateTracksWidthFromLeftToEnd();

	if (config_->getMoveCursorByHorizontalScroll()) {
		emit effectColsCompanded(calculateColNumInRow(curPos_.track, curPos_.colInTrack), getFullColmunSize());
	}
	else {
		emit effectColsCompanded(leftTrackNum_, getScrollableCountByTrack());
	}

	redrawAll();
}

void PatternEditorPanel::onShrinkEffectColumnPressed(int trackNum)
{
	size_t tn = static_cast<size_t>(trackNum);
	if (rightEffn_.at(tn) == 0) return;
	bt_->setEffectDisplayWidth(curSongNum_, trackNum, static_cast<size_t>(--rightEffn_[tn]));
	updateTracksWidthFromLeftToEnd();

	if (config_->getMoveCursorByHorizontalScroll()) {
		emit effectColsCompanded(calculateColNumInRow(curPos_.track, curPos_.colInTrack), getFullColmunSize());
	}
	else {
		emit effectColsCompanded(leftTrackNum_, getScrollableCountByTrack());
	}

	redrawAll();
}

void PatternEditorPanel::onFollowModeChanged()
{
	curPos_.setRows(bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());

	emit vScrollBarChangeRequested(
				curPos_.step, static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, curPos_.order)) - 1);

	// Force redraw all area
	followModeChanged_ = true;
	redrawPatterns();
}

void PatternEditorPanel::onChangeValuesPressed(int value)
{
	if (bt_->isJamMode()) return;

	if (selLeftAbovePos_.order != -1)
		changeValuesInPattern(selLeftAbovePos_, selRightBelowPos_, value);
	else
		changeValuesInPattern(curPos_, curPos_, value);
}

void PatternEditorPanel::onShortcutUpdated()
{
	keyOff_->setKey(strToKeySeq(config_->getKeyOffKey()));
	echoBuf_->setKey(strToKeySeq(config_->getEchoBufferKey()));
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
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		if (event->modifiers().testFlag(Qt::AltModifier)) {
			if (bt_->isJamMode()) {
				return false;
			}
			else {
				deletePreviousStep();
				return true;
			}
		}
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
	case Qt::Key_Down:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		if (event->modifiers().testFlag(Qt::AltModifier)) {
			if (bt_->isJamMode()) {
				return false;
			}
			else {
				insertStep();
				moveCursorToDown(1);
				return true;
			}
		}
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
	case Qt::Key_Tab:
		if (curPos_.track == static_cast<int>(songStyle_.trackAttribs.size()) - 1) {
			if (config_->getWarpCursor())
				moveCursorToRight(-calculateColNumInRow(curPos_.track, curPos_.colInTrack));
		}
		else {
			moveCursorToRight(5 + 2 * rightEffn_[static_cast<size_t>(curPos_.track)] - curPos_.colInTrack);
		}
		return true;
	case Qt::Key_Backtab:
		if (curPos_.track == 0) {
			if (config_->getWarpCursor())
				moveCursorToRight(getFullColmunSize() - 1);
		}
		else {
			moveCursorToRight(-5 - 2 * rightEffn_[static_cast<size_t>(curPos_.track) - 1] - curPos_.colInTrack);
		}
		return true;
	case Qt::Key_Home:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(-curPos_.step);
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_End:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
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
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(-static_cast<int>(config_->getPageJumpLength()));
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_PageDown:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(static_cast<int>(config_->getPageJumpLength()));
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
			if (!config_->getKeyRepetition() && event->isAutoRepeat()) return false;
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
	if (bt_) {
		// Check order size
		int odrSize = static_cast<int>(bt_->getOrderSize(curSongNum_));
		if (curPos_.order >= odrSize) curPos_.setRows(odrSize - 1, 0);

		const QRect& area = event->rect();
		if (area.x() == 0 && area.y() == 0) {
			drawPattern(area);
		}
		else {
			drawPattern(rect());
		}
	}
}

void PatternEditorPanel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowY_ = (geometry().height() + headerHeight_ - stepFontHeight_) >> 1;
	curRowBaselineY_ = curRowY_ + stepFontAscent_ - (stepFontLeading_ >> 1);

	initDisplay();

	redrawAll();
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
					+ hdMuteToggleWidth_ + stepFontWidth_ / 2;
			if (w < event->pos().x() && event->pos().x() < w + hdEffCompandButtonWidth_ + stepFontWidth_) {
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
		if (mousePressPos_.track < 0 || mousePressPos_.order < 0) return;	// Start point is out of range

		if (hovPos_.track >= 0 && hovPos_.order >= 0) {
			setSelectedRectangle(mousePressPos_, hovPos_);
		}

		if (event->x() < stepNumWidth_ && leftTrackNum_ > 0) {
			if (config_->getMoveCursorByHorizontalScroll()) {
				moveCursorToRight(-(5 + 2 * rightEffn_.at(static_cast<size_t>(leftTrackNum_) - 1)));
			}
			else {
				moveViewToRight(-1);
			}
		}
		else if (event->x() > geometry().width() - stepNumWidth_ && hovPos_.track != -1) {
			if (config_->getMoveCursorByHorizontalScroll()) {
				moveCursorToRight(5 + 2 * rightEffn_.at(static_cast<size_t>(leftTrackNum_)));
			}
			else {
				moveViewToRight(1);
			}
		}
		if (event->pos().y() < headerHeight_ + stepFontHeight_) {
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(-1);
		}
		else if (event->pos().y() > geometry().height() - stepFontHeight_) {
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(1);
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
				if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(verDif);
			}
			else if (hovPos_.order == -2 && hovPos_.track >= 0) {	// Header
				if (isPressedPlus_) {
					onExpandEffectColumnPressed(hovPos_.track);
				}
				else if (isPressedMinus_) {
					onShrinkEffectColumnPressed(hovPos_.track);
				}
				else {
					onToggleTrackPressed(hovPos_.track);
					int horDif = calculateColumnDistance(curPos_.track, curPos_.colInTrack,
														 hovPos_.track, 0);
					moveCursorToRight(horDif);
				}
			}
			else if (hovPos_.track == -2 && hovPos_.order >= 0 && hovPos_.step >= 0) {	// Step number
				if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
					int verDif = calculateStepDistance(curPos_.order, curPos_.step,
													   hovPos_.order, hovPos_.step);
					moveCursorToDown(verDif);
				}
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

	case Qt::XButton1:
	{
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
			int order = curPos_.order - 1;
			if (order < 0) order = static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1;
			int step = std::min(
						   curPos_.step,
						   static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, order)) - 1);
			int d = calculateStepDistance(curPos_.order, curPos_.step, order, step);
			moveCursorToDown(d);
		}
		break;
	}

	case Qt::XButton2:
	{
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
			int order = curPos_.order + 1;
			if (static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1 < order) order = 0;
			int step = std::min(
						   curPos_.step,
						   static_cast<int>(bt_->getPatternSizeFromOrderNumber(curSongNum_, order)) - 1);
			int d = calculateStepDistance(curPos_.order, curPos_.step, order, step);
			moveCursorToDown(d);
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
			if (!config_->getDontSelectOnDoubleClick()) {
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

	if (hovPos_ != oldPos) redrawByHoverChanged();

	return true;
}

void PatternEditorPanel::wheelEvent(QWheelEvent *event)
{
	if (bt_->isPlaySong() && bt_->isFollowPlay()) return;
	int cnt = event->angleDelta().y() / 120;
	if (event->modifiers().testFlag(Qt::ControlModifier)) {
		onNoteTransposePressed(cnt);
	}
	else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
		onChangeValuesPressed(cnt);
	}
	else {
		moveCursorToDown(-cnt);
	}
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

	Q_UNUSED(delay)

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
