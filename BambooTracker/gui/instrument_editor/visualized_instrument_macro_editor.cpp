#include "visualized_instrument_macro_editor.hpp"
#include "ui_visualized_instrument_macro_editor.h"
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <utility>
#include "gui/event_guard.hpp"

VisualizedInstrumentMacroEditor::VisualizedInstrumentMacroEditor(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::VisualizedInstrumentMacroEditor),
	  maxDispRowCnt_(0),
	  colCnt_(0),
	  upperRow_(-1),
	  defaultRow_(0),
	  hovRow_(-1),
	  hovCol_(-1),
	  pressRow_(-1),
	  pressCol_(-1),
	  grabLoop_(-1),
	  isGrabLoopHead_(false),
	  isGrabRelease_(false),
	  release_{ VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE, -1 },
	  isIgnoreEvent_(false)
{
	ui->setupUi(this);

	/* Font */
	font_ = QApplication::font();
	font_.setPointSize(10);
	// Check font size
	QFontMetrics metrics(font_);
	fontWidth_ = metrics.width('0');
	fontAscend_ = metrics.ascent();
	fontHeight_ = metrics.height();
	fontLeading_ = metrics.leading();

	/* Width & height */
	tagWidth_ = metrics.width("Release ");

	/* Color */
	loopBackColor_ = QColor::fromRgb(25, 25, 25);
	releaseBackColor_ = QColor::fromRgb(0, 0, 0);
	loopColor_ = QColor::fromRgb(210, 40, 180, 127);
	releaseColor_ = QColor::fromRgb(40, 170, 200, 127);
	loopEdgeColor_ = QColor::fromRgb(180, 20, 180, 127);
	releaseEdgeColor_ = QColor::fromRgb(40, 170, 150, 127);
	tagColor_ = QColor::fromRgb(255, 255, 255);
	hovColor_ = QColor::fromRgb(255, 255, 255, 63);
	loopFontColor_ = QColor::fromRgb(24,223,172);
	releaseFontColor_ = QColor::fromRgb(24,223,172);
	cellColor_ = QColor::fromRgb(38, 183, 173);
	cellTextColor_ = QColor::fromRgb(255, 255, 255);
	borderColor_ = QColor::fromRgb(50, 50, 50);
	maskColor_ = QColor::fromRgb(0, 0, 0, 128);

	ui->panel->setAttribute(Qt::WA_Hover);
	ui->verticalScrollBar->setVisible(false);
	ui->panel->installEventFilter(this);
}

VisualizedInstrumentMacroEditor::~VisualizedInstrumentMacroEditor()
{
	delete ui;
}

void VisualizedInstrumentMacroEditor::AddRow(QString label)
{
	labels_.push_back(label);
	if (labels_.size() <= maxDispRowCnt_) {
		upperRow_ = labels_.size() - 1;
		ui->verticalScrollBar->setVisible(false);
		ui->verticalScrollBar->setMaximum(0);
	}
	else {
		ui->verticalScrollBar->setVisible(true);
		ui->verticalScrollBar->setMaximum(labels_.size() - maxDispRowCnt_);
	}
	updateRowHeight(ui->panel->geometry().height());
}

void VisualizedInstrumentMacroEditor::setMaximumDisplayedRowCount(int count)
{
	maxDispRowCnt_ = count;
	if (labels_.size() <= maxDispRowCnt_) {
		upperRow_ = labels_.size() - 1;
		ui->verticalScrollBar->setVisible(false);
		ui->verticalScrollBar->setMaximum(0);
	}
	else {
		ui->verticalScrollBar->setVisible(true);
		ui->verticalScrollBar->setMaximum(labels_.size() - maxDispRowCnt_);
	}
	updateRowHeight(ui->panel->geometry().height());
}

void VisualizedInstrumentMacroEditor::setDefaultRow(int row)
{
	defaultRow_ = row;
}

int VisualizedInstrumentMacroEditor::getSequenceLength() const
{
	return cols_.size();
}

void VisualizedInstrumentMacroEditor::setSequenceCommand(int row, int col, QString str, int data)
{
	cols_.at(col).row = row;
	cols_.at(col).text = str;
	cols_.at(col).data = data;

	ui->panel->update();

	emit sequenceCommandChanged(row, col);
}

void VisualizedInstrumentMacroEditor::setText(int col, QString text)
{
	cols_.at(col).text = text;
}

void VisualizedInstrumentMacroEditor::setData(int col, int data)
{
	cols_.at(col).data = data;
}

