#include "visualized_instrument_macro_editor.hpp"
#include "ui_visualized_instrument_macro_editor.h"
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <algorithm>
#include <numeric>
#include <utility>
#include "gui/event_guard.hpp"

VisualizedInstrumentMacroEditor::VisualizedInstrumentMacroEditor(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::VisualizedInstrumentMacroEditor),
	  maxDispRowCnt_(0),
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
	  isMultiReleaseState_(false),
	  isLabelOmitted_(false),
	  mmlBase_(0),
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
	tagWidth_ = metrics.width(VisualizedInstrumentMacroEditor::tr("Release "));

	ui->panel->setAttribute(Qt::WA_Hover);
	ui->verticalScrollBar->setVisible(false);
	ui->panel->installEventFilter(this);
}

VisualizedInstrumentMacroEditor::~VisualizedInstrumentMacroEditor()
{
	delete ui;
}

void VisualizedInstrumentMacroEditor::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
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
		int max = labels_.size() - maxDispRowCnt_;
		ui->verticalScrollBar->setMaximum(max);
		ui->verticalScrollBar->setValue(max);
	}
	updateRowHeight();
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
		int max = labels_.size() - maxDispRowCnt_;
		ui->verticalScrollBar->setMaximum(max);
		ui->verticalScrollBar->setValue(max);
	}
	updateRowHeight();
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

	makeMML();

	emit sequenceCommandChanged(row, col);
}

void VisualizedInstrumentMacroEditor::setText(int col, QString text)
{
	cols_.at(col).text = text;
}

void VisualizedInstrumentMacroEditor::setData(int col, int data)
{
	cols_.at(col).data = data;
	makeMML();
}

int VisualizedInstrumentMacroEditor::getSequenceAt(int col) const
{
	return cols_.at(col).row;
}

int VisualizedInstrumentMacroEditor::getSequenceDataAt(int col) const
{
	return cols_.at(col).data;
}

void VisualizedInstrumentMacroEditor::setMultipleReleaseState(bool enabled)
{
	isMultiReleaseState_ = enabled;
}

void VisualizedInstrumentMacroEditor::addSequenceCommand(int row, QString str, int data)
{
	cols_.push_back({ row, data, str });

	updateColumnWidth();
	ui->panel->update();

	ui->colSizeLabel->setText(tr("Size: %1").arg(cols_.size()));

	makeMML();

	emit sequenceCommandAdded(row, cols_.size() - 1);
}

void VisualizedInstrumentMacroEditor::removeSequenceCommand()
{
	if (cols_.size() == 1) return;

	cols_.pop_back();

	// Modify loop
	for (size_t i = 0; i < loops_.size();) {
		if (loops_[i].begin >= cols_.size()) {
			loops_.erase(loops_.begin() + i);
		}
		else {
			if (loops_[i].end >= cols_.size())
				loops_[i].end = cols_.size() - 1;
			++i;
		}
	}

	// Modify release
	if (release_.point >= cols_.size())
		release_.point = -1;

	updateColumnWidth();
	ui->panel->update();

	ui->colSizeLabel->setText(tr("Size: %1").arg(cols_.size()));

	makeMML();

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

	makeMML();

	onLoopChanged();
}

void VisualizedInstrumentMacroEditor::setRelease(ReleaseType type, int point)
{
	release_ = { type, point };

	makeMML();
}

void VisualizedInstrumentMacroEditor::clearData()
{
	cols_.clear();
	loops_.clear();
	release_ = { VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE, -1 };
	updateColumnWidth();

	makeMML();
}

void VisualizedInstrumentMacroEditor::clearRow()
{
	labels_.clear();
}

void VisualizedInstrumentMacroEditor::setUpperRow(int row)
{
	upperRow_ = row;
	int pos = upperRow_ + 1 - getDisplayedRowCount();
	ui->panel->update();
	ui->verticalScrollBar->setValue(ui->verticalScrollBar->maximum() - pos);
}

