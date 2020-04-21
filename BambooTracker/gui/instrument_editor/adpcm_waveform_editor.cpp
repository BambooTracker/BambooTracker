#include "adpcm_waveform_editor.hpp"
#include "ui_adpcm_waveform_editor.h"
#include <cmath>
#include <limits>
#include <vector>
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
#include "chips/codec/ymb_codec.hpp"
#include "gui/event_guard.hpp"

ADPCMWaveformEditor::ADPCMWaveformEditor(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ADPCMWaveformEditor),
	isIgnoreEvent_(false),
	wavMemPixmap_(std::make_unique<QPixmap>()),
	wavViewPixmap_(std::make_unique<QPixmap>())
{
	ui->setupUi(this);

	auto rkfunc = [&](int dummy) {
		Q_UNUSED(dummy)
		if (!isIgnoreEvent_) {
			int rk = ui->rootKeySpinBox->value() * 12 + ui->rootKeyComboBox->currentIndex();
			bt_.lock()->setWaveformADPCMRootKeyNumber(ui->waveNumSpinBox->value(), rk);
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	};
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->rootKeyComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, rkfunc);
	QObject::connect(ui->rootKeySpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, rkfunc);

	ui->waveMemoryWidget->installEventFilter(this);
	ui->waveViewWidget->installEventFilter(this);
}

ADPCMWaveformEditor::~ADPCMWaveformEditor()
{
	delete ui;
}

void ADPCMWaveformEditor::setNumber(int n)
{
	instNum_ = n;
}

void ADPCMWaveformEditor::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;

	Ui::EventGuard eg(isIgnoreEvent_);
	setInstrumentWaveformParameters();
}

void ADPCMWaveformEditor::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
}

void ADPCMWaveformEditor::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
}

void ADPCMWaveformEditor::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mime = event->mimeData();
	if (mime->hasUrls() && mime->urls().length() == 1
			&& QFileInfo(mime->urls().first().toLocalFile()).suffix().toLower() == "wav") {
		event->acceptProposedAction();
	}
}

bool ADPCMWaveformEditor::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == ui->waveMemoryWidget) {
		switch (ev->type()) {
		case QEvent::Paint:
		{
			QPainter painter(ui->waveMemoryWidget);
			painter.drawPixmap(ui->waveMemoryWidget->rect(), *wavMemPixmap_.get());
			break;
		}
		case QEvent::Resize:
			wavMemPixmap_ = std::make_unique<QPixmap>(ui->waveMemoryWidget->size());
			updateSampleMemoryBar();
			break;
		default:
			break;
		}
	}

	if (obj == ui->waveViewWidget) {
		switch (ev->type()) {
		case QEvent::Paint:
		{
			QPainter painter(ui->waveViewWidget);
			painter.drawPixmap(ui->waveViewWidget->rect(), *wavViewPixmap_.get());
			break;
		}
		case QEvent::Resize:
			wavViewPixmap_ = std::make_unique<QPixmap>(ui->waveViewWidget->size());
			updateSampleView();
			break;
		default:
			break;
		}
	}

	return false;
}

void ADPCMWaveformEditor::dropEvent(QDropEvent* event)
{
	importSampleFrom(reinterpret_cast<QDropEvent*>(event)->mimeData()->urls().first().toLocalFile());
}

void ADPCMWaveformEditor::setInstrumentWaveformParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->waveNumSpinBox->setValue(instADPCM->getWaveformNumber());

	ui->waveRepeatCheckBox->setChecked(instADPCM->isWaveformRepeatable());
	int rk = instADPCM->getWaveformRootKeyNumber();
	ui->rootKeyComboBox->setCurrentIndex(rk % 12);
	ui->rootKeySpinBox->setValue(rk / 12);
	ui->rootRateSpinBox->setValue(instADPCM->getWaveformRootDeltaN());
}

void ADPCMWaveformEditor::importSampleFrom(const QString file)
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

	bt_.lock()->storeWaveformADPCMSample(ui->waveNumSpinBox->value(), adpcm);
	ui->rootKeyComboBox->setCurrentIndex(ROOT_KEY % 12);
	ui->rootKeySpinBox->setValue(ROOT_KEY / 12);
	ui->rootRateSpinBox->setValue(calcADPCMDeltaN(wav->getSampleRate()));

	updateSampleView();
	ui->waveViewWidget->update();

	emit modified();
	emit waveformAssignRequested();
}