int VisualizedInstrumentMacroEditor::getSequenceAt(int col) const
{
	return cols_.at(col).row;
}

int VisualizedInstrumentMacroEditor::getSequenceDataAt(int col) const
{
	return cols_.at(col).data;
}

void VisualizedInstrumentMacroEditor::addSequenceCommand(int row, QString str, int data)
{
	++colCnt_;

	updateColumnWidth(ui->panel->geometry().width());
	cols_.push_back({ row, data, str });
	ui->panel->update();

	ui->colSizeLabel->setText("Size: " + QString::number(colCnt_));

	emit sequenceCommandAdded(row, cols_.size() - 1);
}

void VisualizedInstrumentMacroEditor::removeSequenceCommand()
{
	if (colCnt_ == 1) return;

	--colCnt_;

	// Modify loop
	for (size_t i = 0; i < loops_.size();) {
		if (loops_[i].begin >= colCnt_) {
			loops_.erase(loops_.begin() + i);
		}
		else {
			if (loops_[i].end >= colCnt_)
				loops_[i].end = colCnt_ - 1;
			++i;
		}
	}

	// Modify release
	if (release_.point >= colCnt_)
		release_.point = -1;

	updateColumnWidth(ui->panel->geometry().width());
	cols_.pop_back();
	ui->panel->update();

	ui->colSizeLabel->setText("Size: " + QString::number(colCnt_));

	emit sequenceCommandRemoved();
}

void VisualizedInstrumentMacroEditor::addLoop(int begin, int end, int times)
{
	size_t inx = 0;

	for (size_t i = 0; i < loops_.size(); ++i) {
		if (loops_[i].begin > begin) {
			break;
		}
		++inx;
	}

	loops_.insert(loops_.begin() + inx, { begin, end, times });

	onLoopChanged();
}

void VisualizedInstrumentMacroEditor::setRelease(ReleaseType type, int point)
{
	release_ = { type, point };
}

void VisualizedInstrumentMacroEditor::clear()
{
	cols_.clear();
	loops_.clear();
	release_ = { VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE, -1 };
	colCnt_ = 0;
	updateColumnWidth(ui->panel->geometry().width());
}

/******************************/
void VisualizedInstrumentMacroEditor::initDisplay()
{
	pixmap_ = std::make_unique<QPixmap>(ui->panel->geometry().size());
}

void VisualizedInstrumentMacroEditor::drawField()
{
	QPainter painter(pixmap_.get());
	painter.setFont(font_);

	// Row label
	painter.setPen(tagColor_);
	for (int i = 1; i <= maxDispRowCnt_; ++i) {
		painter.drawText(1, rowHeight_ * i - fontHeight_ + fontAscend_ + fontLeading_ / 2,
						 labels_[upperRow_ - i + 1]);
	}

	// Sequence
	painter.setPen(cellTextColor_);
	for (size_t i = 0; i < cols_.size(); ++i) {
		if (upperRow_ >= cols_[i].row && cols_[i].row > upperRow_ - maxDispRowCnt_) {
			int x = tagWidth_ + colWidth_ * i;
			int y = rowHeight_ * (upperRow_ - cols_[i].row);
			painter.fillRect(x, y, colWidth_, rowHeight_, cellColor_);
			painter.drawText(x + 2,
							 y + rowHeight_ - fontHeight_ + fontAscend_ + fontLeading_ / 2,
							 cols_[i].text);
		}
	}

	if (hovCol_ >= 0 && hovRow_ >= 0) {
		painter.fillRect(tagWidth_ + colWidth_ * hovCol_, rowHeight_ * (upperRow_ - hovRow_),
						 colWidth_, rowHeight_, hovColor_);
	}
}

void VisualizedInstrumentMacroEditor::drawLoop()
{
	QPainter painter(pixmap_.get());
	painter.setFont(font_);

	painter.fillRect(0, loopY_, ui->panel->geometry().width(), fontHeight_, loopBackColor_);
	painter.setPen(loopFontColor_);
	painter.drawText(1, loopBaseY_, "Loop");

	int w = tagWidth_;
	for (int i = 0; i < colCnt_; ++i) {
		for (size_t j = 0; j < loops_.size(); ++j) {
			if (loops_[j].begin <= i && i <= loops_[j].end) {
				painter.fillRect(w, loopY_, colWidth_, fontHeight_, loopColor_);
				if (loops_[j].begin == i) {
					painter.fillRect(w, loopY_, 2, fontHeight_, loopEdgeColor_);
					QString times = (loops_[j].times == 1) ? "" : QString::number(loops_[j].times);
					painter.drawText(w + 2, loopBaseY_, "Loop " + times);
				}
				if (loops_[j].end == i) {
					painter.fillRect(w + colWidth_ - 2, loopY_, 2, fontHeight_, loopEdgeColor_);
				}
			}
		}
		if (hovRow_ == -2 && hovCol_ == i)
			painter.fillRect(w, loopY_, colWidth_, fontHeight_, hovColor_);
		w += colWidth_;
	}
}