void VisualizedInstrumentMacroEditor::setLabel(int row, QString text)
{
	labels_.at(row) = text;
	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::clearAllLabelText()
{
	std::fill(labels_.begin(), labels_.end(), "");
	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::setLabelDiaplayMode(bool isOmitted)
{
	isLabelOmitted_ = isOmitted;
	ui->panel->update();
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
	painter.setPen(palette_->instSeqTagColor);
	if (isLabelOmitted_ && !labels_.empty()) {
		painter.drawText(1,
						 rowHeights_.front() - fontHeight_ + fontAscend_ + fontLeading_ / 2,
						 labels_[upperRow_]);
		int c = getDisplayedRowCount() / 2;
		painter.drawText(1,
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + c + 1, 0)
						 - fontHeight_ + fontAscend_ + fontLeading_ / 2,
						 labels_[upperRow_ - c]);
		int l = getDisplayedRowCount() - 1;
		painter.drawText(1,
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + l + 1, 0)
						 - fontHeight_ + fontAscend_ + fontLeading_ / 2,
						 labels_[upperRow_ - l]);
	}
	else {
		for (int i = 0; i < maxDispRowCnt_; ++i) {
			painter.drawText(1,
							 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + i + 1, 0)
							 - fontHeight_ + fontAscend_ + fontLeading_ / 2,
							 labels_[upperRow_ - i]);
		}
	}

	// Sequence
	painter.setPen(palette_->instSeqCellTextColor);
	for (size_t i = 0; i < cols_.size(); ++i) {
		if (upperRow_ >= cols_[i].row && cols_[i].row > upperRow_ - maxDispRowCnt_) {
			int x = tagWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + i, 0);
			int y = std::accumulate(rowHeights_.begin(), rowHeights_.begin() + (upperRow_ - cols_[i].row), 0);
			painter.fillRect(x, y, colWidths_[i], rowHeights_[upperRow_ - cols_[i].row], palette_->instSeqCellColor);
			painter.drawText(x + 2,
							 y + rowHeights_[upperRow_ - cols_[i].row] - fontHeight_ + fontAscend_ + (fontLeading_ / 2),
							 cols_[i].text);
		}
	}

	if (hovCol_ >= 0 && hovRow_ >= 0) {
		painter.fillRect(tagWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + hovCol_, 0),
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + hovRow_, 0),
						 colWidths_[hovCol_], rowHeights_[hovRow_], palette_->instSeqHovColor);
	}
}

void VisualizedInstrumentMacroEditor::drawLoop()
{
	QPainter painter(pixmap_.get());
	painter.setFont(font_);

	painter.fillRect(0, loopY_, ui->panel->geometry().width(), fontHeight_, palette_->instSeqLoopBackColor);
	painter.setPen(palette_->instSeqLoopTextColor);
	painter.drawText(1, loopBaseY_, tr("Loop"));

	int w = tagWidth_;
	for (int i = 0; i < cols_.size(); ++i) {
		for (size_t j = 0; j < loops_.size(); ++j) {
			if (loops_[j].begin <= i && i <= loops_[j].end) {
				painter.fillRect(w, loopY_, colWidths_[i], fontHeight_, palette_->instSeqLoopColor);
				if (loops_[j].begin == i) {
					painter.fillRect(w, loopY_, 2, fontHeight_, palette_->instSeqLoopEdgeColor);
					QString times = (loops_[j].times == 1) ? "" : QString::number(loops_[j].times);
					painter.drawText(w + 2, loopBaseY_, tr("Loop %1").arg(times));
				}
				if (loops_[j].end == i) {
					painter.fillRect(w + colWidths_[i] - 2, loopY_, 2, fontHeight_, palette_->instSeqLoopEdgeColor);
				}
			}
		}
		if (hovRow_ == -2 && hovCol_ == i)
			painter.fillRect(w, loopY_, colWidths_[i], fontHeight_, palette_->instSeqHovColor);
		w += colWidths_[i];
	}
}

