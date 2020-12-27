/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "visualized_instrument_macro_editor.hpp"
#include "ui_visualized_instrument_macro_editor.h"
#include <algorithm>
#include <numeric>
#include <utility>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "gui/event_guard.hpp"

VisualizedInstrumentMacroEditor::VisualizedInstrumentMacroEditor(QWidget *parent)
	: QWidget(parent),
	  font_(QApplication::font()),
	  met_(font_),
	  maxDispRowCnt_(0),
	  upperRow_(-1),
	  defaultRow_(0),
	  hovRow_(-1),
	  hovCol_(-1),
	  type_(NoType),
	  permittedReleaseType_(ReleaseType::FIXED_RELEASE),
	  isLabelOmitted_(false),
	  release_{ ReleaseType::NO_RELEASE, -1 },
	  ui(new Ui::VisualizedInstrumentMacroEditor),
	  pressRow_(-1),
	  pressCol_(-1),
	  grabLoop_(-1),
	  isGrabLoopHead_(false),
	  isGrabRelease_(false),
	  isMultiReleaseState_(false),
	  mmlBase_(0),
	  isIgnoreEvent_(false)
{
	ui->setupUi(this);

	/* Font */
	font_.setPointSize(10);
	// Check font size
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	fontWidth_ = met_.horizontalAdvance('0');
#else
	fontWidth_ = met_.width('0');
#endif
	fontAscend_ = met_.ascent();
	fontHeight_ = met_.height();
	fontLeading_ = met_.leading();

	/* Width & height */
	autoFitLabelWidth();

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

void VisualizedInstrumentMacroEditor::AddRow(QString label, bool fitLabelWidth)
{
	labels_.push_back(label);
	if (static_cast<int>(labels_.size()) <= maxDispRowCnt_) {
		upperRow_ = static_cast<int>(labels_.size()) - 1;
		ui->verticalScrollBar->setVisible(false);
		ui->verticalScrollBar->setMaximum(0);
	}
	else {
		ui->verticalScrollBar->setVisible(true);
		int max = static_cast<int>(labels_.size()) - maxDispRowCnt_;
		ui->verticalScrollBar->setMaximum(max);
		ui->verticalScrollBar->setValue(max);
	}
	if (fitLabelWidth) autoFitLabelWidth();
	updateRowHeight();
}

void VisualizedInstrumentMacroEditor::setMaximumDisplayedRowCount(int count)
{
	maxDispRowCnt_ = count;
	if (static_cast<int>(labels_.size()) <= maxDispRowCnt_) {
		upperRow_ = static_cast<int>(labels_.size()) - 1;
		ui->verticalScrollBar->setVisible(false);
		ui->verticalScrollBar->setMaximum(0);
	}
	else {
		ui->verticalScrollBar->setVisible(true);
		int max = static_cast<int>(labels_.size()) - maxDispRowCnt_;
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
	return static_cast<int>(cols_.size());
}

void VisualizedInstrumentMacroEditor::setSequenceCommand(int row, int col, QString str, int data)
{
	size_t idx = static_cast<size_t>(col);
	cols_.at(idx).row = row;
	cols_.at(idx).text = str;
	cols_.at(idx).data = data;

	ui->panel->update();

	printMML();

	emit sequenceCommandChanged(row, col);
}

void VisualizedInstrumentMacroEditor::setText(int col, QString text)
{
	cols_.at(static_cast<size_t>(col)).text = text;
}

void VisualizedInstrumentMacroEditor::setData(int col, int data)
{
	cols_.at(static_cast<size_t>(col)).data = data;
	printMML();
}

int VisualizedInstrumentMacroEditor::getSequenceAt(int col) const
{
	return cols_.at(static_cast<size_t>(col)).row;
}

int VisualizedInstrumentMacroEditor::getSequenceDataAt(int col) const
{
	return cols_.at(static_cast<size_t>(col)).data;
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

	printMML();

	emit sequenceCommandAdded(row, static_cast<int>(cols_.size()) - 1);
}

void VisualizedInstrumentMacroEditor::removeSequenceCommand()
{
	if (cols_.size() == 1) return;

	cols_.pop_back();

	// Modify loop
	for (size_t i = 0; i < loops_.size();) {
		if (loops_[i].begin >= static_cast<int>(cols_.size())) {
			loops_.erase(loops_.begin() + static_cast<int>(i));
		}
		else {
			if (loops_[i].end >= static_cast<int>(cols_.size()))
				loops_[i].end = static_cast<int>(cols_.size()) - 1;
			++i;
		}
	}

	// Modify release
	if (release_.point >= static_cast<int>(cols_.size()))
		release_.point = -1;

	updateColumnWidth();
	ui->panel->update();

	ui->colSizeLabel->setText(tr("Size: %1").arg(cols_.size()));

	printMML();

	emit sequenceCommandRemoved();
}

void VisualizedInstrumentMacroEditor::addLoop(int begin, int end, int times)
{
	int inx = 0;

	for (size_t i = 0; i < loops_.size(); ++i) {
		if (loops_[i].begin > begin) {
			break;
		}
		++inx;
	}

	loops_.insert(loops_.begin() + inx, { begin, end, times });

	printMML();

	onLoopChanged();
}

void VisualizedInstrumentMacroEditor::setSequenceType(SequenceType type)
{
	type_ = type;

	updateLabels();
	printMML();
}

void VisualizedInstrumentMacroEditor::setPermittedReleaseTypes(int types)
{
	permittedReleaseType_ = types;
}

void VisualizedInstrumentMacroEditor::setRelease(ReleaseType type, int point)
{
	release_ = { type, point };

	printMML();
}

void VisualizedInstrumentMacroEditor::clearData()
{
	cols_.clear();
	loops_.clear();
	release_ = { ReleaseType::NO_RELEASE, -1 };
	updateColumnWidth();

	printMML();
}

void VisualizedInstrumentMacroEditor::clearRow()
{
	labels_.clear();
	autoFitLabelWidth();
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
	labels_.at(static_cast<size_t>(row)) = text;
	autoFitLabelWidth();
	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::clearAllLabelText()
{
	std::fill(labels_.begin(), labels_.end(), "");
	autoFitLabelWidth();
	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::setLabelDiaplayMode(bool isOmitted)
{
	isLabelOmitted_ = isOmitted;
	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::autoFitLabelWidth()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	int w = met_.horizontalAdvance(VisualizedInstrumentMacroEditor::tr("Release") + " ");
	for (auto& lab : labels_) w = std::max(w, met_.horizontalAdvance(lab + " "));
#else
	int w = met_.width(VisualizedInstrumentMacroEditor::tr("Release") + " ");
	for (auto& lab : labels_) w = std::max(w, met_.width(lab + " "));
#endif
	labWidth_ = w;
}

/******************************/
void VisualizedInstrumentMacroEditor::initDisplay()
{
	pixmap_ = QPixmap(ui->panel->geometry().size());
}

void VisualizedInstrumentMacroEditor::drawField()
{
	QPainter painter(&pixmap_);
	painter.setFont(font_);

	int textOffset = fontAscend_ - fontHeight_ + fontLeading_ / 2;

	// Row label
	painter.setPen(palette_->instSeqTagColor);
	int dispCnt = getDisplayedRowCount();
	if (isLabelOmitted_ && !labels_.empty()) {
		painter.drawText(1,
						 rowHeights_.front() + textOffset,
						 labels_[static_cast<size_t>(upperRow_)]);
		int c = dispCnt / 2;
		painter.drawText(1,
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + c + 1, 0) + textOffset,
						 labels_[static_cast<size_t>(upperRow_ - c)]);
		int l = dispCnt - 1;
		painter.drawText(1,
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + l + 1, 0) + textOffset,
						 labels_[static_cast<size_t>(upperRow_ - l)]);
	}
	else {
		for (int i = 0; i < dispCnt; ++i) {
			painter.drawText(1,
							 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + i + 1, 0) + textOffset,
							 labels_[static_cast<size_t>(upperRow_ - i)]);
		}
	}

	for (size_t i = 1; i < cols_.size(); i += 2) {
		painter.fillRect(labWidth_
						 + std::accumulate(colWidths_.begin(), colWidths_.begin() + static_cast<int>(i), 0),
						 0,
						 colWidths_[i], fieldHeight_,
						 palette_->instSeqOddColColor);
	}

	// Sequence
	painter.setPen(palette_->instSeqCellTextColor);
	for (size_t i = 0; i < cols_.size(); ++i) {
		if (upperRow_ >= cols_[i].row && cols_[i].row > upperRow_ - dispCnt) {
			int x = labWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + static_cast<int>(i), 0);
			int hi = upperRow_ - cols_[i].row;
			int y = std::accumulate(rowHeights_.begin(), rowHeights_.begin() + hi, 0);
			painter.fillRect(x, y, colWidths_[i],
							 rowHeights_[static_cast<size_t>(hi)], palette_->instSeqCellColor);
			painter.drawText(x + 2,
							 y + rowHeights_[static_cast<size_t>(upperRow_ - cols_[i].row)] + textOffset,
					cols_[i].text);
		}
	}

	if (hovCol_ >= 0 && hovRow_ >= 0) {
		painter.fillRect(labWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + hovCol_, 0),
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + hovRow_, 0),
						 colWidths_[static_cast<size_t>(hovCol_)], rowHeights_[static_cast<size_t>(hovRow_)],
				palette_->instSeqHovColor);
	}
}