void VisualizedInstrumentMacroEditor::drawRelease()
{
	QPainter painter(pixmap_.get());
	painter.setFont(font_);

	painter.fillRect(0, releaseY_, ui->panel->geometry().width(), fontHeight_, releaseBackColor_);
	painter.setPen(releaseFontColor_);
	painter.drawText(1, releaseBaseY_, "Release");

	int w = tagWidth_;
	for (int i = 0; i < colCnt_; ++i) {
		if (release_.point == i) {
			painter.fillRect(w, releaseY_, ui->panel->geometry().width() - w, fontHeight_, releaseColor_);
			painter.fillRect(w, releaseY_, 2, fontHeight_, releaseEdgeColor_);
			QString type;
			switch (release_.type) {
			case VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE:
				type = "";
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::FIX:
				type = "Fix";
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE:
				type = "Absolute";
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE:
				type = "Relative";
				break;
			}
			painter.setPen(releaseFontColor_);
			painter.drawText(w + 2, releaseBaseY_, type);
		}
		if (hovRow_ == -3 && hovCol_ == i)
			painter.fillRect(w, releaseY_, colWidth_, fontHeight_, hovColor_);
		w += colWidth_;
	}
}

void VisualizedInstrumentMacroEditor::drawBorder()
{
	QPainter painter(pixmap_.get());
	painter.setPen(borderColor_);
	painter.drawLine(tagWidth_, 0, tagWidth_, ui->panel->geometry().height());
	for (int i = 1; i < maxDispRowCnt_; ++i) {
		painter.drawLine(tagWidth_, rowHeight_ * i, ui->panel->geometry().width(), rowHeight_ * i);
	}
}

void VisualizedInstrumentMacroEditor::drawShadow()
{
	QPainter painter(pixmap_.get());
	painter.fillRect(0, 0, ui->panel->geometry().width(), ui->panel->geometry().height(), maskColor_);
}

int VisualizedInstrumentMacroEditor::checkLoopRegion(int col)
{
	int ret = -1;

	for (size_t i = 0; i < loops_.size(); ++i) {
		if (loops_[i].begin <= col) {
			if (loops_[i].end >= col) {
				ret = i;
			}
		}
		else {
			break;
		}
	}

	return ret;
}

void VisualizedInstrumentMacroEditor::moveLoop()
{
	if (hovCol_ < 0) return;

	if (isGrabLoopHead_) {
		if (hovCol_ < loops_[grabLoop_].begin) {
			if (grabLoop_ > 0 && loops_[grabLoop_ - 1].end >= hovCol_) {
				loops_[grabLoop_].begin = loops_[grabLoop_ - 1].end + 1;
			}
			else {
				loops_[grabLoop_].begin = hovCol_;
			}
		}
		else if (hovCol_ > loops_[grabLoop_].begin) {
			if (hovCol_ > loops_[grabLoop_].end) {
				loops_.erase(loops_.begin() + grabLoop_);
			}
			else {
				loops_[grabLoop_].begin = hovCol_;
			}
		}
	}
	else {
		if (hovCol_ < loops_[grabLoop_].end) {
			if (hovCol_ < loops_[grabLoop_].begin) {
				loops_.erase(loops_.begin() + grabLoop_);
			}
			else {
				loops_[grabLoop_].end = hovCol_;
			}
		}
		else if (hovCol_ > loops_[grabLoop_].end) {
			if (grabLoop_ < loops_.size() - 1 && loops_[grabLoop_ + 1].begin <= hovCol_) {
				loops_[grabLoop_].end = loops_[grabLoop_ + 1].begin - 1;
			}
			else {
				loops_[grabLoop_].end = hovCol_;
			}
		}
	}
}

