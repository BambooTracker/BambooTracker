#include "adpcm_sample_editor.hpp"
#include "ui_adpcm_sample_editor.h"
#include <cmath>
#include <limits>
#include <vector>
#include <algorithm>
#include <QMimeData>
#include <QFile>
#include <QIODevice>
#include <QFileInfo>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>
#include <QPainter>
#include <QRect>
#include <QRectF>
#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QWheelEvent>
#include <QHoverEvent>
#include "chips/codec/ymb_codec.hpp"
#include "gui/event_guard.hpp"
#include "gui/instrument_editor/sample_length_dialog.hpp"
#include "gui/instrument_editor/grid_settings_dialog.hpp"

ADPCMSampleEditor::ADPCMSampleEditor(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ADPCMSampleEditor),
	isIgnoreEvent_(false),
	memPixmap_(std::make_unique<QPixmap>()),
	sampViewPixmap_(std::make_unique<QPixmap>()),
	zoom_(0),
	gridIntr_(1),
	cursorSamp_(0, 0),
	prevPressedSamp_(-1, -1),
	addrStart_(0),
	addrStop_(0),
	sample_(2)
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
	auto gridbutton = new QToolButton();
	gridbutton->setPopupMode(QToolButton::MenuButtonPopup);
	gridbutton->setMenu(gridMenu);
	gridbutton->setDefaultAction(ui->action_Grid_View);
	tb->addWidget(gridbutton);
	tb->addSeparator();
	tb->addActions({ ui->action_Draw_Sample, ui->actionRe_verse });
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
			&& QFileInfo(mime->urls().first().toLocalFile()).suffix().toLower() == "wav") {
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
			painter.drawPixmap(ui->memoryWidget->rect(), *memPixmap_.get());
			break;
		}
		case QEvent::Resize:
			memPixmap_ = std::make_unique<QPixmap>(ui->memoryWidget->size());
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
			painter.drawPixmap(ui->sampleViewWidget->rect(), *sampViewPixmap_.get());
			break;
		}
		case QEvent::Resize:
			sampViewPixmap_ = std::make_unique<QPixmap>(ui->sampleViewWidget->size());
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
				int px = prevPressedSamp_.x();
				int py = prevPressedSamp_.y();
				int cx = cursorSamp_.x();
				int cy = cursorSamp_.y();
				if (px < cx) {
					for (int x = px + 1; x <= cx; ++x)
						sample_.at(x) = (cy - py) * (x - px) + py;
				}
				else if (px == cx) {
					sample_.at(cx) = cy;
				}
				else {
					for (int x = cx; x < px; ++x)
						sample_.at(x) = (py - cy) * (x - cx) + cy;
				}
				std::vector<uint8_t> adpcm(sample_.size() >> 1);
				codec::ymb_encode(sample_.data(), adpcm.data(), static_cast<long>(sample_.size()));
				bt_.lock()->storeSampleADPCMRawSample(ui->sampleNumSpinBox->value(), std::move(adpcm));
				prevPressedSamp_ = cursorSamp_;
				emit modified();
				emit sampleAssignRequested();
				emit sampleParameterChanged(ui->sampleNumSpinBox->value());
			}
			break;
		}
		case QEvent::MouseButtonPress:
		{
			if (!ui->action_Draw_Sample->isChecked()) break;
			sample_.at(cursorSamp_.x()) = cursorSamp_.y();
			std::vector<uint8_t> adpcm(sample_.size() >> 1);
			codec::ymb_encode(sample_.data(), adpcm.data(), static_cast<long>(sample_.size()));
			bt_.lock()->storeSampleADPCMRawSample(ui->sampleNumSpinBox->value(), std::move(adpcm));
			prevPressedSamp_ = cursorSamp_;
			emit modified();
			emit sampleAssignRequested();
			emit sampleParameterChanged(ui->sampleNumSpinBox->value());
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

	sample_.resize(sample.size() * 2);
	codec::ymb_decode(sample.data(), sample_.data(), static_cast<long>(sample_.size()));
	updateSampleView();
	ui->sampleViewWidget->update();

	ui->detailLabel->setText(updateDetailView());
}