void VisualizedInstrumentMacroEditor::drawRelease()
{
	QPainter painter(pixmap_.get());
	painter.setFont(font_);

	painter.fillRect(0, releaseY_, ui->panel->geometry().width(), fontHeight_, palette_->instSeqReleaseBackColor);
	painter.setPen(palette_->instSeqReleaseTextColor);
	painter.drawText(1, releaseBaseY_, tr("Release"));

	int w = tagWidth_;
	for (int i = 0; i < cols_.size(); ++i) {
		if (release_.point == i) {
			painter.fillRect(w, releaseY_, ui->panel->geometry().width() - w, fontHeight_, palette_->instSeqReleaseColor);
			painter.fillRect(w, releaseY_, 2, fontHeight_, palette_->instSeqReleaseEdgeColor);
			QString type;
			switch (release_.type) {
			case VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE:
				type = "";
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::FIX:
				type = tr("Fix");
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE:
				type = tr("Absolute");
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE:
				type = tr("Relative");
				break;
			}
			painter.setPen(palette_->instSeqReleaseTextColor);
			painter.drawText(w + 2, releaseBaseY_, type);
		}
		if (hovRow_ == -3 && hovCol_ == i)
			painter.fillRect(w, releaseY_, colWidths_[i], fontHeight_, palette_->instSeqHovColor);
		w += colWidths_[i];
	}
}

void VisualizedInstrumentMacroEditor::drawBorder()
{
	QPainter painter(pixmap_.get());
	painter.setPen(palette_->instSeqBorderColor);
	painter.drawLine(tagWidth_, 0, tagWidth_, ui->panel->geometry().height());
	for (int i = 1; i < maxDispRowCnt_; ++i) {
		painter.drawLine(tagWidth_, std::accumulate(rowHeights_.begin(), rowHeights_.begin() + i, 0),
						 ui->panel->geometry().width(), std::accumulate(rowHeights_.begin(), rowHeights_.begin() + i, 0));
	}
}

void VisualizedInstrumentMacroEditor::drawShadow()
{
	QPainter painter(pixmap_.get());
	painter.fillRect(0, 0, ui->panel->geometry().width(), ui->panel->geometry().height(), palette_->instSeqMaskColor);
}

void VisualizedInstrumentMacroEditor::makeMML()
{
	if (cols_.empty()) return;

	QString text = "";
	std::vector<Loop> lstack;

	for (size_t cnt = 0; cnt < cols_.size(); ++cnt) {
		if (release_.point == cnt) {
			switch (release_.type) {
			case ReleaseType::FIX:		text += "| ";	break;
			case ReleaseType::ABSOLUTE:	text += "/ ";	break;
			case ReleaseType::RELATIVE:	text += ": ";	break;
			default:									break;
			}
		}

		for (size_t i = 0; i < loops_.size(); ++i) {
			if (loops_[i].begin == cnt) {
				lstack.push_back(loops_[i]);
				text += "[ ";
			}
			else if (loops_[i].begin > cnt) {
				break;
			}
		}

		text += (QString::number(cols_[cnt].row + mmlBase_) + " ");

		while (!lstack.empty()) {
			if (lstack.back().end == cnt) {
				text += "]";
				if (lstack.back().times > 1) {
					text += QString::number(lstack.back().times);
				}
				text += " ";
				lstack.pop_back();
			}
			else {
				break;
			}
		}
	}

	ui->lineEdit->setText(text);
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

	makeMML();
}

void VisualizedInstrumentMacroEditor::setMMLDisplay0As(int n)
{
	mmlBase_ = n;
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
	if (!palette_) return;

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
	updateRowHeight();
	updateColumnWidth();

	releaseY_ = ui->panel->geometry().height() - fontHeight_;
	releaseBaseY_ = releaseY_ + fontAscend_ + fontLeading_ / 2;
	loopY_ = releaseY_ - fontHeight_;
	loopBaseY_ = releaseBaseY_ - fontHeight_;

	fieldHeight_ = loopY_;

	initDisplay();
}