/********** Events **********/
bool VisualizedInstrumentMacroEditor::eventFilter(QObject*object, QEvent* event)
{
	if (object->objectName() == "panel") {
		switch (event->type()) {
			case QEvent::Paint:
			paintEventInView(dynamic_cast<QPaintEvent*>(event));
			return false;
		case QEvent::Resize:
			resizeEventInView(dynamic_cast<QResizeEvent*>(event));
			return false;
		case QEvent::MouseButtonPress:
			if (isEnabled())
				mousePressEventInView(dynamic_cast<QMouseEvent*>(event));
			return false;
		case QEvent::MouseButtonDblClick:
			if (isEnabled())
				mousePressEventInView(dynamic_cast<QMouseEvent*>(event));
			return false;
		case QEvent::MouseButtonRelease:
			if (isEnabled())
				mouseReleaseEventInView(dynamic_cast<QMouseEvent*>(event));
			return false;
		case QEvent::MouseMove:
			if (isEnabled())
				mouseMoveEventInView();
			return true;
		case QEvent::HoverMove:
			mouseHoverdEventInView(dynamic_cast<QHoverEvent*>(event));
			return false;
		case QEvent::Leave:
			leaveEventInView();
			return false;
		case QEvent::Wheel:
			wheelEventInView(dynamic_cast<QWheelEvent*>(event));
			return false;
		default:
			return false;
		}
	}

	return QWidget::eventFilter(object, event);
}

void VisualizedInstrumentMacroEditor::paintEventInView(QPaintEvent* event)
{
	pixmap_->fill(Qt::black);

	drawField();

	drawLoop();
	drawRelease();
	drawBorder();
	if (!isEnabled()) drawShadow();

	QPainter painter(ui->panel);
	painter.drawPixmap(event->rect(), *pixmap_.get());
}

void VisualizedInstrumentMacroEditor::resizeEventInView(QResizeEvent* event)
{
	updateRowHeight(ui->panel->geometry().height());
	updateColumnWidth(ui->panel->geometry().width());

	releaseY_ = ui->panel->geometry().height() - fontHeight_;
	releaseBaseY_ = releaseY_ + fontAscend_ + fontLeading_ / 2;
	loopY_ = releaseY_ - fontHeight_;
	loopBaseY_ = releaseBaseY_ - fontHeight_;

	fieldHeight_ = loopY_;

	initDisplay();
}