void ADPCMSampleEditor::importSampleFrom(const QString file)
{
	std::unique_ptr<WavContainer> wav;
	try {
		QFile fp(file);
		if (!fp.open(QIODevice::ReadOnly)) throw FileInputError(FileIO::FileType::WAV);
		QByteArray array = fp.readAll();
		fp.close();

		wav = std::make_unique<WavContainer>(BinaryContainer(std::vector<char>(array.begin(), array.end())));
	}
	catch (FileIOError& e) {
		Q_UNUSED(e)
		QMessageBox::critical(this, tr("Error"), tr("Failed to import the wav."));
		return;
	}
	catch (std::exception& e) {
		QMessageBox::critical(this, tr("Error"), tr("Failed to import the wav.\n%1").arg(QString(e.what())));
		return;
	}

	if (wav->getSampleRate() < 2000 || 16000 < wav->getSampleRate()) {
		QMessageBox::critical(this, tr("Error"),
							  tr("Supported sample rate is 2kHz-16kHz, but the rate of selected sample is %1.")
							  .arg(wav->getSampleRate()));
		return;
	}

	if (wav->getChannelCount() != 1) {
		QMessageBox::critical(this, tr("Error"), tr("The selected sample is not mono channel."));
		return;
	}

	BinaryContainer bc = wav->getSample();
	size_t rawSize = bc.size() / 2;
	std::vector<int16_t> raw(rawSize);
	for (size_t i = 0; i < rawSize; ++i) {
		raw[i] = bc.readInt16(i * 2);
	}
	std::vector<uint8_t> adpcm((raw.size() + 1) / 2);
	codec::ymb_encode(raw.data(), adpcm.data(), static_cast<long>(raw.size()));

	const int ROOT_KEY = 60;	//C5

	bt_.lock()->storeSampleADPCMRawSample(ui->sampleNumSpinBox->value(), adpcm);
	ui->rootKeyComboBox->setCurrentIndex(ROOT_KEY % 12);
	ui->rootKeySpinBox->setValue(ROOT_KEY / 12);
	ui->rootRateSpinBox->setValue(calcADPCMDeltaN(wav->getSampleRate()));

	updateSampleView();
	ui->sampleViewWidget->update();

	emit modified();
	emit sampleAssignRequested();
	emit sampleParameterChanged(ui->sampleNumSpinBox->value());
}

void ADPCMSampleEditor::updateSampleMemoryBar()
{
	QRect bar = memPixmap_->rect();
	if (!bar.isValid()) return;

	QPainter painter(memPixmap_.get());
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
	QRect rect = sampViewPixmap_->rect();
	if (!rect.isValid()) return;

	// Slider
	size_t tmplen = sample_.size() - 1;
	for (int z = zoom_; z >= 0; --z) {
		size_t len = tmplen >> z;
		if (len) {
			zoom_ = z;
			ui->horizontalScrollBar->setMaximum(tmplen - len);
			break;
		}
	}

	QPainter painter(sampViewPixmap_.get());
	painter.fillRect(rect, palette_->instADPCMSampViewBackColor);

	painter.setPen(palette_->instADPCMSampViewCenterColor);
	const int maxX = rect.width();
	const int centerY = rect.height() >> 1;
	painter.drawLine(0, centerY, maxX, centerY);

	painter.setPen(palette_->instADPCMSampViewForeColor);
	const int16_t maxY = std::numeric_limits<int16_t>::max();
	const size_t seglen = sample_.size() >> zoom_;
	const size_t first = ui->horizontalScrollBar->value();
	const bool showGrid = ui->action_Grid_View->isChecked();
	if (maxX < static_cast<int>(seglen)) {
		int prevY = centerY;
		size_t g = first;
		for (int x = 0; x <= maxX; ++x) {
			size_t i = seglen * x / maxX;
			int16_t sample = sample_[i + first];
			int y = centerY - (centerY * sample / maxY);
			if (showGrid && g <= i) {
				painter.setPen(palette_->instADPCMSampViewGridColor);
				painter.drawLine(x, 0, x, rect.height());
				g = (g / gridIntr_ + 1) * gridIntr_;
				painter.setPen(palette_->instADPCMSampViewForeColor);
			}
			if (x) painter.drawLine(x - 1, prevY, x, y);
			prevY = y;
		}
	}
	else {
		QPoint prev, p;
		for (size_t i = 0; i < seglen; ++i) {
			p.setX(maxX * i / (seglen - 1));
			int16_t sample = sample_[i + first];
			p.setY(centerY - (centerY * sample / maxY));
			if (showGrid && !(i % gridIntr_)) {
				painter.setPen(palette_->instADPCMSampViewGridColor);
				painter.drawLine(p.x(), 0, p.x(), rect.height());
				painter.setPen(palette_->instADPCMSampViewForeColor);
			}
			if (p.x()) painter.drawLine(prev, p);
			prev = p;
		}
	}
}