void ADPCMWaveformEditor::updateSampleMemoryBar()
{
	QRect bar = wavMemPixmap_->rect();
	if (!bar.isValid()) return;

	QPainter painter(wavMemPixmap_.get());
	painter.fillRect(bar, palette_->instADPCMMemBackColor);

	double maxSize = bt_.lock()->getADPCMStoredSize() >> 5;
	double limit = bt_.lock()->getADPCMLimit() >> 5;	// By 32 bytes
	QRectF allSamp(0, 0, std::max(1., bar.width() * (maxSize / limit)), rect().height());
	painter.fillRect(allSamp, palette_->instADPCMMemAllColor);

	std::unique_ptr<InstrumentADPCM> inst(dynamic_cast<InstrumentADPCM*>(bt_.lock()->getInstrument(instNum_).release()));
	size_t start = inst->getWaveformStartAddress();
	size_t stop = inst->getWaveformStopAddress();
	if (start || start != stop) {
		QRectF curSamp(bar.width() * (start / limit),
					   0, std::max(1., bar.width() * ((stop - start) / limit)), rect().height());
		painter.fillRect(curSamp, palette_->instADPCMMemCurColor);
	}
}

void ADPCMWaveformEditor::updateSampleView()
{
	QRect rect = wavViewPixmap_->rect();
	if (!rect.isValid()) return;

	QPainter painter(wavViewPixmap_.get());
	painter.fillRect(rect, palette_->instADPCMWaveViewBackColor);

	std::unique_ptr<InstrumentADPCM> inst(dynamic_cast<InstrumentADPCM*>(bt_.lock()->getInstrument(instNum_).release()));
	std::vector<uint8_t> adpcm = inst->getWaveformSamples();
	std::vector<int16_t> wave(adpcm.size() * 2);
	codec::ymb_decode(&adpcm[0], &wave[0], static_cast<long>(wave.size()));

	painter.setPen(palette_->instADPCMWaveViewCenterColor);
	const int maxX = rect.width();
	const int centerY = rect.height() / 2;
	painter.drawLine(0, centerY, maxX, centerY);

	painter.setPen(palette_->instADPCMWaveViewForeColor);
	const int16_t maxY = std::numeric_limits<int16_t>::max();
	const double size = wave.size();
	int prevY = centerY;
	for (int x = 0; x < maxX; ++x) {
		size_t i = static_cast<size_t>(size * x / maxX);
		int16_t sample = wave[i];
		int y = centerY - (centerY * sample / maxY);
		if (x) painter.drawLine(x - 1, prevY, x, y);
		prevY = y;
	}
}

void ADPCMWaveformEditor::onWaveformNumberChanged()
{
	// Change users view
	std::vector<int> users = bt_.lock()->getWaveformADPCMUsers(ui->waveNumSpinBox->value());
	QStringList l;
	std::transform(users.begin(), users.end(), std::back_inserter(l), [](int n) {
		return QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
	});
	ui->waveUsersLineEdit->setText(l.join(","));

	updateSampleMemoryBar();
	ui->waveMemoryWidget->update();

	updateSampleView();
	ui->waveViewWidget->update();
}

void ADPCMWaveformEditor::onWaveformParameterChanged(int wfNum)
{
	if (ui->waveNumSpinBox->value() == wfNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentWaveformParameters();
	}
}

void ADPCMWaveformEditor::onWaveformSampleMemoryUpdated()
{
	updateSampleMemoryBar();
	ui->waveMemoryWidget->update();
}

void ADPCMWaveformEditor::on_waveNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMWaveform(instNum_, arg1);
		setInstrumentWaveformParameters();
		emit waveformNumberChanged();
		emit modified();

		if (config_.lock()->getWriteOnlyUsedSamples()) {
			emit onWaveformSampleMemoryUpdated();
		}
	}

	onWaveformNumberChanged();
}

void ADPCMWaveformEditor::on_rootRateSpinBox_valueChanged(int arg1)
{
	ui->rootRateSpinBox->setSuffix(
				QString(" (0x") + QString("%1 | ").arg(arg1, 3, 16, QChar('0')).toUpper()
				+ QString("%1Hz)").arg(QString::number(arg1 * 55500. * std::pow(2., -16.), 'f', 3)));

	if (!isIgnoreEvent_) {
		bt_.lock()->setWaveformADPCMRootDeltaN(ui->waveNumSpinBox->value(), arg1);
		emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
		emit modified();
	}
}

void ADPCMWaveformEditor::on_waveRepeatCheckBox_toggled(bool checked)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setWaveformADPCMRepeatEnabled(ui->waveNumSpinBox->value(), checked);
		emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
		emit modified();
	}
}

void ADPCMWaveformEditor::on_waveImportPushButton_clicked()
{
	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString file = QFileDialog::getOpenFileName(this, tr("Import sample"),
												(dir.isEmpty() ? "./" : dir), tr("WAV signed 16-bit PCM (*.wav)"));
	if (file.isNull()) return;

	importSampleFrom(file);
}

void ADPCMWaveformEditor::on_waveClearPushButton_clicked()
{
	bt_.lock()->clearWaveformADPCMSample(ui->waveNumSpinBox->value());

	updateSampleMemoryBar();
	ui->waveMemoryWidget->update();

	updateSampleView();
	ui->waveViewWidget->update();

	emit modified();

	if (config_.lock()->getWriteOnlyUsedSamples()) {
		emit onWaveformSampleMemoryUpdated();
	}
}