void VisualizedInstrumentMacroEditor::drawLoop()
{
	QPainter painter(&pixmap_);
	painter.setFont(font_);

	painter.fillRect(0, loopY_, panelWidth(), fontHeight_, palette_->instSeqLoopBackColor);
	painter.setPen(palette_->instSeqLoopTextColor);
	painter.drawText(1, loopBaseY_, tr("Loop"));

	int w = labWidth_;
	int seqLen = static_cast<int>(cols_.size());
	for (int i = 0; i < seqLen; ++i) {
		for (size_t j = 0; j < loops_.size(); ++j) {
			if (loops_[j].begin <= i && i <= loops_[j].end) {
				painter.fillRect(w, loopY_, colWidths_[static_cast<size_t>(i)],
						fontHeight_, palette_->instSeqLoopColor);
				if (loops_[j].begin == i) {
					painter.fillRect(w, loopY_, 2, fontHeight_, palette_->instSeqLoopEdgeColor);
					QString times = (loops_[j].times == 1) ? "" : QString::number(loops_[j].times);
					painter.drawText(w + 2, loopBaseY_, tr("Loop %1").arg(times));
				}
				if (loops_[j].end == i) {
					painter.fillRect(w + colWidths_[static_cast<size_t>(i)] - 2, loopY_, 2,
							fontHeight_, palette_->instSeqLoopEdgeColor);
				}
			}
		}

		if (hovRow_ == -2 && hovCol_ == i)
			painter.fillRect(w, loopY_, colWidths_[static_cast<size_t>(i)],
					fontHeight_, palette_->instSeqHovColor);

		w += colWidths_[static_cast<size_t>(i)];
	}
}