void ADPCMSampleEditor::updateUsersView()
{
	std::vector<int> users = bt_.lock()->getSampleADPCMUsers(ui->sampleNumSpinBox->value());
	QStringList l;
	std::transform(users.begin(), users.end(), std::back_inserter(l), [](int n) {
		return QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
	});
	ui->usersLineEdit->setText(l.join(","));
}

void ADPCMSampleEditor::detectCursorSamplePosition(int cx, int cy)
{
	const QRect& rect = ui->sampleViewWidget->rect();

	// Detect x
	const size_t len = sample_.size() >> zoom_;
	const size_t w = rect.width();
	if (len < w) {
		const int segW = rect.width() / (len - 1);
		int th = segW >> 1;
		for (size_t i = 0; i < len; ++i, th += segW) {
			if (cx < th) {
				cursorSamp_.setX(ui->horizontalScrollBar->value() + i);
				break;
			}
		}
	}
	else {
		cursorSamp_.setX(len * cx / w);
	}

	// Detect y
	const double centerY = rect.height() >> 1;
	int y = std::numeric_limits<int16_t>::max() * (centerY - cy) / centerY;
	cursorSamp_.setY(clamp(y, static_cast<int>(std::numeric_limits<int16_t>::min()),
						   static_cast<int>(std::numeric_limits<int16_t>::max())));

	// Update position view
	ui->detailLabel->setText(updateDetailView());
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

		std::vector<uint8_t> adpcm(sample_.size() / 2);
		codec::ymb_encode(sample_.data(), adpcm.data(), static_cast<long>(sample_.size()));
		bt_.lock()->storeSampleADPCMRawSample(ui->sampleNumSpinBox->value(), std::move(adpcm));

		updateSampleView();
		ui->sampleViewWidget->update();

		emit modified();
		emit sampleAssignRequested();
		emit sampleParameterChanged(ui->sampleNumSpinBox->value());
	}
}

void ADPCMSampleEditor::on_actionRe_verse_triggered()
{
	std::vector<uint8_t> adpcm(sample_.size() / 2);
	std::reverse(sample_.begin(), sample_.end());
	codec::ymb_encode(sample_.data(), adpcm.data(), static_cast<long>(sample_.size()));
	bt_.lock()->storeSampleADPCMRawSample(ui->sampleNumSpinBox->value(), std::move(adpcm));

	updateSampleView();
	ui->sampleViewWidget->update();

	emit modified();
	emit sampleAssignRequested();
	emit sampleParameterChanged(ui->sampleNumSpinBox->value());
}

void ADPCMSampleEditor::on_actionZoom_In_triggered()
{
	int z = zoom_ + 1;
	size_t len = sample_.size() >> z;
	if (len > 1) {
		zoom_ = z;
		updateSampleView();
		ui->sampleViewWidget->update();

		ui->detailLabel->setText(updateDetailView());
	}
}

void ADPCMSampleEditor::on_actionZoom_Out_triggered()
{
	if (zoom_) {
		--zoom_;
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
												(dir.isEmpty() ? "./" : dir), tr("WAV signed 16-bit PCM (*.wav)"));
	if (file.isNull()) return;

	importSampleFrom(file);
}

void ADPCMSampleEditor::on_action_Clear_triggered()
{
	bt_.lock()->clearSampleADPCMRawSample(ui->sampleNumSpinBox->value());

	updateSampleMemoryBar();
	ui->memoryWidget->update();

	updateSampleView();
	ui->sampleViewWidget->update();

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
