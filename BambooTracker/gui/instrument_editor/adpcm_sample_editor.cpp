/*
 * Copyright (C) 2020-2021 Rerrah
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

#include "adpcm_sample_editor.hpp"
#include "ui_adpcm_sample_editor.h"
#include <cmath>
#include <limits>
#include <vector>
#include <set>
#include <algorithm>
#include <QMimeData>
#include <QFile>
#include <QIODevice>
#include <QFileInfo>
#include <QFileDialog>
#include <QStringList>
#include <QPainter>
#include <QRect>
#include <QRectF>
#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QWheelEvent>
#include <QHoverEvent>
#include "chip/codec/ymb_codec.hpp"
#include "instrument/sample_adpcm.hpp"
#include "gui/event_guard.hpp"
#include "gui/instrument_editor/sample_length_dialog.hpp"
#include "gui/instrument_editor/grid_settings_dialog.hpp"
#include "gui/file_io_error_message_box.hpp"
#include "gui/instrument_editor/instrument_editor_utils.hpp"
#include "utils.hpp"

ADPCMSampleEditor::ADPCMSampleEditor(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ADPCMSampleEditor),
	isIgnoreEvent_(false),
	zoom_(0),
	viewedSampLen_(2),
	gridIntr_(1),
	cursorSamp_(0, 0),
	prevPressedSamp_(-1, -1),
	addrStart_(0),
	addrStop_(0),
	sample_(2),
	drawMode_(DrawMode::Disabled)
{
	ui->setupUi(this);

	auto rkfunc = [&](int dummy) {
		Q_UNUSED(dummy)
		if (!isIgnoreEvent_) {
			int rk = ui->rootKeySpinBox->value() * 12 + ui->rootKeyComboBox->currentIndex();
			bt_.lock()->setSampleADPCMRootKeyNumber(ui->sampleNumSpinBox->value(), rk);
			emit sampleParameterChanged(ui->sampleNumSpinBox->value());
			emit modified();
		}
	};
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->rootKeyComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, rkfunc);
	QObject::connect(ui->rootKeySpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, rkfunc);

	ui->memoryWidget->installEventFilter(this);
	ui->sampleViewWidget->setAttribute(Qt::WA_Hover);
	ui->sampleViewWidget->installEventFilter(this);

	auto tb = new QToolBar();
	tb->setIconSize(QSize(16, 16));
	tb->setMinimumHeight(28);
	tb->setMaximumHeight(28);
	tb->addActions({ ui->action_Clear, ui->action_Import, ui->action_Resize });
	tb->addSeparator();
	tb->addActions({ ui->actionZoom_In, ui->actionZoom_Out });
	auto gridMenu = new QMenu();
	gridMenu->addActions({ ui->action_Grid_View, ui->actionG_rid_Settings });
	auto gridButton = new QToolButton();
	gridButton->setPopupMode(QToolButton::MenuButtonPopup);
	gridButton->setMenu(gridMenu);
	gridButton->setDefaultAction(ui->action_Grid_View);
	tb->addWidget(gridButton);
	tb->addSeparator();
	auto editMenu = new QMenu();
	editMenu->addActions({ ui->action_Draw_Sample, ui->actionDirec_t_Draw });
	auto editButton = new QToolButton();
	editButton->setPopupMode(QToolButton::MenuButtonPopup);
	editButton->setMenu(editMenu);
	editButton->setDefaultAction(ui->action_Draw_Sample);
	QObject::connect(editButton, &QToolButton::triggered, editButton, &QToolButton::setDefaultAction);
	tb->addWidget(editButton);
	tb->addAction(ui->actionRe_verse);
	ui->verticalLayout_2->insertWidget(0, tb);
}

ADPCMSampleEditor::~ADPCMSampleEditor()
{
	delete ui;
}

void ADPCMSampleEditor::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
}

void ADPCMSampleEditor::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
}

void ADPCMSampleEditor::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
}

int ADPCMSampleEditor::getSampleNumber() const
{
	return ui->sampleNumSpinBox->value();
}

void ADPCMSampleEditor::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mime = event->mimeData();
	if (mime->hasUrls() && mime->urls().length() == 1
			&& QFileInfo(mime->urls().at(0).toLocalFile()).suffix().toLower() == "wav") {
		event->acceptProposedAction();
	}
}

bool ADPCMSampleEditor::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == ui->memoryWidget) {
		switch (ev->type()) {
		case QEvent::Paint:
		{
			QPainter painter(ui->memoryWidget);
			painter.drawPixmap(ui->memoryWidget->rect(), memPixmap_);
			break;
		}
		case QEvent::Resize:
			memPixmap_ = QPixmap(ui->memoryWidget->size());
			updateSampleMemoryBar();
			break;
		default:
			break;
		}
		return false;
	}

	if (obj == ui->sampleViewWidget) {
		switch (ev->type()) {
		case QEvent::Paint:
		{
			QPainter painter(ui->sampleViewWidget);
			painter.drawPixmap(ui->sampleViewWidget->rect(), sampViewPixmap_);
			break;
		}
		case QEvent::Resize:
			sampViewPixmap_ = QPixmap(ui->sampleViewWidget->size());
			updateSampleView();
			break;
		case QEvent::Wheel:
		{
			auto we = dynamic_cast<QWheelEvent*>(ev);
			int cnt = we->angleDelta().y() / 120;
			bool ctrl = we->modifiers().testFlag(Qt::ControlModifier);
			if (cnt > 0) {
				for (int i = 0; i < cnt; ++i) {
					if (ctrl) on_actionZoom_In_triggered();
					else ui->horizontalScrollBar->setValue(ui->horizontalScrollBar->value() - 1);
				}
			}
			else {
				for (int i = 0; cnt < i; --i) {
					if (ctrl) on_actionZoom_Out_triggered();
					else ui->horizontalScrollBar->setValue(ui->horizontalScrollBar->value() + 1);
				}
			}
			break;
		}
		case QEvent::HoverMove:
		{
			auto pos = dynamic_cast<QHoverEvent*>(ev)->pos();
			detectCursorSamplePosition(pos.x(), pos.y());

			if (prevPressedSamp_.x() != -1) {	// Change sample
				const int px = prevPressedSamp_.x();
				const int py = prevPressedSamp_.y();
				const int cx = cursorSamp_.x();
				const int cy = cursorSamp_.y();
				if (px < cx) {
					const int dx = cx - px;
					const int dy = cy - py;
					for (int x = px + 1; x <= cx; ++x)
						sample_.at(x) = (x - px) * dy / dx + py;
				}
				else if (px == cx) {
					sample_.at(cx) = cy;
				}
				else {
					const int dx = px - cx;
					const int dy = py - cy;
					for (int x = cx; x < px; ++x)
						sample_.at(x) = (x - cx) * dy / dx + cy;
				}
				prevPressedSamp_ = cursorSamp_;

				if (drawMode_ == DrawMode::Direct) {
					sendEditedSample();
				}
				else {
					updateSampleView();
					ui->sampleViewWidget->update();
				}
			}
			break;
		}
		case QEvent::MouseButtonPress:
		{
			if (drawMode_ == DrawMode::Disabled) break;
			sample_.at(cursorSamp_.x()) = cursorSamp_.y();
			prevPressedSamp_ = cursorSamp_;

			if (drawMode_ == DrawMode::Direct) {
				sendEditedSample();
			}
			else {
				updateSampleView();
				ui->sampleViewWidget->update();
			}
			break;
		}
		case QEvent::MouseButtonRelease:
		{
			prevPressedSamp_.setX(-1);	// Clear
			break;
		}
		default:
			break;
		}
		return false;
	}

	return false;
}

void ADPCMSampleEditor::dropEvent(QDropEvent* event)
{
	importSampleFrom(reinterpret_cast<QDropEvent*>(event)->mimeData()->urls().first().toLocalFile());
}

void ADPCMSampleEditor::setInstrumentSampleParameters(int sampNum, bool repeatable, int rKeyNum,
													  int rDeltaN, size_t start, size_t stop,
													  std::vector<uint8_t> sample)
{
	Ui::EventGuard ev(isIgnoreEvent_);

	ui->sampleNumSpinBox->setValue(sampNum);

	updateUsersView();

	ui->repeatCheckBox->setChecked(repeatable);
	ui->rootKeyComboBox->setCurrentIndex(rKeyNum % 12);
	ui->rootKeySpinBox->setValue(rKeyNum / 12);
	ui->rootRateSpinBox->setValue(rDeltaN);

	addrStart_ = start;
	addrStop_ = stop;
	updateSampleMemoryBar();
	ui->memoryWidget->update();

	if (!ui->action_Draw_Sample->isChecked()) {
		sample_.resize(sample.size() * 2);
		codec::ymb_decode(sample.data(), sample_.data(), static_cast<long>(sample_.size()));

		// Slider settings
		for (int z = 0, len = sample_.size(); ; ++z) {
			int tmp = (len + 1) >> 1;
			if (tmp < 2) {
				zoom_ = z;
				viewedSampLen_ = len;
				ui->horizontalScrollBar->setMaximum(sample_.size() - len);
				break;
			}
			else if (z == zoom_) {
				viewedSampLen_ = len;
				ui->horizontalScrollBar->setMaximum(sample_.size() - len);
				break;
			}
			else {
				len = tmp;
			}
		}
		updateSampleView();
		ui->sampleViewWidget->update();
	}

	ui->detailLabel->setText(updateDetailView());
}

void ADPCMSampleEditor::importSampleFrom(const QString file)
{
	std::unique_ptr<io::WavContainer> wav;
	try {
		io::BinaryContainer container;
		{
			QFile fp(file);
			if (!fp.open(QIODevice::ReadOnly)) {
				FileIOErrorMessageBox::openError(file, true, io::FileType::WAV, this);
				return;
			}
			QByteArray array = fp.readAll();
			fp.close();
			std::move(array.begin(), array.end(), std::back_inserter(container));
		}

		wav = std::make_unique<io::WavContainer>(container);
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(file, true, e, this).exec();
		return;
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(file, true, io::FileType::WAV, QString(e.what()), this).exec();
		return;
	}

	if (wav->getSampleRate() < 2000 || 55466 < wav->getSampleRate()) {
		FileIOErrorMessageBox(file, true, io::FileType::WAV,
							  tr("Supported sample rate is 2kHz-55.5kHz, but the rate of selected sample is %1.")
							  .arg(wav->getSampleRate()), this).exec();
		return;
	}

	if (wav->getChannelCount() != 1) {
		FileIOErrorMessageBox(file, true, io::FileType::WAV,
							  tr("The selected sample is not mono channel."), this).exec();
		return;
	}

	io::BinaryContainer bc = wav->getSample();
	size_t rawSize = bc.size() / 2;
	std::vector<int16_t> raw(rawSize);
	for (size_t i = 0; i < rawSize; ++i) {
		raw[i] = bc.readInt16(i * 2);
	}
	std::vector<uint8_t> adpcm((raw.size() + 1) / 2);
	codec::ymb_encode(raw.data(), adpcm.data(), static_cast<long>(raw.size()));

	bt_.lock()->storeSampleADPCMRawSample(ui->sampleNumSpinBox->value(), std::move(adpcm));
	ui->rootKeyComboBox->setCurrentIndex(SampleADPCM::DEF_ROOT_KEY % 12);
	ui->rootKeySpinBox->setValue(SampleADPCM::DEF_ROOT_KEY / 12);
	ui->rootRateSpinBox->setValue(SampleADPCM::calculateADPCMDeltaN(wav->getSampleRate()));

	emit modified();
	emit sampleAssignRequested();
	emit sampleParameterChanged(ui->sampleNumSpinBox->value());
}

void ADPCMSampleEditor::updateSampleMemoryBar()
{
	QRect bar = memPixmap_.rect();
	if (!bar.isValid()) return;

	QPainter painter(&memPixmap_);
	painter.fillRect(bar, palette_->instADPCMMemBackColor);

	double maxSize = bt_.lock()->getADPCMStoredSize() >> 5;
	double limit = bt_.lock()->getADPCMLimit() >> 5;	// By 32 bytes
	QRectF allSamp(0, 0, std::max(1., bar.width() * (maxSize / limit)), rect().height());
	painter.fillRect(allSamp, palette_->instADPCMMemAllColor);

	if (addrStart_ || addrStart_ != addrStop_) {
		QRectF curSamp(bar.width() * (addrStart_ / limit),
					   0, std::max(1., bar.width() * ((addrStop_ - addrStart_) / limit)), rect().height());
		painter.fillRect(curSamp, palette_->instADPCMMemCurColor);
	}
}

void ADPCMSampleEditor::updateSampleView()
{
	QRect rect = sampViewPixmap_.rect();
	if (!rect.isValid()) return;

	QPainter painter(&sampViewPixmap_);
	painter.fillRect(rect, palette_->instADPCMSampViewBackColor);

	painter.setPen(palette_->instADPCMSampViewCenterColor);
	const int maxX = rect.width();
	const int centerY = rect.height() >> 1;
	painter.drawLine(0, centerY, maxX, centerY);

	QColor foreColor;
	switch (drawMode_) {
	case DrawMode::Disabled:	foreColor = palette_->instADPCMSampViewForeColor;		break;
	case DrawMode::Normal:		foreColor = palette_->instADPCMSampViewDrawColor;		break;
	case DrawMode::Direct:		foreColor = palette_->instADPCMSampViewDirectDrawColor;	break;
	}
	painter.setPen(foreColor);
	const int16_t maxY = std::numeric_limits<int16_t>::max();
	const size_t first = ui->horizontalScrollBar->value();
	const bool showGrid = ui->action_Grid_View->isChecked();
	if (maxX < viewedSampLen_) {
		int prevY = centerY;
		size_t g = first;
		for (int x = 0; x < maxX; ++x) {
			size_t i = (viewedSampLen_ - 1) * x / (maxX - 1);
			size_t p = i + first;
			int16_t sample = sample_.at(p);
			int y = centerY - (centerY * sample / maxY);
			if (showGrid && g <= p) {
				painter.setPen(palette_->instADPCMSampViewGridColor);
				painter.drawLine(x, 0, x, rect.height());
				g = (g / gridIntr_ + 1) * gridIntr_;
				painter.setPen(foreColor);
			}
			if (x) painter.drawLine(x - 1, prevY, x, y);
			prevY = y;
		}
	}
	else {
		QPoint prev, p;
		for (int i = 0; i < viewedSampLen_; ++i) {
			p.setX((maxX - 1) * i / (viewedSampLen_ - 1));
			int16_t sample = sample_[i + first];
			p.setY(centerY - (centerY * sample / maxY));
			if (showGrid && !(i % gridIntr_)) {
				painter.setPen(palette_->instADPCMSampViewGridColor);
				painter.drawLine(p.x(), 0, p.x(), rect.height());
				painter.setPen(foreColor);
			}
			if (p.x()) painter.drawLine(prev, p);
			prev = p;
		}
	}
}

void ADPCMSampleEditor::updateUsersView()
{
	std::multiset<int> users = bt_.lock()->getSampleADPCMUsers(ui->sampleNumSpinBox->value());
	ui->usersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void ADPCMSampleEditor::detectCursorSamplePosition(int cx, int cy)
{
	const QRect& rect = ui->sampleViewWidget->rect();

	// Detect x
	const int w = rect.width();
	if (viewedSampLen_ < w) {
		const double segW = rect.width() / (viewedSampLen_ - 1.);
		double th = segW / 2.;
		for (int i = 0; i < viewedSampLen_; ++i, th += segW) {
			if (cx < th) {
				cursorSamp_.setX(ui->horizontalScrollBar->value() + i);
				break;
			}
		}
	}
	else {
		cursorSamp_.setX(
					ui->horizontalScrollBar->value() + (viewedSampLen_ - 1) * utils::clamp(cx, 0, w - 1) / (w - 1));
	}

	// Detect y
	const double centerY = rect.height() >> 1;
	int y = std::numeric_limits<int16_t>::max() * (centerY - cy) / centerY;
	cursorSamp_.setY(utils::clamp(y, static_cast<int>(std::numeric_limits<int16_t>::min()),
								  static_cast<int>(std::numeric_limits<int16_t>::max())));

	// Update position view
	ui->detailLabel->setText(updateDetailView());
}

void ADPCMSampleEditor::sendEditedSample()
{
	std::vector<uint8_t> adpcm(sample_.size() >> 1);
	codec::ymb_encode(sample_.data(), adpcm.data(), static_cast<long>(sample_.size()));
	bt_.lock()->storeSampleADPCMRawSample(ui->sampleNumSpinBox->value(), std::move(adpcm));
	emit modified();
	emit sampleAssignRequested();
	emit sampleParameterChanged(ui->sampleNumSpinBox->value());
}

void ADPCMSampleEditor::onSampleNumberChanged()
{
	updateUsersView();
}

void ADPCMSampleEditor::onSampleMemoryUpdated(size_t start, size_t stop)
{
	addrStart_ = start;
	addrStop_ = stop;
	updateSampleMemoryBar();
	ui->memoryWidget->update();
}

void ADPCMSampleEditor::on_sampleNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) emit sampleNumberChanged(arg1);	// Direct connection

	onSampleNumberChanged();

	updateSampleMemoryBar();
	ui->memoryWidget->update();

	updateSampleView();
	ui->sampleViewWidget->update();
}

void ADPCMSampleEditor::on_repeatCheckBox_toggled(bool checked)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setSampleADPCMRepeatEnabled(ui->sampleNumSpinBox->value(), checked);
		emit sampleParameterChanged(ui->sampleNumSpinBox->value());
		emit modified();
	}
}

void ADPCMSampleEditor::on_rootRateSpinBox_valueChanged(int arg1)
{
	ui->rootRateSpinBox->setSuffix(
				QString(" (0x") + QString("%1 | ").arg(arg1, 3, 16, QChar('0')).toUpper()
				+ QString("%1Hz)").arg(QString::number(arg1 * 55500. * std::pow(2., -16.), 'f', 3)));

	if (!isIgnoreEvent_) {
		bt_.lock()->setSampleADPCMRootDeltaN(ui->sampleNumSpinBox->value(), arg1);
		emit sampleParameterChanged(ui->sampleNumSpinBox->value());
		emit modified();
	}
}

void ADPCMSampleEditor::on_action_Resize_triggered()
{
	SampleLengthDialog diag(sample_.size());
	if (diag.exec() == QDialog::Accepted) {
		sample_.resize(diag.getLength());
		sendEditedSample();

		updateSampleView();
		ui->sampleViewWidget->update();
	}
}

void ADPCMSampleEditor::on_actionRe_verse_triggered()
{
	std::reverse(sample_.begin(), sample_.end());
	sendEditedSample();

	updateSampleView();
	ui->sampleViewWidget->update();
}

void ADPCMSampleEditor::on_actionZoom_In_triggered()
{
	int len = (viewedSampLen_ + 1) >> 1;
	if (len > 1) {
		++zoom_;
		viewedSampLen_ = len;
		ui->horizontalScrollBar->setMaximum(sample_.size() - len);
		updateSampleView();
		ui->sampleViewWidget->update();

		ui->detailLabel->setText(updateDetailView());
	}
}

void ADPCMSampleEditor::on_actionZoom_Out_triggered()
{
	if (zoom_) {
		--zoom_;
		viewedSampLen_ = sample_.size() >> zoom_;
		ui->horizontalScrollBar->setMaximum(sample_.size() - viewedSampLen_);
		updateSampleView();
		ui->sampleViewWidget->update();

		ui->detailLabel->setText(updateDetailView());
	}
}

void ADPCMSampleEditor::on_horizontalScrollBar_valueChanged(int value)
{
	Q_UNUSED(value)
	updateSampleView();
	ui->sampleViewWidget->update();
}

void ADPCMSampleEditor::on_action_Import_triggered()
{
	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString file = QFileDialog::getOpenFileName(this, tr("Import sample"),
												(dir.isEmpty() ? "./" : dir),
												tr("WAV signed 16-bit PCM (*.wav)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
												, QFileDialog::DontUseNativeDialog
#endif
												);
	if (file.isNull()) return;

	importSampleFrom(file);
}

void ADPCMSampleEditor::on_action_Clear_triggered()
{
	bt_.lock()->clearSampleADPCMRawSample(ui->sampleNumSpinBox->value());

	updateSampleMemoryBar();
	ui->memoryWidget->update();

	emit modified();

	if (config_.lock()->getWriteOnlyUsedSamples()) {
		emit sampleAssignRequested();
	}
	else {
		emit sampleMemoryChanged();
	}

	emit sampleParameterChanged(ui->sampleNumSpinBox->value());
}

void ADPCMSampleEditor::on_action_Grid_View_triggered()
{
	updateSampleView();
	ui->sampleViewWidget->update();
}

void ADPCMSampleEditor::on_actionG_rid_Settings_triggered()
{
	GridSettingsDialog diag(gridIntr_);
	if (diag.exec() == QDialog::Accepted) {
		gridIntr_ = diag.getInterval();
		updateSampleView();
		ui->sampleViewWidget->update();
	}
}

void ADPCMSampleEditor::on_action_Draw_Sample_triggered(bool checked)
{
	if (checked) {
		ui->actionDirec_t_Draw->setChecked(false);
		drawMode_ = DrawMode::Normal;
	}
	else {
		if (drawMode_ == DrawMode::Normal) drawMode_ = DrawMode::Disabled;
		sendEditedSample();	// Convert sample
	}
	updateSampleView();
}

void ADPCMSampleEditor::on_actionDirec_t_Draw_triggered(bool checked)
{
	if (checked) {
		ui->action_Draw_Sample->setChecked(false);
		if (drawMode_ == DrawMode::Normal) sendEditedSample();	// Convert sample
		drawMode_ = DrawMode::Direct;
	}
	else {
		if (drawMode_ == DrawMode::Direct) drawMode_ = DrawMode::Disabled;
	}
	updateSampleView();
}