void VisualizedInstrumentMacroEditor::drawRelease()
{
	QPainter painter(&pixmap_);
	painter.setFont(font_);

	painter.fillRect(0, releaseY_, panelWidth(), fontHeight_, palette_->instSeqReleaseBackColor);
	painter.setPen(palette_->instSeqReleaseTextColor);
	painter.drawText(1, releaseBaseY_, tr("Release"));

	int w = labWidth_;
	int seqLen = static_cast<int>(cols_.size());
	for (int i = 0; i < seqLen; ++i) {
		if (release_.point == i) {
			painter.fillRect(w, releaseY_, panelWidth() - w, fontHeight_, palette_->instSeqReleaseColor);
			painter.fillRect(w, releaseY_, 2, fontHeight_, palette_->instSeqReleaseEdgeColor);
			QString type;
			switch (release_.type) {
			case ReleaseType::NO_RELEASE:
				type = "";
				break;
			case ReleaseType::FIXED_RELEASE:
				type = tr("Fixed");
				break;
			case ReleaseType::ABSOLUTE_RELEASE:
				type = tr("Absolute");
				break;
			case ReleaseType::RELATIVE_RELEASE:
				type = tr("Relative");
				break;
			}
			painter.setPen(palette_->instSeqReleaseTextColor);
			painter.drawText(w + 2, releaseBaseY_, type);
		}

		if (hovRow_ == -3 && hovCol_ == i)
			painter.fillRect(w, releaseY_, colWidths_[static_cast<size_t>(i)],
					fontHeight_, palette_->instSeqHovColor);

		w += colWidths_[static_cast<size_t>(i)];
	}
}

void VisualizedInstrumentMacroEditor::drawBorder()
{
	QPainter painter(&pixmap_);
	painter.setPen(palette_->instSeqBorderColor);
	painter.drawLine(labWidth_, 0, labWidth_, ui->panel->geometry().height());
	for (int i = 1; i < maxDispRowCnt_; ++i) {
		painter.drawLine(labWidth_, std::accumulate(rowHeights_.begin(), rowHeights_.begin() + i, 0),
						 panelWidth(), std::accumulate(rowHeights_.begin(), rowHeights_.begin() + i, 0));
	}
}