void VisualizedInstrumentMacroEditor::mousePressEventInView(QMouseEvent* event)
{
	if (!colCnt_) return;

	pressRow_ = hovRow_;
	pressCol_ = hovCol_;

	// Check grab
	int x = event->pos().x();
	if (hovRow_ == -2) {
		if (event->button() == Qt::LeftButton) {
			for (int col = 0, w = tagWidth_; col < colCnt_; ++col, w += colWidth_) {
				if (w - 4 < x && x < w + 4) {
					for (size_t i = 0; i < loops_.size(); ++i) {
						if (loops_[i].begin == col) {
							grabLoop_ = i;
							isGrabLoopHead_ = true;
						}
						else if (loops_[i].begin > col) {
							break;
						}
					}
				}
				else if (w + colWidth_ - 4 < x && x < w + colWidth_ + 4) {
					for (size_t i = 0; i < loops_.size(); ++i) {
						if (loops_[i].end == col) {
							grabLoop_ = i;
							isGrabLoopHead_ = false;
						}
						else if (loops_[i].end > col) {
							break;
						}
					}
				}
			}
		}
	}
	else if (hovRow_ == -3 && release_.point != -1) {
		if (event->button() == Qt::LeftButton) {
			int w = tagWidth_ + colWidth_ * release_.point;
			if (w - 4 < x && x < w + 4) {
				isGrabRelease_ = true;
			}
		}
	}

	// Press process
	if (pressCol_ > -1) {
		if (pressRow_ == -2) {
			if (grabLoop_ == -1) {
				int i = checkLoopRegion(pressCol_);
				switch (event->button()) {
				case Qt::LeftButton:
				{
					if (i == -1) {	// New loop
						addLoop(pressCol_, pressCol_, 1);
					}
					else {	// Loop count up
						++loops_[i].times;
						onLoopChanged();
					}
					break;
				}
				case Qt::RightButton:
				{
					if (i > -1) {	// Loop count down
						if (loops_[i].times > 1) {
							--loops_[i].times;
						}
						else {	// Erase loop
							loops_.erase(loops_.begin() + i);
						}
						onLoopChanged();
					}
					break;
				}
				default:
					break;
				}
			}
		}
		else if (pressRow_ == -3) {
			if (!isGrabRelease_) {
				switch (event->button()) {
				case Qt::LeftButton:
				{
					if (release_.point == -1 || pressCol_ < release_.point) {	// New release
						release_.type = (release_.type == VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE)
										? VisualizedInstrumentMacroEditor::ReleaseType::FIX
										: release_.type;
						release_.point = pressCol_;
					}
					else {	// Change release type
						switch (release_.type) {
						case VisualizedInstrumentMacroEditor::ReleaseType::FIX:
							release_.type = VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE;
							break;
						case VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE:
							release_.type = VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE;
							break;
						case VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE:
						case VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE:
							release_.type = VisualizedInstrumentMacroEditor::ReleaseType::FIX;
							break;
						}
					}
					emit releaseChanged(release_.type, release_.point);
					break;
				}
				case Qt::RightButton:
				{
					if (pressCol_ >= release_.point) {	// Erase release
						release_.point = -1;
						emit releaseChanged(release_.type, release_.point);
					}
					break;
				}
				default:
					break;
				}
			}
		}
		else {
			setSequenceCommand(hovRow_, hovCol_);
		}
	}

	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::mouseReleaseEventInView(QMouseEvent* event)
{
	if (!colCnt_) return;

	if (grabLoop_ != -1) {	// Move loop
		if (event->button() == Qt::LeftButton) {
			moveLoop();
			onLoopChanged();
		}
	}
	else if (isGrabRelease_) {	// Move release
		if (event->button() == Qt::LeftButton) {
			if (hovCol_ > -1) {
				release_.point = hovCol_;
				emit releaseChanged(release_.type, release_.point);
			}
		}
	}

	pressRow_ = -1;
	pressCol_ = -1;
	grabLoop_ = -1;
	isGrabLoopHead_ = false;
	isGrabRelease_ = false;

	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::mouseMoveEventInView()
{
	if (!colCnt_) return;

	if (pressRow_ >= 0 && pressCol_ >= 0 && hovRow_ >= 0 && hovCol_ >= 0) {
		if (cols_[hovCol_].row != hovRow_) setSequenceCommand(hovRow_, hovCol_);
	}
}

void VisualizedInstrumentMacroEditor::mouseHoverdEventInView(QHoverEvent* event)
{
	if (!colCnt_) return;

	int oldCol = hovCol_;
	int oldRow = hovRow_;

	QPoint pos = event->pos();

	// Detect column
	if (pos.x() < tagWidth_) {
		hovCol_ = -2;
	}
	else {
		hovCol_ = 0;
		for(int w = tagWidth_ + colWidth_; w < pos.x(); w += colWidth_, ++hovCol_)
			;
		if (hovCol_ >= colCnt_) hovCol_ = -1;	// Out of range
	}

	// Detect row
	if (releaseY_ < pos.y()) {
		hovRow_ = -3;
	}
	else if (loopY_ < pos.y()) {
		hovRow_ = -2;
	}
	else {
		for (int i = 0, w = rowHeight_; i < maxDispRowCnt_; ++i, w += rowHeight_) {
			if (pos.y() < w) {
				hovRow_ = upperRow_ - i;
				break;
			}
		}
	}

	if (hovRow_ != oldRow || hovCol_ != oldCol) ui->panel->update();
}

void VisualizedInstrumentMacroEditor::leaveEventInView()
{
	hovRow_ = -1;
	hovCol_ = -1;

	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::wheelEventInView(QWheelEvent* event)
{
	if (!colCnt_) return;

	Ui::EventGuard eg(isIgnoreEvent_);
	int degree = event->angleDelta().y() / 8;
	int pos = ui->verticalScrollBar->value() + degree / 15;
	if (0 > pos) pos = 0;
	else if (pos > labels_.size() - maxDispRowCnt_) pos = labels_.size() - maxDispRowCnt_;
	scrollUp(pos);
	ui->panel->update();

	ui->verticalScrollBar->setValue(pos);
}

void VisualizedInstrumentMacroEditor::on_colIncrToolButton_clicked()
{
	addSequenceCommand(defaultRow_);
}

void VisualizedInstrumentMacroEditor::on_colDecrToolButton_clicked()
{
	removeSequenceCommand();
}

void VisualizedInstrumentMacroEditor::on_verticalScrollBar_valueChanged(int value)
{
	if (!isIgnoreEvent_) {
		scrollUp(value);
		ui->panel->update();
	}
}

void VisualizedInstrumentMacroEditor::onLoopChanged()
{
	std::vector<int> begins, ends, times;
	for (auto& l : loops_) {
		begins.push_back(l.begin);
		ends.push_back(l.end);
		times.push_back(l.times);
	}

	emit loopChanged(std::move(begins), std::move(ends), std::move(times));
}