void VisualizedInstrumentMacroEditor::mousePressEventInView(QMouseEvent* event)
{
	if (!cols_.size()) return;

	pressRow_ = hovRow_;
	pressCol_ = hovCol_;

	// Check grab
	int x = event->pos().x();
	if (hovRow_ == -2) {
		if (event->button() == Qt::LeftButton) {
			for (int col = 0, w = tagWidth_; col < cols_.size(); ++col) {
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
				else if (w + colWidths_[col] - 4 < x && x < w + colWidths_[col] + 4) {
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
				w += colWidths_[col];
			}
		}
	}
	else if (hovRow_ == -3 && release_.point != -1) {
		if (event->button() == Qt::LeftButton) {
			int w = tagWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + release_.point, 0);
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
						makeMML();
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
						makeMML();
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
						makeMML();
						emit releaseChanged(release_.type, release_.point);
					}
					else if (isMultiReleaseState_) {	// Change release type
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
						makeMML();
						emit releaseChanged(release_.type, release_.point);
					}
					break;
				}
				case Qt::RightButton:
				{
					if (pressCol_ >= release_.point) {	// Erase release
						release_ = { VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE, -1 };
						makeMML();
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
			setSequenceCommand(upperRow_ - hovRow_, hovCol_);
		}
	}

	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::mouseReleaseEventInView(QMouseEvent* event)
{
	if (!cols_.size()) return;

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
				makeMML();
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
	if (!cols_.size()) return;

	if (pressRow_ >= 0 && pressCol_ >= 0 && hovRow_ >= 0 && hovCol_ >= 0) {
		if (cols_[upperRow_ - hovCol_].row != (upperRow_ - hovRow_))
			setSequenceCommand(upperRow_ - hovRow_, hovCol_);
	}
}

void VisualizedInstrumentMacroEditor::mouseHoverdEventInView(QHoverEvent* event)
{
	if (!cols_.size()) return;

	int oldCol = hovCol_;
	int oldRow = hovRow_;

	QPoint pos = event->pos();

	// Detect column
	if (pos.x() < tagWidth_) {
		hovCol_ = -2;
	}
	else {
		for (int i = 0, w = tagWidth_; i < cols_.size(); ++i) {
			w += colWidths_[i];
			if (pos.x() < w) {
				hovCol_ = i;
				break;
			}
		}
		if (hovCol_ >= cols_.size()) hovCol_ = -1;	// Out of range
	}

	// Detect row
	if (releaseY_ < pos.y()) {
		hovRow_ = -3;
	}
	else if (loopY_ < pos.y()) {
		hovRow_ = -2;
	}
	else {
		int cnt = getDisplayedRowCount();
		for (int i = 0, w = 0; i < cnt; ++i) {
			w += rowHeights_[i];
			if (pos.y() < w) {
				hovRow_ = i;
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
	if (!cols_.size()) return;

	Ui::EventGuard eg(isIgnoreEvent_);
	int degree = - event->angleDelta().y() / 8;
	int pos = ui->verticalScrollBar->value() + degree / 15;
	if (0 > pos) pos = 0;
	else if (pos > labels_.size() - maxDispRowCnt_) pos = labels_.size() - maxDispRowCnt_;
	scrollUp(ui->verticalScrollBar->maximum() - pos);
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
		scrollUp(ui->verticalScrollBar->maximum() - value);
		ui->panel->update();
	}
}

void VisualizedInstrumentMacroEditor::on_lineEdit_editingFinished()
{
	if (cols_.empty()) return;

	QString text = ui->lineEdit->text();

	std::vector<Column> column;
	std::vector<Loop> loop;
	std::vector<size_t> lstack;
	Release release = { ReleaseType::NO_RELEASE, -1 };

	int cnt = 0;
	while (!text.isEmpty()) {

		QRegularExpressionMatch m = QRegularExpression("^\\[").match(text);
		if (m.hasMatch()) {
			loop.push_back({ cnt, cnt, 1 });
			lstack.push_back(loop.size() - 1);
			text.remove(QRegularExpression("^\\["));
			continue;
		}

		m = QRegularExpression("^\\](\\d*)").match(text);
		if (m.hasMatch()) {
			if (lstack.empty() || cnt == 0) return;
			loop[lstack.back()].end = cnt - 1;
			if (!m.captured(1).isEmpty()) {
				int t = m.captured(1).toInt();
				if (t > 1) loop[lstack.back()].times = t;
				else return;
			}
			lstack.pop_back();
			text.remove(QRegularExpression("^\\]\\d*"));
			continue;
		}

		m = QRegularExpression("^\\|").match(text);
		if (m.hasMatch()) {
			if (release.point > -1) return;
			release = { ReleaseType::FIX, cnt };
			text.remove(QRegularExpression("^\\|"));
			continue;
		}

		m = QRegularExpression("^/").match(text);
		if (m.hasMatch()) {
			if (release.point > -1) return;
			release = { ReleaseType::ABSOLUTE, cnt };
			text.remove(QRegularExpression("^/"));
			continue;
		}

		m = QRegularExpression("^:").match(text);
		if (m.hasMatch()) {
			if (release.point > -1) return;
			release = { ReleaseType::RELATIVE, cnt };
			text.remove(QRegularExpression("^:"));
			continue;
		}

		m = QRegularExpression("^(-?\\d+)").match(text);
		if (m.hasMatch()) {
			int d = m.captured(1).toInt() - mmlBase_;
			if (d < 0 || labels_.size() <= d) return;
			column.push_back({ d, -1, "" });
			++cnt;
			text.remove(QRegularExpression("^-?\\d+"));
			continue;
		}

		m = QRegularExpression("^ +").match(text);
		if (m.hasMatch()) {
			text.remove(QRegularExpression("^ +"));
			continue;
		}

		return;
	}

	if (column.empty()) return;
	if (!lstack.empty()) return;
	if (release.point > -1 && release.point >= column.size()) return;

	while (cols_.size() > 1) removeSequenceCommand();
	setSequenceCommand(column.front().row, 0);
	for (size_t i = 1; i < column.size(); ++i) {
		addSequenceCommand(column[i].row);
	}

	loops_ = loop;
	onLoopChanged();

	release_ = release;
	emit releaseChanged(release.type, release.point);

	ui->panel->update();

	makeMML();
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

void VisualizedInstrumentMacroEditor::updateColumnWidth()
{
	colWidths_.clear();

	if (!cols_.size()) return;

	float ww = (ui->panel->geometry().width() - tagWidth_) / static_cast<float>(cols_.size());
	int w = static_cast<int>(ww);
	float dif = ww - w;
	float sum = 0;
	for (size_t i = 0; i < cols_.size(); ++i) {
		int width = w;
		sum += dif;
		if (sum >= 1.0f) {
			++width;
			sum -= 1.0;
		}
		colWidths_.push_back(width);
	}
}

void VisualizedInstrumentMacroEditor::updateRowHeight()
{
	rowHeights_.clear();

	if (!labels_.size()) return;

	int div = getDisplayedRowCount();
	float hh = (ui->panel->geometry().height() - fontHeight_ * 2) / static_cast<float>(div);
	int h = static_cast<int>(hh);
	float dif = hh - h;
	float sum = 0;

	for (int i = 0; i < div; ++i) {
		int height = h;
		sum += dif;
		if (sum >= 1.0f) {
			++height;
			sum -= 1.0;
		}
		rowHeights_.push_back(height);
	}
}