void VisualizedInstrumentMacroEditor::drawShadow()
{
	QPainter painter(&pixmap_);
	painter.fillRect(0, 0, panelWidth(), ui->panel->geometry().height(), palette_->instSeqMaskColor);
}

void VisualizedInstrumentMacroEditor::updateLabels()
{
}

void VisualizedInstrumentMacroEditor::printMML()
{
	if (cols_.empty()) return;

	QString text = "";
	std::vector<Loop> lstack;

	int seqLen = static_cast<int>(cols_.size());
	for (int cnt = 0; cnt < seqLen; ++cnt) {
		if (release_.point == cnt) {
			switch (release_.type) {
			case ReleaseType::FIXED_RELEASE:	text += "| ";	break;
			case ReleaseType::ABSOLUTE_RELEASE:	text += "/ ";	break;
			case ReleaseType::RELATIVE_RELEASE:	text += ": ";	break;
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

		text += (convertSequenceDataUnitToMML(cols_[static_cast<size_t>(cnt)]) + " ");

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

QString VisualizedInstrumentMacroEditor::convertSequenceDataUnitToMML(Column col)
{
	return QString::number(col.row + mmlBase_);
}

void VisualizedInstrumentMacroEditor::interpretMML()
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

		if (permittedReleaseType_ & ReleaseType::FIXED_RELEASE) {
			m = QRegularExpression("^\\|").match(text);
			if (m.hasMatch()) {
				if (release.point > -1) return;
				release = { ReleaseType::FIXED_RELEASE, cnt };
				text.remove(QRegularExpression("^\\|"));
				continue;
			}
		}

		if (permittedReleaseType_ & ReleaseType::ABSOLUTE_RELEASE) {
			m = QRegularExpression("^/").match(text);
			if (m.hasMatch()) {
				if (release.point > -1) return;
				release = { ReleaseType::ABSOLUTE_RELEASE, cnt };
				text.remove(QRegularExpression("^/"));
				continue;
			}
		}

		if (permittedReleaseType_ & ReleaseType::RELATIVE_RELEASE) {
			m = QRegularExpression("^:").match(text);
			if (m.hasMatch()) {
				if (release.point > -1) return;
				release = { ReleaseType::RELATIVE_RELEASE, cnt };
				text.remove(QRegularExpression("^:"));
				continue;
			}
		}

		if (interpretSlopeInMML(text, cnt, column)) continue;	// Slope

		if (interpretDataInMML(text, cnt, column)) continue;	// Data

		m = QRegularExpression("^ +").match(text);
		if (m.hasMatch()) {
			text.remove(QRegularExpression("^ +"));
			continue;
		}

		return;
	}

	if (column.empty()) return;
	if (!lstack.empty()) return;
	if (release.point > -1 && release.point >= static_cast<int>(column.size())) return;

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
}

bool VisualizedInstrumentMacroEditor::interpretSlopeInMML(QString& text, int& cnt, std::vector<Column>& column)
{
	QRegularExpressionMatch m = QRegularExpression("^(?<start>-?\\d+) *_ *(?<end>-?\\d+)( *_ *(?<diff>-?\\d+))?").match(text);
	if (m.hasMatch()) {
		int start = m.captured("start").toInt();
		int end = m.captured("end").toInt();
		QString diffStr = m.captured("diff");
		if (start < 0 || maxInMML() <= start || end < 0 || maxInMML() <= end || start == end)
			return false;
		int diff;
		int d = start;
		if (start < end) {
			if (diffStr.isEmpty()) {
				diff = 1;
			}
			else {
				diff = diffStr.toInt();
				if (diff <= 0) return false;
			}
			for (bool flag = true; flag ; ) {
				if (end <= d) {
					flag = false;
					d = end;
				}
				column.push_back({ d, -1, "" });
				d += diff;
				++cnt;
			}
		}
		else {
			if (diffStr.isEmpty()) {
				diff = -1;
			}
			else {
				diff = diffStr.toInt();
				if (0 <= diff) return false;
			}
			for (bool flag = true; flag ; ) {
				if (d <= end) {
					flag = false;
					d = end;
				}
				column.push_back({ d, -1, "" });
				d += diff;
				++cnt;
			}
		}
		text.remove(QRegularExpression("^-?\\d+ *_ *-?\\d+( *_ *-?\\d+)?"));
		return true;
	}
	return false;
}

bool VisualizedInstrumentMacroEditor::interpretDataInMML(QString& text, int& cnt, std::vector<Column>& column)
{
	QRegularExpressionMatch m = QRegularExpression("^(-?\\d+)").match(text);
	if (m.hasMatch()) {
		int d = m.captured(1).toInt() - mmlBase_;
		if (d < 0 || maxInMML() <= d) return false;
		column.push_back({ d, -1, "" });
		++cnt;
		text.remove(QRegularExpression("^-?\\d+"));
		return true;
	}
	return false;
}

int VisualizedInstrumentMacroEditor::checkLoopRegion(int col)
{
	int ret = -1;

	for (size_t i = 0; i < loops_.size(); ++i) {
		if (loops_[i].begin <= col) {
			if (loops_[i].end >= col) {
				ret = static_cast<int>(i);
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

	size_t idx = static_cast<size_t>(grabLoop_);
	if (isGrabLoopHead_) {
		if (hovCol_ < loops_[idx].begin) {
			if (grabLoop_ > 0 && loops_[idx - 1].end >= hovCol_) {
				loops_[idx].begin = loops_[idx - 1].end + 1;
			}
			else {
				loops_[idx].begin = hovCol_;
			}
		}
		else if (hovCol_ > loops_[idx].begin) {
			if (hovCol_ > loops_[idx].end) {
				loops_.erase(loops_.begin() + grabLoop_);
			}
			else {
				loops_[idx].begin = hovCol_;
			}
		}
	}
	else {
		if (hovCol_ < loops_[idx].end) {
			if (hovCol_ < loops_[idx].begin) {
				loops_.erase(loops_.begin() + grabLoop_);
			}
			else {
				loops_[idx].end = hovCol_;
			}
		}
		else if (hovCol_ > loops_[idx].end) {
			if (grabLoop_ < static_cast<int>(loops_.size()) - 1 && loops_[idx + 1].begin <= hovCol_) {
				loops_[idx].end = loops_[idx + 1].begin - 1;
			}
			else {
				loops_[idx].end = hovCol_;
			}
		}
	}

	printMML();
}

void VisualizedInstrumentMacroEditor::setMMLDisplay0As(int n)
{
	mmlBase_ = n;
}

int VisualizedInstrumentMacroEditor::panelWidth() const
{
	return ui->panel->geometry().width();
}

/********** Events **********/
bool VisualizedInstrumentMacroEditor::eventFilter(QObject*object, QEvent* event)
{
	if (object == ui->panel) {
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

	pixmap_.fill(Qt::black);

	drawField();

	drawLoop();
	drawRelease();
	drawBorder();
	if (!isEnabled()) drawShadow();

	QPainter painter(ui->panel);
	painter.drawPixmap(event->rect(), pixmap_, event->rect());
}

void VisualizedInstrumentMacroEditor::resizeEventInView(QResizeEvent* event)
{
	Q_UNUSED(event)

	updateRowHeight();
	updateColumnWidth();

	releaseY_ = ui->panel->geometry().height() - fontHeight_;
	releaseBaseY_ = releaseY_ + fontAscend_ + fontLeading_ / 2;
	loopY_ = releaseY_ - fontHeight_;
	loopBaseY_ = releaseBaseY_ - fontHeight_;

	fieldHeight_ = loopY_;

	initDisplay();
}

int VisualizedInstrumentMacroEditor::detectRowNumberForMouseEvent(int col, int internalRow) const
{
	Q_UNUSED(col)
	return upperRow_ - internalRow;
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
			int seqLen = static_cast<int>(cols_.size());
			for (int col = 0, w = labWidth_; col < seqLen; ++col) {
				if (w - 4 < x && x < w + 4) {
					for (size_t i = 0; i < loops_.size(); ++i) {
						if (loops_[i].begin == col) {
							grabLoop_ = static_cast<int>(i);
							isGrabLoopHead_ = true;
						}
						else if (loops_[i].begin > col) {
							break;
						}
					}
				}
				else if (w + colWidths_[static_cast<size_t>(col)] - 4 < x
						 && x < w + colWidths_[static_cast<size_t>(col)] + 4) {
					for (size_t i = 0; i < loops_.size(); ++i) {
						if (loops_[i].end == col) {
							grabLoop_ = static_cast<int>(i);
							isGrabLoopHead_ = false;
						}
						else if (loops_[i].end > col) {
							break;
						}
					}
				}
				w += colWidths_[static_cast<size_t>(col)];
			}
		}
	}
	else if (hovRow_ == -3 && release_.point != -1) {
		if (event->button() == Qt::LeftButton) {
			int w = labWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + release_.point, 0);
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
						++loops_[static_cast<size_t>(i)].times;
						printMML();
						onLoopChanged();
					}
					break;
				}
				case Qt::RightButton:
				{
					if (i > -1) {	// Loop count down
						if (loops_[static_cast<size_t>(i)].times > 1) {
							--loops_[static_cast<size_t>(i)].times;
						}
						else {	// Erase loop
							loops_.erase(loops_.begin() + i);
						}
						printMML();
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
						release_.type = (release_.type == ReleaseType::NO_RELEASE)
										? ReleaseType::FIXED_RELEASE
										: release_.type;
						release_.point = pressCol_;
						printMML();
						emit releaseChanged(release_.type, release_.point);
					}
					else if (isMultiReleaseState_) {	// Change release type
						switch (release_.type) {
						case ReleaseType::FIXED_RELEASE:
							release_.type = ReleaseType::ABSOLUTE_RELEASE;
							break;
						case ReleaseType::ABSOLUTE_RELEASE:
							release_.type = ReleaseType::RELATIVE_RELEASE;
							break;
						case ReleaseType::NO_RELEASE:
						case ReleaseType::RELATIVE_RELEASE:
							release_.type = ReleaseType::FIXED_RELEASE;
							break;
						}
						printMML();
						emit releaseChanged(release_.type, release_.point);
					}
					break;
				}
				case Qt::RightButton:
				{
					if (pressCol_ >= release_.point) {	// Erase release
						release_ = { ReleaseType::NO_RELEASE, -1 };
						printMML();
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
			setSequenceCommand(detectRowNumberForMouseEvent(hovCol_, hovRow_), hovCol_);
			prevPressRow_ = hovRow_;
			prevPressCol_ = hovCol_;
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
				printMML();
				emit releaseChanged(release_.type, release_.point);
			}
		}
	}

	pressRow_ = -1;
	pressCol_ = -1;
	prevPressRow_ = -1;
	prevPressCol_ = -1;
	grabLoop_ = -1;
	isGrabLoopHead_ = false;
	isGrabRelease_ = false;

	ui->panel->update();
}

void VisualizedInstrumentMacroEditor::mouseMoveEventInView()
{
	if (!cols_.size()) return;

	if (pressRow_ >= 0 && pressCol_ >= 0 && hovRow_ >= 0 && hovCol_ >= 0) {
		if (prevPressRow_ != hovRow_ || prevPressCol_ != hovCol_) {
			prevPressRow_ = hovRow_;
			prevPressCol_ = hovCol_;
			setSequenceCommand(detectRowNumberForMouseEvent(hovCol_, hovRow_), hovCol_);
		}
	}
}

void VisualizedInstrumentMacroEditor::mouseHoverdEventInView(QHoverEvent* event)
{
	if (!cols_.size()) return;

	int oldCol = hovCol_;
	int oldRow = hovRow_;

	QPoint pos = event->pos();

	// Detect column
	if (pos.x() < labWidth_) {
		hovCol_ = -2;
	}
	else {
		int seqLen = static_cast<int>(cols_.size());
		for (int i = 0, w = labWidth_; i < seqLen; ++i) {
			w += colWidths_[static_cast<size_t>(i)];
			if (pos.x() < w) {
				hovCol_ = i;
				break;
			}
		}
		if (hovCol_ >= seqLen) hovCol_ = -1;	// Out of range
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
			w += rowHeights_[static_cast<size_t>(i)];
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
	int labCnt = static_cast<int>(labels_.size());
	if (0 > pos) pos = 0;
	else if (pos > labCnt - maxDispRowCnt_) pos = labCnt - maxDispRowCnt_;
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

int VisualizedInstrumentMacroEditor::maxInMML() const
{
	return static_cast<int>(labels_.size());
}

void VisualizedInstrumentMacroEditor::on_lineEdit_editingFinished()
{
	interpretMML();
	printMML();
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

	float ww = (panelWidth() - labWidth_) / static_cast<float>(cols_.size());
	int w = static_cast<int>(ww);
	float dif = ww - w;
	float sum = 0;
	for (size_t i = 0; i < cols_.size(); ++i) {
		int width = w;
		sum += dif;
		if (sum >= 1.0f) {
			++width;
			sum -= 1.0f;
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
			sum -= 1.0f;
		}
		rowHeights_.push_back(height);
	}
}
