#include "mainwindow.hpp"
#include <fstream>
#include <QString>
#include <QLineEdit>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QRegularExpression>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QProgressDialog>
#include <QRect>
#include <QDesktopWidget>
#include <QAudioDeviceInfo>
#include "ui_mainwindow.h"
#include "jam_manager.hpp"
#include "song.hpp"
#include "track.hpp"
#include "instrument.hpp"
#include "version.hpp"
#include "gui/command/commands_qt.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "gui/module_properties_dialog.hpp"
#include "gui/groove_settings_dialog.hpp"
#include "gui/configuration_dialog.hpp"
#include "gui/comment_edit_dialog.hpp"
#include "gui/wave_export_settings_dialog.hpp"
#include "gui/vgm_export_settings_dialog.hpp"
#include "gui/json.hpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	config_(std::make_shared<Configuration>()),
	palette_(std::make_shared<ColorPalette>()),
	bt_(std::make_shared<BambooTracker>(config_)),
	comStack_(std::make_shared<QUndoStack>(this)),
	instForms_(std::make_shared<InstrumentFormManager>()),
	isModifiedForNotCommand_(false),
	isEditedPattern_(true),
	isEditedOrder_(false),
	isSelectedPO_(false)
{
	ui->setupUi(this);

	Json::loadConfiguration(config_);
	if (config_->getMainWindowX() == -1) {	// When unset
		QRect rec = geometry();
		rec.moveCenter(QApplication::desktop()->availableGeometry().center());
		setGeometry(rec);
		config_->setMainWindowX(x());
		config_->setMainWindowY(y());
	}
	else {
		move(config_->getMainWindowX(), config_->getMainWindowY());
	}
	resize(config_->getMainWindowWidth(), config_->getMainWindowHeight());
	if (config_->getMainWindowMaximized()) showMaximized();
	ui->actionFollow_Mode->setChecked(config_->getFollowMode());
	bt_->setFollowPlay(config_->getFollowMode());

	/* Command stack */
	QObject::connect(comStack_.get(), &QUndoStack::indexChanged,
					 this, [&](int idx) {
		setWindowModified(idx || isModifiedForNotCommand_);
		ui->actionUndo->setEnabled(comStack_->canUndo());
		ui->actionRedo->setEnabled(comStack_->canRedo());
	});

	/* Audio stream */
	if (config_->getSoundDevice() == u8"") {
		QString sndDev = QAudioDeviceInfo::defaultOutputDevice().deviceName();
		config_->setSoundDevice(sndDev.toUtf8().toStdString());
	}
	stream_ = std::make_shared<AudioStream>(bt_->getStreamRate(),
											bt_->getStreamDuration(),
											bt_->getModuleTickFrequency(),
											QString::fromUtf8(config_->getSoundDevice().c_str(),
															  config_->getSoundDevice().length()));
	QObject::connect(stream_.get(), &AudioStream::streamInterrupted,
					 this, [&]() {
		if (!bt_->streamCountUp()) {
			ui->orderList->update();
			ui->patternEditor->updatePosition();
		}
	}, Qt::DirectConnection);
	QObject::connect(stream_.get(), &AudioStream::bufferPrepared,
					 this, [&](int16_t *container, size_t nSamples) {
		bt_->getStreamSamples(container, nSamples);
	}, Qt::DirectConnection);

	/* Module settings */
	QObject::connect(ui->modTitleLineEdit, &QLineEdit::textEdited,
					 this, [&](QString str) {
		bt_->setModuleTitle(str.toUtf8().toStdString());
		setModifiedTrue();
		setWindowTitle();
	});
	QObject::connect(ui->authorLineEdit, &QLineEdit::textEdited,
					 this, [&](QString str) {
		bt_->setModuleAuthor(str.toUtf8().toStdString());
		setModifiedTrue();
	});
	QObject::connect(ui->copyrightLineEdit, &QLineEdit::textEdited,
					 this, [&](QString str) {
		bt_->setModuleCopyright(str.toUtf8().toStdString());
		setModifiedTrue();
	});
	QObject::connect(ui->tickFreqSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int freq) {
		if (freq != bt_->getModuleTickFrequency()) {
			bt_->setModuleTickFrequency(freq);
			stream_->setInturuption(freq);
			setModifiedTrue();
		}
	});
	QObject::connect(ui->modSetDialogOpenToolButton, &QToolButton::clicked,
					 this, &MainWindow::on_actionModule_Properties_triggered);

	/* Song number */
	QObject::connect(ui->songNumSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int num) {
		bt_->setCurrentSongNumber(num);
		loadSong();
	});

	/* Song settings */
	QObject::connect(ui->tempoSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int tempo) {
		int curSong = bt_->getCurrentSongNumber();
		if (tempo != bt_->getSongTempo(curSong)) {
			bt_->setSongTempo(curSong, tempo);
			setModifiedTrue();
		}
	});
	QObject::connect(ui->speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int speed) {
		int curSong = bt_->getCurrentSongNumber();
		if (speed != bt_->getSongSpeed(curSong)) {
			bt_->setSongSpeed(curSong, speed);
			setModifiedTrue();
		}
	});
	QObject::connect(ui->patternSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int size) {
		bt_->setDefaultPatternSize(bt_->getCurrentSongNumber(), size);
		ui->patternEditor->onDefaultPatternSizeChanged();
		setModifiedTrue();
	});
	QObject::connect(ui->grooveSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int n) {
		bt_->setSongGroove(bt_->getCurrentSongNumber(), n);
		setModifiedTrue();
	});

	/* Pattern step highlight */
	ui->stepHighrightSpinBox->setValue(8);
	QObject::connect(ui->stepHighrightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int count) {
		bt_->setModuleStepHighlightDistance(count);
		ui->patternEditor->setPatternHighlightCount(count);
		ui->patternEditor->update();
	});

	/* Octave */
	ui->octaveSpinBox->setValue(bt_->getCurrentOctave());
	QObject::connect(ui->octaveSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int octave) { bt_->setCurrentOctave(octave); });

	/* Instrument list */
	ui->instrumentListWidget->setStyleSheet(
				QString(
					"QListWidget {"
					"	color: rgba(%1, %2, %3, %4);"
					"	background: rgba(%5, %6, %7, %8);"
					"}"
				).arg(palette_->ilistTextColor.red()).arg(palette_->ilistTextColor.green())
				.arg(palette_->ilistTextColor.blue()).arg(palette_->ilistTextColor.alpha())
				.arg(palette_->ilistBackColor.red()).arg(palette_->ilistBackColor.green())
				.arg(palette_->ilistBackColor.blue()).arg(palette_->ilistBackColor.alpha())
				+ QString(
					"QListWidget::item:hover {"
					"	color: rgba(%1, %2, %3, %4);"
					"	background: rgba(%5, %6, %7, %8);"
					"}"
					).arg(palette_->ilistHovTextColor.red()).arg(palette_->ilistHovTextColor.green())
					.arg(palette_->ilistHovTextColor.blue()).arg(palette_->ilistHovTextColor.alpha())
					.arg(palette_->ilistHovBackColor.red()).arg(palette_->ilistHovBackColor.green())
					.arg(palette_->ilistHovBackColor.blue()).arg(palette_->ilistHovBackColor.alpha())
				+ QString(
					"QListWidget::item:selected {"
					"	color: rgba(%1, %2, %3, %4);"
					"	background: rgba(%5, %6, %7, %8);"
					"}"
					).arg(palette_->ilistSelTextColor.red()).arg(palette_->ilistSelTextColor.green())
					.arg(palette_->ilistSelTextColor.blue()).arg(palette_->ilistSelTextColor.alpha())
					.arg(palette_->ilistSelBackColor.red()).arg(palette_->ilistSelBackColor.green())
					.arg(palette_->ilistSelBackColor.blue()).arg(palette_->ilistSelBackColor.alpha())
				+ QString(
					"QListWidget::item:selected:hover {"
					"	color: rgba(%1, %2, %3, %4);"
					"	background: rgba(%5, %6, %7, %8);"
					"}"
					).arg(palette_->ilistHovSelTextColor.red()).arg(palette_->ilistHovSelTextColor.green())
					.arg(palette_->ilistHovSelTextColor.blue()).arg(palette_->ilistHovSelTextColor.alpha())
					.arg(palette_->ilistHovSelBackColor.red()).arg(palette_->ilistHovSelBackColor.green())
					.arg(palette_->ilistHovSelBackColor.blue()).arg(palette_->ilistHovSelBackColor.alpha())
				);
	ui->instrumentListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->instrumentListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->instrumentListWidget->setFocusPolicy(Qt::NoFocus);
	// Set core data to editor when add insrument
	QObject::connect(ui->instrumentListWidget->model(), &QAbstractItemModel::rowsInserted,
					 this, &MainWindow::onInstrumentListWidgetItemAdded);

	/* Pattern editor */
	ui->patternEditor->setCore(bt_);
	ui->patternEditor->setCommandStack(comStack_);
	ui->patternEditor->setConfiguration(config_);
	ui->patternEditor->setColorPallete(palette_);
	ui->patternEditor->installEventFilter(this);
	QObject::connect(ui->patternEditor, &PatternEditor::currentTrackChanged,
					 ui->orderList, &OrderListEditor::setCurrentTrack);
	QObject::connect(ui->patternEditor, &PatternEditor::currentOrderChanged,
					 ui->orderList, &OrderListEditor::setCurrentOrder);
	QObject::connect(ui->patternEditor, &PatternEditor::focusIn,
					 this, &MainWindow::updateMenuByPattern);
	QObject::connect(ui->patternEditor, &PatternEditor::focusOut,
					 this, &MainWindow::onPatternAndOrderFocusLost);
	QObject::connect(ui->patternEditor, &PatternEditor::selected,
					 this, &MainWindow::updateMenuByPatternAndOrderSelection);

	/* Order List */
	ui->orderList->setCore(bt_);
	ui->orderList->setCommandStack(comStack_);
	ui->orderList->setConfiguration(config_);
	ui->orderList->setColorPallete(palette_);
	ui->orderList->installEventFilter(this);
	QObject::connect(ui->orderList, &OrderListEditor::currentTrackChanged,
					 ui->patternEditor, &PatternEditor::setCurrentTrack);
	QObject::connect(ui->orderList, &OrderListEditor::currentOrderChanged,
					 ui->patternEditor, &PatternEditor::setCurrentOrder);
	QObject::connect(ui->orderList, &OrderListEditor::orderEdited,
					 ui->patternEditor, &PatternEditor::onOrderListEdited);
	QObject::connect(ui->orderList, &OrderListEditor::focusIn,
					 this, &MainWindow::updateMenuByOrder);
	QObject::connect(ui->orderList, &OrderListEditor::focusOut,
					 this, &MainWindow::onPatternAndOrderFocusLost);
	QObject::connect(ui->orderList, &OrderListEditor::selected,
					 this, &MainWindow::updateMenuByPatternAndOrderSelection);

	/* Clipboard */
	QObject::connect(QApplication::clipboard(), &QClipboard::dataChanged,
					 this, [&]() {
		if (isEditedOrder_) updateMenuByOrder();
		else if (isEditedPattern_) updateMenuByPattern();
	});

	loadModule();
}

MainWindow::~MainWindow()
{
	delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	if (auto fmForm = qobject_cast<InstrumentEditorFMForm*>(watched)) {
		// Change current instrument by activating FM editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(fmForm->getInstrumentNumber());
			ui->instrumentListWidget->setCurrentRow(row);
			return false;
		}
		else if (event->type() == QEvent::Resize) {
			config_->setInstrumentFMWindowWidth(fmForm->width());
			config_->setInstrumentFMWindowHeight(fmForm->height());
			return false;
		}
	}

	if (auto ssgForm = qobject_cast<InstrumentEditorSSGForm*>(watched)) {
		// Change current instrument by activating SSG editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(ssgForm->getInstrumentNumber());
			ui->instrumentListWidget->setCurrentRow(row);
			return false;
		}
		else if (event->type() == QEvent::Resize) {
			config_->setInstrumentSSGWindowWidth(ssgForm->width());
			config_->setInstrumentSSGWindowHeight(ssgForm->height());
			return false;
		}
	}

	if (auto orderList = qobject_cast<OrderListEditor*>(watched)) {
		// Catch space key pressing in order list
		if (event->type() == QEvent::KeyPress) {
			auto keyEvent = dynamic_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Space) {
				keyPressEvent(keyEvent);
				return true;
			}
		}
	}

	return false;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();

	/* General keys */
	switch (key) {
	case Qt::Key_Asterisk:	changeOctave(true);			break;
	case Qt::Key_Slash:		changeOctave(false);		break;

	default:
		if (!event->isAutoRepeat()) {
			// Musical keyboard
			switch (key) {
			case Qt::Key_Z:			bt_->jamKeyOn(JamKey::LOW_C);		break;
			case Qt::Key_S:			bt_->jamKeyOn(JamKey::LOW_CS);		break;
			case Qt::Key_X:			bt_->jamKeyOn(JamKey::LOW_D);		break;
			case Qt::Key_D:			bt_->jamKeyOn(JamKey::LOW_DS);		break;
			case Qt::Key_C:			bt_->jamKeyOn(JamKey::LOW_E);		break;
			case Qt::Key_V:			bt_->jamKeyOn(JamKey::LOW_F);		break;
			case Qt::Key_G:			bt_->jamKeyOn(JamKey::LOW_FS);		break;
			case Qt::Key_B:			bt_->jamKeyOn(JamKey::LOW_G);		break;
			case Qt::Key_H:			bt_->jamKeyOn(JamKey::LOW_GS);		break;
			case Qt::Key_N:			bt_->jamKeyOn(JamKey::LOW_A);		break;
			case Qt::Key_J:			bt_->jamKeyOn(JamKey::LOW_AS);		break;
			case Qt::Key_M:			bt_->jamKeyOn(JamKey::LOW_B);		break;
			case Qt::Key_Comma:		bt_->jamKeyOn(JamKey::LOW_C_H);		break;
			case Qt::Key_L:			bt_->jamKeyOn(JamKey::LOW_CS_H);	break;
			case Qt::Key_Period:	bt_->jamKeyOn(JamKey::LOW_D_H);		break;
			case Qt::Key_Q:			bt_->jamKeyOn(JamKey::HIGH_C);		break;
			case Qt::Key_2:			bt_->jamKeyOn(JamKey::HIGH_CS);		break;
			case Qt::Key_W:			bt_->jamKeyOn(JamKey::HIGH_D);		break;
			case Qt::Key_3:			bt_->jamKeyOn(JamKey::HIGH_DS);		break;
			case Qt::Key_E:			bt_->jamKeyOn(JamKey::HIGH_E);		break;
			case Qt::Key_R:			bt_->jamKeyOn(JamKey::HIGH_F);		break;
			case Qt::Key_5:			bt_->jamKeyOn(JamKey::HIGH_FS);		break;
			case Qt::Key_T:			bt_->jamKeyOn(JamKey::HIGH_G);		break;
			case Qt::Key_6:			bt_->jamKeyOn(JamKey::HIGH_GS);		break;
			case Qt::Key_Y:			bt_->jamKeyOn(JamKey::HIGH_A);		break;
			case Qt::Key_7:			bt_->jamKeyOn(JamKey::HIGH_AS);		break;
			case Qt::Key_U:			bt_->jamKeyOn(JamKey::HIGH_B);		break;
			case Qt::Key_I:			bt_->jamKeyOn(JamKey::HIGH_C_H);	break;
			case Qt::Key_9:			bt_->jamKeyOn(JamKey::HIGH_CS_H);	break;
			case Qt::Key_O:			bt_->jamKeyOn(JamKey::HIGH_D_H);	break;
			default:	break;
			}
		}
		break;
	}
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	int key = event->key();

	if (!event->isAutoRepeat()) {
		// Musical keyboard
		switch (key) {
		case Qt::Key_Z:			bt_->jamKeyOff(JamKey::LOW_C);		break;
		case Qt::Key_S:			bt_->jamKeyOff(JamKey::LOW_CS);		break;
		case Qt::Key_X:			bt_->jamKeyOff(JamKey::LOW_D);		break;
		case Qt::Key_D:			bt_->jamKeyOff(JamKey::LOW_DS);		break;
		case Qt::Key_C:			bt_->jamKeyOff(JamKey::LOW_E);		break;
		case Qt::Key_V:			bt_->jamKeyOff(JamKey::LOW_F);		break;
		case Qt::Key_G:			bt_->jamKeyOff(JamKey::LOW_FS);		break;
		case Qt::Key_B:			bt_->jamKeyOff(JamKey::LOW_G);		break;
		case Qt::Key_H:			bt_->jamKeyOff(JamKey::LOW_GS);		break;
		case Qt::Key_N:			bt_->jamKeyOff(JamKey::LOW_A);		break;
		case Qt::Key_J:			bt_->jamKeyOff(JamKey::LOW_AS);		break;
		case Qt::Key_M:			bt_->jamKeyOff(JamKey::LOW_B);		break;
		case Qt::Key_Comma:		bt_->jamKeyOff(JamKey::LOW_C_H);	break;
		case Qt::Key_L:			bt_->jamKeyOff(JamKey::LOW_CS_H);	break;
		case Qt::Key_Period:	bt_->jamKeyOff(JamKey::LOW_D_H);	break;
		case Qt::Key_Q:			bt_->jamKeyOff(JamKey::HIGH_C);		break;
		case Qt::Key_2:			bt_->jamKeyOff(JamKey::HIGH_CS);	break;
		case Qt::Key_W:			bt_->jamKeyOff(JamKey::HIGH_D);		break;
		case Qt::Key_3:			bt_->jamKeyOff(JamKey::HIGH_DS);	break;
		case Qt::Key_E:			bt_->jamKeyOff(JamKey::HIGH_E);		break;
		case Qt::Key_R:			bt_->jamKeyOff(JamKey::HIGH_F);		break;
		case Qt::Key_5:			bt_->jamKeyOff(JamKey::HIGH_FS);	break;
		case Qt::Key_T:			bt_->jamKeyOff(JamKey::HIGH_G);		break;
		case Qt::Key_6:			bt_->jamKeyOff(JamKey::HIGH_GS);	break;
		case Qt::Key_Y:			bt_->jamKeyOff(JamKey::HIGH_A);		break;
		case Qt::Key_7:			bt_->jamKeyOff(JamKey::HIGH_AS);	break;
		case Qt::Key_U:			bt_->jamKeyOff(JamKey::HIGH_B);		break;
		case Qt::Key_I:			bt_->jamKeyOff(JamKey::HIGH_C_H);	break;
		case Qt::Key_9:			bt_->jamKeyOff(JamKey::HIGH_CS_H);	break;
		case Qt::Key_O:			bt_->jamKeyOff(JamKey::HIGH_D_H);	break;
		default:	break;
		}
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	auto mime = event->mimeData();
	if (mime->hasUrls() && mime->urls().length() == 1
			&& mime->urls().first().toLocalFile().endsWith(".btm"))
		event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
	if (isWindowModified()) {
		auto modTitleStd = bt_->getModuleTitle();
		QString modTitle = QString::fromUtf8(modTitleStd.c_str(), modTitleStd.length());
		if (modTitle.isEmpty()) modTitle = "Untitled";
		QMessageBox dialog(QMessageBox::Warning,
						   "BambooTracker",
						   "Save changes to " + modTitle + "?",
						   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		switch (dialog.exec()) {
		case QMessageBox::Yes:
			if (!on_actionSave_triggered()) return;
			break;
		case QMessageBox::No:
			break;
		case QMessageBox::Cancel:
			return;
		default:
			break;
		}
	}

	bt_->stopPlaySong();
	lockControls(false);
	if (bt_->loadModule(event->mimeData()->urls().first().toLocalFile().toStdString())) {
		isModifiedForNotCommand_ = false;
		setWindowModified(false);
		loadModule();
	}
	else {
		QMessageBox::critical(this, "Error", "Failed to load module.");
	}
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	if (!isMaximized()) {	// Check previous size
		config_->setMainWindowWidth(event->oldSize().width());
		config_->setMainWindowHeight(event->oldSize().height());
	}
}

void MainWindow::moveEvent(QMoveEvent* event)
{
	QWidget::moveEvent(event);

	if (!isMaximized()) {	// Check previous position
		config_->setMainWindowX(event->oldPos().x());
		config_->setMainWindowY(event->oldPos().y());
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (isWindowModified()) {
		auto modTitleStd = bt_->getModuleTitle();
		QString modTitle = QString::fromUtf8(modTitleStd.c_str(), modTitleStd.length());
		if (modTitle.isEmpty()) modTitle = "Untitled";
		QMessageBox dialog(QMessageBox::Warning,
						   "BambooTracker",
						   "Save changes to " + modTitle + "?",
						   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		switch (dialog.exec()) {
		case QMessageBox::Yes:
			if (!on_actionSave_triggered()) return;
			break;
		case QMessageBox::No:
			break;
		case QMessageBox::Cancel:
			event->ignore();
			return;
		default:
			break;
		}
	}

	if (isMaximized()) {
		config_->setMainWindowMaximized(true);
	}
	else {
		config_->setMainWindowMaximized(false);
		config_->setMainWindowWidth(width());
		config_->setMainWindowHeight(height());
		config_->setMainWindowX(x());
		config_->setMainWindowY(y());
	}
	config_->setFollowMode(bt_->isFollowPlay());

	instForms_->closeAll();
	Json::saveConfiguration(config_);

	event->accept();
}

/********** Instrument list **********/
void MainWindow::addInstrument()
{
	switch (bt_->getCurrentTrackAttribute().source) {
	case SoundSource::FM:
	case SoundSource::SSG:
	{
		auto& list = ui->instrumentListWidget;

		int num = bt_->findFirstFreeInstrumentNumber();
		QString name = "Instrument " + QString::number(num);
		bt_->addInstrument(num, name.toUtf8().toStdString());

		TrackAttribute attrib = bt_->getCurrentTrackAttribute();
		comStack_->push(new AddInstrumentQtCommand(list, num, name, attrib.source, instForms_));
		break;
	}
	case SoundSource::DRUM:
		break;
	}
}

void MainWindow::removeInstrument(int row)
{
	auto& list = ui->instrumentListWidget;
	int num = list->item(row)->data(Qt::UserRole).toInt();

	bt_->removeInstrument(num);
	comStack_->push(new RemoveInstrumentQtCommand(list, num, row, instForms_));
}

void MainWindow::editInstrument()
{
	auto item = ui->instrumentListWidget->currentItem();
	int num = item->data(Qt::UserRole).toInt();
	instForms_->showForm(num);
}

int MainWindow::findRowFromInstrumentList(int instNum)
{
	auto& list = ui->instrumentListWidget;
	int row = 0;
	for (; row < list->count(); ++row) {
		auto item = list->item(row);
		if (item->data(Qt::UserRole).toInt() == instNum) break;
	}
	return row;
}

void MainWindow::editInstrumentName()
{
	auto list = ui->instrumentListWidget;
	auto item = list->currentItem();
	int num = item->data(Qt::UserRole).toInt();
	QString oldName = instForms_->getFormInstrumentName(num);
    auto line = new QLineEdit(oldName);

	QObject::connect(line, &QLineEdit::editingFinished,
                     this, [&, item, list, num, oldName]() {
		QString newName = qobject_cast<QLineEdit*>(list->itemWidget(item))->text();
		list->removeItemWidget(item);
		bt_->setInstrumentName(num, newName.toUtf8().toStdString());
		int row = findRowFromInstrumentList(num);
		comStack_->push(new ChangeInstrumentNameQtCommand(list, num, row, instForms_, oldName, newName));
	});

    ui->instrumentListWidget->setItemWidget(item, line);

	line->selectAll();
	line->setFocus();
}

void MainWindow::cloneInstrument()
{
	int num = bt_->findFirstFreeInstrumentNumber();
	if (num == -1) return;

	int refNum = ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	// KEEP CODE ORDER //
	bt_->cloneInstrument(num, refNum);
	comStack_->push(new CloneInstrumentQtCommand(
						ui->instrumentListWidget, num, refNum, instForms_));
	//----------//
}

void MainWindow::deepCloneInstrument()
{
	int num = bt_->findFirstFreeInstrumentNumber();
	if (num == -1) return;

	int refNum = ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	// KEEP CODE ORDER //
	bt_->deepCloneInstrument(num, refNum);
	comStack_->push(new DeepCloneInstrumentQtCommand(
						ui->instrumentListWidget, num, refNum, instForms_));
	//----------//
}

void MainWindow::loadInstrument()
{
	QString file = QFileDialog::getOpenFileName(this, "Open instrument", "./",
												"BambooTracker instrument file (*.bti)");
	if (file.isNull()) return;

	int n = bt_->findFirstFreeInstrumentNumber();
	if (n != -1 && bt_->loadInstrument(file.toStdString(), n)) {
		auto inst = bt_->getInstrument(n);
		auto name = inst->getName();
		comStack_->push(new AddInstrumentQtCommand(ui->instrumentListWidget, n,
												   QString::fromUtf8(name.c_str(), name.length()),
												   inst->getSoundSource(), instForms_));
	}
	else {
		QMessageBox::critical(this, "Error", "Failed to load instrument.");
	}
}

void MainWindow::saveInstrument()
{
	QString file = QFileDialog::getSaveFileName(this, "Save instrument", "./",
												"BambooTracker instrument file (*.bti)");
	if (file.isNull()) return;
	if (!file.endsWith(".bti")) file += ".bti";	// For linux

	if (!bt_->saveInstrument(file.toStdString(),
							 ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt()))
		QMessageBox::critical(this, "Error", "Failed to save instrument.");
}

/********** Undo-Redo **********/
void MainWindow::undo()
{
	bt_->undo();
	comStack_->undo();
}

void MainWindow::redo()
{
	bt_->redo();
	comStack_->redo();
}

/********** Load data **********/
void MainWindow::loadModule()
{
	instForms_->clearAll();
	ui->instrumentListWidget->clear();

	auto modTitle = bt_->getModuleTitle();
	ui->modTitleLineEdit->setText(QString::fromUtf8(modTitle.c_str(), modTitle.length()));
	auto modAuthor = bt_->getModuleAuthor();
	ui->authorLineEdit->setText(QString::fromUtf8(modAuthor.c_str(), modAuthor.length()));
	auto modCopyright = bt_->getModuleCopyright();
	ui->copyrightLineEdit->setText(QString::fromUtf8(modCopyright.c_str(), modCopyright.length()));
	ui->songNumSpinBox->setMaximum(bt_->getSongCount() - 1);
	ui->stepHighrightSpinBox->setValue(bt_->getModuleStepHighlightDistance());

	for (auto& idx : bt_->getInstrumentIndices()) {
		auto inst = bt_->getInstrument(idx);
		auto name = inst->getName();
		comStack_->push(new AddInstrumentQtCommand(
							ui->instrumentListWidget, idx, QString::fromUtf8(name.c_str(), name.length()),
							inst->getSoundSource(), instForms_));
	}

	isSavedModBefore_ = false;

	loadSong();

	// Clear records
	QApplication::clipboard()->clear();
	comStack_->clear();
	bt_->clearCommandHistory();
}

void MainWindow::loadSong()
{
	// Init position
	if (ui->songNumSpinBox->value() >= bt_->getSongCount())
		bt_->setCurrentSongNumber(bt_->getSongCount() - 1);
	else
		bt_->setCurrentSongNumber(bt_->getCurrentSongNumber());
	bt_->setCurrentOrderNumber(0);
	bt_->setCurrentTrack(0);
	bt_->setCurrentStepNumber(0);

	// Init ui
	int curSong = bt_->getCurrentSongNumber();
	ui->songNumSpinBox->setValue(curSong);
	auto title = bt_->getSongTitle(curSong);
	ui->songTitleLineEdit->setText(QString::fromUtf8(title.c_str(), title.length()));
	switch (bt_->getSongStyle(curSong).type) {
	case SongType::STD:		ui->songStyleLineEdit->setText("Standard");			break;
	case SongType::FMEX:	ui->songStyleLineEdit->setText("FM3ch extension");	break;
	}
	ui->tickFreqSpinBox->setValue(bt_->getModuleTickFrequency());
	ui->tempoSpinBox->setValue(bt_->getSongTempo(curSong));
	ui->speedSpinBox->setValue(bt_->getSongSpeed(curSong));
	ui->patternSizeSpinBox->setValue(bt_->getDefaultPatternSize(curSong));
	ui->grooveSpinBox->setValue(bt_->getSongGroove(curSong));
	ui->grooveSpinBox->setMaximum(bt_->getGrooveCount() - 1);
	if (bt_->isUsedTempoInSong(curSong)) {
		ui->tickFreqSpinBox->setEnabled(true);
		ui->tempoSpinBox->setEnabled(true);
		ui->speedSpinBox->setEnabled(true);
		ui->grooveCheckBox->setChecked(false);
		ui->grooveSpinBox->setEnabled(false);
	}
	else {

		ui->tickFreqSpinBox->setEnabled(false);
		ui->tempoSpinBox->setEnabled(false);
		ui->speedSpinBox->setEnabled(false);
		ui->grooveCheckBox->setChecked(true);
		ui->grooveSpinBox->setEnabled(true);
	}
	ui->orderList->onSongLoaded();
	ui->patternEditor->onSongLoaded();

	setWindowTitle();
}

/********** Play song **********/
void MainWindow::startPlaySong()
{
	bt_->startPlaySong();
	ui->patternEditor->updatePosition();
	lockControls(true);
}

void MainWindow::startPlayFromStart()
{
	bt_->startPlayFromStart();
	ui->patternEditor->updatePosition();
	lockControls(true);
}

void MainWindow::startPlayPattern()
{
	bt_->startPlayPattern();
	ui->patternEditor->updatePosition();
	lockControls(true);
}

void MainWindow::startPlayFromCurrentStep()
{
	bt_->startPlayFromCurrentStep();
	lockControls(true);
}

void MainWindow::stopPlaySong()
{
	bt_->stopPlaySong();
	lockControls(false);
	ui->patternEditor->update();
	ui->orderList->update();
}

void MainWindow::lockControls(bool isLock)
{
	ui->modSetDialogOpenToolButton->setEnabled(!isLock);
	ui->songNumSpinBox->setEnabled(!isLock);
}

/********** Octave change **********/
void MainWindow::changeOctave(bool upFlag)
{
	if (upFlag) ui->octaveSpinBox->stepUp();
	else ui->octaveSpinBox->stepDown();
}

/********** Configuration change **********/
void MainWindow::changeConfiguration()
{
	stream_->setRate(config_->getSampleRate());
	stream_->setDuration(config_->getBufferLength());
	stream_->setDevice(
				QString::fromUtf8(config_->getSoundDevice().c_str(), config_->getSoundDevice().length()));
	bt_->changeConfiguration(config_);
}

/******************************/
void MainWindow::setWindowTitle()
{
	int n = bt_->getCurrentSongNumber();
	auto filePathStd = bt_->getModulePath();
	auto songTitleStd = bt_->getSongTitle(n);
	QString filePath = QString::fromUtf8(filePathStd.c_str(), filePathStd.length());
	QString fileName = filePath.isEmpty() ? "Untitled" : QFileInfo(filePath).fileName();
	QString songTitle = QString::fromUtf8(songTitleStd.c_str(), songTitleStd.length());
	if (songTitle.isEmpty()) songTitle = "Untitled";
	QMainWindow::setWindowTitle(QString("%1[*] [#%2 %3] - BambooTracker")
								.arg(fileName).arg(QString::number(n)).arg(songTitle));
}

void MainWindow::setModifiedTrue()
{
	isModifiedForNotCommand_ = true;
	setWindowModified(true);
}

/******************************/
/********** Instrument list events **********/
void MainWindow::on_instrumentListWidget_customContextMenuRequested(const QPoint &pos)
{
	auto& list = ui->instrumentListWidget;
	QPoint globalPos = list->mapToGlobal(pos);
	QMenu menu;

	QAction* add = menu.addAction("Add", this, &MainWindow::addInstrument);
	QAction* remove = menu.addAction("Remove", this, [&]() {
		removeInstrument(ui->instrumentListWidget->currentRow());
	});
    menu.addSeparator();
	QAction* name = menu.addAction("Edit name", this, &MainWindow::editInstrumentName);
    menu.addSeparator();
	QAction* clone = menu.addAction("Clone", this, &MainWindow::cloneInstrument);
	QAction* dClone = menu.addAction("Deep clone", this, &MainWindow::deepCloneInstrument);
    menu.addSeparator();
	QAction* ldFile = menu.addAction("Load from file...", this, &MainWindow::loadInstrument);
	QAction* svFile = menu.addAction("Save to file...", this, &MainWindow::saveInstrument);
    menu.addSeparator();
	QAction* edit = menu.addAction("Edit...", this, &MainWindow::editInstrument);

	if (bt_->findFirstFreeInstrumentNumber() == -1) {    // Max size
		add->setEnabled(false);
		ldFile->setEnabled(false);
	}
	else {
		switch (bt_->getCurrentTrackAttribute().source) {
		case SoundSource::DRUM:	add->setEnabled(false);	break;
		default:	break;
		}
	}
	auto item = list->currentItem();
	if (item == nullptr) {    // Not selected
		remove->setEnabled(false);
		name->setEnabled(false);
		svFile->setEnabled(false);
		edit->setEnabled(false);
    }
	if (item == nullptr || bt_->findFirstFreeInstrumentNumber() == -1) {
		clone->setEnabled(false);
		dClone->setEnabled(false);
	}

	menu.exec(globalPos);
}

void MainWindow::on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item)
{
	Q_UNUSED(item)
	editInstrument();
}

void MainWindow::onInstrumentListWidgetItemAdded(const QModelIndex &parent, int start, int end)
{
	Q_UNUSED(parent)
	Q_UNUSED(end)

	// Set core data to editor when add insrument
	int n = ui->instrumentListWidget->item(start)->data(Qt::UserRole).toInt();
	auto& form = instForms_->getForm(n);
	switch (instForms_->getFormInstrumentSoundSource(n)) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form.get());
		fmForm->resize(config_->getInstrumentFMWindowWidth(), config_->getInstrumentFMWindowHeight());
		QObject::connect(fmForm, &InstrumentEditorFMForm::envelopeNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMEnvelopeNumberChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::envelopeParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMEnvelopeParameterChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::lfoNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMLFONumberChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::lfoParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMLFOParameterChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::operatorSequenceNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMOperatorSequenceNumberChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::operatorSequenceParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMOperatorSequenceParameterChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::arpeggioNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMArpeggioNumberChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::arpeggioParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMArpeggioParameterChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::pitchNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMPitchNumberChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::pitchParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentFMPitchParameterChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::jamKeyOnEvent,
						 this, &MainWindow::keyPressEvent, Qt::DirectConnection);
		QObject::connect(fmForm, &InstrumentEditorFMForm::jamKeyOffEvent,
						 this, &MainWindow::keyReleaseEvent, Qt::DirectConnection);
		QObject::connect(fmForm, &InstrumentEditorFMForm::octaveChanged,
						 this, &MainWindow::changeOctave, Qt::DirectConnection);
		QObject::connect(fmForm, &InstrumentEditorFMForm::modified,
						 this, &MainWindow::setModifiedTrue);

		fmForm->installEventFilter(this);
		fmForm->setCore(bt_);

		instForms_->onInstrumentFMEnvelopeNumberChanged();
		instForms_->onInstrumentFMLFONumberChanged();
		instForms_->onInstrumentFMOperatorSequenceNumberChanged();
		instForms_->onInstrumentFMArpeggioNumberChanged();
		instForms_->onInstrumentFMPitchNumberChanged();
		break;
	}
	case SoundSource::SSG:
	{
		auto ssgForm = qobject_cast<InstrumentEditorSSGForm*>(form.get());
		ssgForm->resize(config_->getInstrumentSSGWindowWidth(), config_->getInstrumentSSGWindowHeight());
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::waveFormNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGWaveFormNumberChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::waveFormParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGWaveFormParameterChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::toneNoiseNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGToneNoiseNumberChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::toneNoiseParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGToneNoiseParameterChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::envelopeNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGEnvelopeNumberChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::envelopeParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGEnvelopeParameterChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::arpeggioNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGArpeggioNumberChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::arpeggioParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGArpeggioParameterChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::pitchNumberChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGPitchNumberChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::pitchParameterChanged,
						 instForms_.get(), &InstrumentFormManager::onInstrumentSSGPitchParameterChanged);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::jamKeyOnEvent,
						 this, &MainWindow::keyPressEvent, Qt::DirectConnection);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::jamKeyOffEvent,
						 this, &MainWindow::keyReleaseEvent, Qt::DirectConnection);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::octaveChanged,
						 this, &MainWindow::changeOctave, Qt::DirectConnection);
		QObject::connect(ssgForm, &InstrumentEditorSSGForm::modified,
						 this, &MainWindow::setModifiedTrue);

		ssgForm->installEventFilter(this);
		ssgForm->setCore(bt_);

		instForms_->onInstrumentSSGWaveFormNumberChanged();
		instForms_->onInstrumentSSGToneNoiseNumberChanged();
		instForms_->onInstrumentSSGEnvelopeNumberChanged();
		instForms_->onInstrumentSSGArpeggioNumberChanged();
		instForms_->onInstrumentSSGPitchNumberChanged();
		break;
	}
	default:
		break;
	}
}

void MainWindow::on_instrumentListWidget_itemSelectionChanged()
{
	int num = (ui->instrumentListWidget->currentRow() == -1)
			  ? -1
			  : ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	bt_->setCurrentInstrument(num);

	bool isEnabled = (num != -1);
	ui->actionRemove_Instrument->setEnabled(isEnabled);
	ui->actionClone_Instrument->setEnabled(isEnabled);
	ui->actionDeep_Clone_Instrument->setEnabled(isEnabled);
	ui->actionSave_To_File->setEnabled(isEnabled);
	ui->actionEdit->setEnabled(isEnabled);
}

void MainWindow::on_grooveCheckBox_stateChanged(int arg1)
{
	if (arg1 == Qt::Checked) {
		ui->tickFreqSpinBox->setValue(60);
		ui->tickFreqSpinBox->setEnabled(false);
		ui->tempoSpinBox->setValue(150);
		ui->tempoSpinBox->setEnabled(false);
		ui->speedSpinBox->setEnabled(false);
		ui->grooveSpinBox->setEnabled(true);
		bt_->toggleTempoOrGrooveInSong(bt_->getCurrentSongNumber(), false);
	}
	else {
		ui->tickFreqSpinBox->setEnabled(true);
		ui->tempoSpinBox->setEnabled(true);
		ui->speedSpinBox->setEnabled(true);
		ui->grooveSpinBox->setEnabled(false);
		bt_->toggleTempoOrGrooveInSong(bt_->getCurrentSongNumber(), true);
	}

	setModifiedTrue();
}

void MainWindow::on_actionExit_triggered()
{
	close();
}

void MainWindow::on_actionUndo_triggered()
{
	undo();
}

void MainWindow::on_actionRedo_triggered()
{
	redo();
}

void MainWindow::on_actionCut_triggered()
{
	if (isEditedPattern_) ui->patternEditor->cutSelectedCells();
}

void MainWindow::on_actionCopy_triggered()
{
	if (isEditedPattern_) ui->patternEditor->copySelectedCells();
	else if (isEditedOrder_) ui->orderList->copySelectedCells();
}

void MainWindow::on_actionPaste_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onPastePressed();
	else if (isEditedOrder_) ui->orderList->onPastePressed();
}

void MainWindow::on_actionPaste_Mix_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onPasteMixPressed();
}

void MainWindow::on_actionDelete_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onDeletePressed();
	else if (isEditedOrder_) ui->orderList->deleteOrder();
}

void MainWindow::updateMenuByPattern()
{
	isEditedPattern_ = true;
	isEditedOrder_ = false;

	if (bt_->isJamMode()) {
		// Edit
		ui->actionPaste->setEnabled(false);
		ui->actionPaste_Mix->setEnabled(false);
		ui->actionDelete->setEnabled(false);
		// Pattern
		ui->actionInterpolate->setEnabled(false);
		ui->actionReverse->setEnabled(false);
		ui->actionReplace_Instrument->setEnabled(false);
		ui->actionExpand->setEnabled(false);
		ui->actionShrink->setEnabled(false);
		ui->actionDecrease_Note->setEnabled(false);
		ui->actionIncrease_Note->setEnabled(false);
		ui->actionDecrease_Octave->setEnabled(false);
		ui->actionIncrease_Octave->setEnabled(false);
	}
	else {
		// Edit
		bool enabled = QApplication::clipboard()->text().startsWith("PATTERN_");
		ui->actionPaste->setEnabled(enabled);
		ui->actionPaste_Mix->setEnabled(enabled);
		ui->actionDelete->setEnabled(true);
		// Pattern
		ui->actionInterpolate->setEnabled(isSelectedPO_);
		ui->actionReverse->setEnabled(isSelectedPO_);
		ui->actionReplace_Instrument->setEnabled(
					isSelectedPO_ && ui->instrumentListWidget->currentRow() != -1);
		ui->actionExpand->setEnabled(isSelectedPO_);
		ui->actionShrink->setEnabled(isSelectedPO_);
		ui->actionDecrease_Note->setEnabled(true);
		ui->actionIncrease_Note->setEnabled(true);
		ui->actionDecrease_Octave->setEnabled(true);
		ui->actionIncrease_Octave->setEnabled(true);
	}

	// Song
	ui->actionInsert_Order->setEnabled(false);
	ui->actionRemove_Order->setEnabled(false);
	ui->actionDuplicate_Order->setEnabled(false);
	ui->actionMove_Order_Up->setEnabled(false);
	ui->actionMove_Order_Down->setEnabled(false);
	ui->actionClone_Patterns->setEnabled(false);
	ui->actionClone_Order->setEnabled(false);
}

void MainWindow::updateMenuByOrder()
{
	isEditedPattern_ = false;
	isEditedOrder_ = true;

	if (bt_->isJamMode()) {
		// Edit
		ui->actionPaste->setEnabled(false);
		ui->actionDelete->setEnabled(false);
		// Song
		ui->actionInsert_Order->setEnabled(false);
		ui->actionRemove_Order->setEnabled(false);
		ui->actionDuplicate_Order->setEnabled(false);
		ui->actionMove_Order_Up->setEnabled(false);
		ui->actionMove_Order_Down->setEnabled(false);
		ui->actionClone_Patterns->setEnabled(false);
		ui->actionClone_Order->setEnabled(false);
	}
	else {
		// Edit
		bool enabled = QApplication::clipboard()->text().startsWith("ORDER_");
		ui->actionPaste->setEnabled(enabled);
		ui->actionDelete->setEnabled(true);
		// Song
		ui->actionInsert_Order->setEnabled(true);
		ui->actionRemove_Order->setEnabled(true);
		ui->actionDuplicate_Order->setEnabled(true);
		ui->actionMove_Order_Up->setEnabled(true);
		ui->actionMove_Order_Down->setEnabled(true);
		ui->actionClone_Patterns->setEnabled(true);
		ui->actionClone_Order->setEnabled(true);
	}
	// Edit
	ui->actionPaste_Mix->setEnabled(false);

	// Pattern
	ui->actionInterpolate->setEnabled(false);
	ui->actionReverse->setEnabled(false);
	ui->actionReplace_Instrument->setEnabled(false);
	ui->actionExpand->setEnabled(false);
	ui->actionShrink->setEnabled(false);
	ui->actionDecrease_Note->setEnabled(false);
	ui->actionIncrease_Note->setEnabled(false);
	ui->actionDecrease_Octave->setEnabled(false);
	ui->actionIncrease_Octave->setEnabled(false);
}

void MainWindow::onPatternAndOrderFocusLost()
{
	/*
	ui->actionCopy->setEnabled(false);
	ui->actionCut->setEnabled(false);
	ui->actionPaste->setEnabled(false);
	ui->actionPaste_Mix->setEnabled(false);
	ui->actionDelete->setEnabled(false);
	*/
}

void MainWindow::updateMenuByPatternAndOrderSelection(bool isSelected)
{
	isSelectedPO_ = isSelected;

	if (bt_->isJamMode()) {
		// Edit
		ui->actionCopy->setEnabled(false);
		ui->actionCut->setEnabled(false);
		// Pattern
		ui->actionInterpolate->setEnabled(false);
		ui->actionReverse->setEnabled(false);
		ui->actionReplace_Instrument->setEnabled(false);
		ui->actionExpand->setEnabled(false);
		ui->actionShrink->setEnabled(false);
	}
	else {
		// Edit
		ui->actionCopy->setEnabled(isSelected);
		ui->actionCut->setEnabled(isEditedPattern_ ? isSelected : false);
		// Pattern
		bool enabled = (isEditedPattern_ && isEditedPattern_) ? isSelected : false;
		ui->actionInterpolate->setEnabled(enabled);
		ui->actionReverse->setEnabled(enabled);
		ui->actionReplace_Instrument->setEnabled(
					enabled && ui->instrumentListWidget->currentRow() != -1);
		ui->actionExpand->setEnabled(enabled);
		ui->actionShrink->setEnabled(enabled);
	}
}

void MainWindow::on_actionAll_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onSelectPressed(1);
	else if (isEditedOrder_) ui->orderList->onSelectPressed(1);
}

void MainWindow::on_actionNone_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onSelectPressed(0);
	else if (isEditedOrder_) ui->orderList->onSelectPressed(0);
}

void MainWindow::on_actionDecrease_Note_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onTransposePressed(false, false);
}

void MainWindow::on_actionIncrease_Note_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onTransposePressed(false, true);
}

void MainWindow::on_actionDecrease_Octave_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onTransposePressed(true, false);
}

void MainWindow::on_actionIncrease_Octave_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onTransposePressed(true, true);
}

void MainWindow::on_actionInsert_Order_triggered()
{
	if (isEditedOrder_) ui->orderList->insertOrderBelow();
}

void MainWindow::on_actionRemove_Order_triggered()
{
	if (isEditedOrder_) ui->orderList->deleteOrder();
}

void MainWindow::on_actionModule_Properties_triggered()
{
	ModulePropertiesDialog dialog(bt_);
	if (dialog.exec() == QDialog::Accepted
			&& showUndoResetWarningDialog("Do you want to change song properties?")) {
		bt_->stopPlaySong();
		lockControls(false);
		loadModule();
		setModifiedTrue();
		setWindowTitle();
	}
}

void MainWindow::on_actionNew_Instrument_triggered()
{
	addInstrument();
}

void MainWindow::on_actionRemove_Instrument_triggered()
{
	removeInstrument(ui->instrumentListWidget->currentRow());
}

void MainWindow::on_actionClone_Instrument_triggered()
{
	cloneInstrument();
}

void MainWindow::on_actionDeep_Clone_Instrument_triggered()
{
	deepCloneInstrument();
}

void MainWindow::on_actionEdit_triggered()
{
	editInstrument();
}

void MainWindow::on_actionPlay_triggered()
{
	startPlaySong();
}

void MainWindow::on_actionPlay_Pattern_triggered()
{
	startPlayPattern();
}

void MainWindow::on_actionPlay_From_Start_triggered()
{
	startPlayFromStart();
}

void MainWindow::on_actionPlay_From_Cursor_triggered()
{
	startPlayFromCurrentStep();
}

void MainWindow::on_actionStop_triggered()
{
	stopPlaySong();
}

void MainWindow::on_actionEdit_Mode_triggered()
{
	bt_->toggleJamMode();
	ui->orderList->changeEditable();
	ui->patternEditor->changeEditable();

	if (isEditedOrder_) updateMenuByOrder();
	else if (isEditedPattern_) updateMenuByPattern();
	updateMenuByPatternAndOrderSelection(isSelectedPO_);
}

void MainWindow::on_actionMute_Track_triggered()
{
	ui->patternEditor->onMuteTrackPressed();
}

void MainWindow::on_actionSolo_Track_triggered()
{
	ui->patternEditor->onSoloTrackPressed();
}

void MainWindow::on_actionKill_Sound_triggered()
{
	bt_->killSound();
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, "About", QString(
					   "BambooTracker v")
					   + QString::fromStdString(Version::ofApplicationInString())
					   + QString("\n"
					   "Copyright (C) 2018 Rerrah\n"
					   "\n"
					   "Libraries:\n"
					   "- Qt (GPL v2+ or LGPL v3)\n"
					   "- VGMPlay by (C) Valley Bell (GPL v2)\n"
					   "- MAME (MAME License)\n"
					   "- Silk icon set 1.3 by (C) Mark James (CC BY 2.5)"));
}

void MainWindow::on_actionFollow_Mode_triggered()
{
	bt_->setFollowPlay(ui->actionFollow_Mode->isChecked());
}

void MainWindow::on_actionGroove_Settings_triggered()
{
	std::vector<std::vector<int>> seqs;
	for (size_t i = 0; i < bt_->getGrooveCount(); ++i) {
		seqs.push_back(bt_->getGroove(i));
	}

	GrooveSettingsDialog diag;
	diag.setGrooveSquences(seqs);
	if (diag.exec() == QDialog::Accepted) {
		bt_->stopPlaySong();
		lockControls(false);
		bt_->setGrooves(diag.getGrooveSequences());
		ui->grooveSpinBox->setMaximum(bt_->getGrooveCount() - 1);
		setModifiedTrue();
	}
}

void MainWindow::on_actionConfiguration_triggered()
{
	ConfigurationDialog diag(config_);
	QObject::connect(&diag, &ConfigurationDialog::applyPressed, this, &MainWindow::changeConfiguration);

	if (diag.exec() == QDialog::Accepted) {
		bt_->stopPlaySong();
		changeConfiguration();
		Json::saveConfiguration(config_);
		lockControls(false);
	}
}

void MainWindow::on_actionExpand_triggered()
{
	ui->patternEditor->onExpandPressed();
}

void MainWindow::on_actionShrink_triggered()
{
	ui->patternEditor->onShrinkPressed();
}

void MainWindow::on_actionDuplicate_Order_triggered()
{
	ui->orderList->onDuplicatePressed();
}

void MainWindow::on_actionMove_Order_Up_triggered()
{
	ui->orderList->onMoveOrderPressed(true);
}

void MainWindow::on_actionMove_Order_Down_triggered()
{
	ui->orderList->onMoveOrderPressed(false);
}

void MainWindow::on_actionClone_Patterns_triggered()
{
	ui->orderList->onClonePatternsPressed();
}

void MainWindow::on_actionClone_Order_triggered()
{
	ui->orderList->onCloneOrderPressed();
}

void MainWindow::on_actionNew_triggered()
{
	if (isWindowModified()) {
		auto modTitleStd = bt_->getModuleTitle();
		QString modTitle = QString::fromUtf8(modTitleStd.c_str(), modTitleStd.length());
		if (modTitle.isEmpty()) modTitle = "Untitled";
		QMessageBox dialog(QMessageBox::Warning,
						   "BambooTracker",
						   "Save changes to " + modTitle + "?",
						   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		switch (dialog.exec()) {
		case QMessageBox::Yes:
			if (!on_actionSave_triggered()) return;
			break;
		case QMessageBox::No:
			break;
		case QMessageBox::Cancel:
			return;
		default:
			break;
		}
	}

	bt_->stopPlaySong();
	lockControls(false);
	bt_->makeNewModule();
	isModifiedForNotCommand_ = false;
	setWindowModified(false);
	loadModule();
}

void MainWindow::on_actionComments_triggered()
{
	auto comment = bt_->getModuleComment();
	CommentEditDialog diag(QString::fromUtf8(comment.c_str(), comment.length()));
	if (diag.exec() == QDialog::Accepted) {
		bt_->setModuleComment(diag.getComment().toUtf8().toStdString());
		setModifiedTrue();
	}
}

bool MainWindow::on_actionSave_triggered()
{
	auto path = QString::fromStdString(bt_->getModulePath());
	if (!path.isEmpty() && QFileInfo::exists(path) && QFileInfo(path).isFile()) {
		if (!isSavedModBefore_ && config_->getBackupModules()) {
			if (!bt_->backupModule(path.toStdString())) {
				QMessageBox::critical(this, "Error", "Failed to backup module.");
				return false;
			}
		}

		if (bt_->saveModule(bt_->getModulePath())) {
			isModifiedForNotCommand_ = false;
			isSavedModBefore_ = true;
			setWindowModified(false);
			setWindowTitle();
			return true;
		}
		else {
			QMessageBox::critical(this, "Error", "Failed to save module.");
			return false;
		}
	}
	else {
		return on_actionSave_As_triggered();
	}
}

bool MainWindow::on_actionSave_As_triggered()
{
	QString file = QFileDialog::getSaveFileName(this, "Save module", "./",
												"BambooTracker module file (*.btm)");
	if (file.isNull()) return false;
	if (!file.endsWith(".btm")) file += ".btm";	// For linux

	if (std::ifstream(file.toStdString()).is_open()) {	// Already exists
		if (!isSavedModBefore_ && config_->getBackupModules()) {
			if (!bt_->backupModule(file.toStdString())) {
				QMessageBox::critical(this, "Error", "Failed to backup module.");
				return false;
			}
		}
	}

	bt_->setModulePath(file.toStdString());
	if (bt_->saveModule(bt_->getModulePath())) {
		isModifiedForNotCommand_ = false;
		isSavedModBefore_ = true;
		setWindowModified(false);
		setWindowTitle();
		return true;
	}
	else {
		QMessageBox::critical(this, "Error", "Failed to save module.");
		return false;
	}
}

void MainWindow::on_actionOpen_triggered()
{
	if (isWindowModified()) {
		auto modTitleStd = bt_->getModuleTitle();
		QString modTitle = QString::fromUtf8(modTitleStd.c_str(), modTitleStd.length());
		if (modTitle.isEmpty()) modTitle = "Untitled";
		QMessageBox dialog(QMessageBox::Warning,
						   "BambooTracker",
						   "Save changes to " + modTitle + "?",
						   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		switch (dialog.exec()) {
		case QMessageBox::Yes:
			if (!on_actionSave_triggered()) return;
			break;
		case QMessageBox::No:
			break;
		case QMessageBox::Cancel:
			return;
		default:
			break;
		}
	}

	QString file = QFileDialog::getOpenFileName(this, "Open module", "./",
												"BambooTracker module file (*.btm)");
	if (file.isNull()) return;

	bt_->stopPlaySong();
	lockControls(false);
	if (bt_->loadModule(file.toStdString())) {
		isModifiedForNotCommand_ = false;
		setWindowModified(false);
		loadModule();
	}
	else {
		QMessageBox::critical(this, "Error", "Failed to load module.");
	}
}

void MainWindow::on_actionLoad_From_File_triggered()
{
	loadInstrument();
}

void MainWindow::on_actionSave_To_File_triggered()
{
	saveInstrument();
}

void MainWindow::on_actionInterpolate_triggered()
{
	ui->patternEditor->onInterpolatePressed();
}

void MainWindow::on_actionReverse_triggered()
{
	ui->patternEditor->onReversePressed();
}

void MainWindow::on_actionReplace_Instrument_triggered()
{
	ui->patternEditor->onReplaceInstrumentPressed();
}

void MainWindow::on_actionRow_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onSelectPressed(2);
	else if (isEditedOrder_) ui->orderList->onSelectPressed(2);
}

void MainWindow::on_actionColumn_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onSelectPressed(3);
	else if (isEditedOrder_) ui->orderList->onSelectPressed(3);
}

void MainWindow::on_actionPattern_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onSelectPressed(4);
	else if (isEditedOrder_) ui->orderList->onSelectPressed(4);
}

void MainWindow::on_actionOrder_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onSelectPressed(5);
	else if (isEditedOrder_) ui->orderList->onSelectPressed(5);
}

void MainWindow::on_actionRemove_Unused_Instruments_triggered()
{
	if (showUndoResetWarningDialog("Do you want to remove all unused instruments?"))
	{
		bt_->stopPlaySong();
		lockControls(false);

		auto list = ui->instrumentListWidget;
		for (auto& n : bt_->getUnusedInstrumentIndices()) {
			for (int i = 0; i < list->count(); ++i) {
				if (list->item(i)->data(Qt::UserRole).toInt() == n) {
					removeInstrument(i);
				}
			}
		}
		bt_->clearUnusedInstrumentProperties();
		bt_->clearCommandHistory();
		comStack_->clear();
		setModifiedTrue();
	}
}

void MainWindow::on_actionRemove_Unused_Patterns_triggered()
{
	if (showUndoResetWarningDialog("Do you want to remove all unused patterns?"))
	{
		bt_->stopPlaySong();
		lockControls(false);

		bt_->clearUnusedPatterns();
		bt_->clearCommandHistory();
		comStack_->clear();
		setModifiedTrue();
	}
}

void MainWindow::on_actionWAV_triggered()
{
	WaveExportSettingsDialog diag;
	if (diag.exec() != QDialog::Accepted) return;

	QString file = QFileDialog::getSaveFileName(this, "Export to wav", "./",
												"WAV signed 16-bit PCM (*.wav)");
	if (file.isNull()) return;
	if (!file.endsWith(".wav")) file += ".wav";	// For linux

	QProgressDialog progress(
				"Export to WAV",
				"Cancel",
				0,
				bt_->getAllStepCount(bt_->getCurrentSongNumber()) * diag.getLoopCount() + 3
				);
	progress.setValue(0);
	progress.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	progress.setWindowFlag(Qt::WindowCloseButtonHint, false);
	progress.show();

	bt_->stopPlaySong();
	lockControls(false);
	stream_->stop();

	bool res = bt_->exportToWav(file.toStdString(), diag.getLoopCount(),
								[&progress]() -> bool {
									QApplication::processEvents();
									progress.setValue(progress.value() + 1);
									return progress.wasCanceled();
								});
	if (!res) QMessageBox::critical(this, "Error", "Failed to export to wav file.");

	stream_->start();
}

void MainWindow::on_actionVGM_triggered()
{
	VgmExportSettingsDialog diag;
	if (diag.exec() != QDialog::Accepted) return;
	GD3Tag tag = diag.getGD3Tag();

	QString file = QFileDialog::getSaveFileName(this, "Export to vgm", "./",
												"VGM file (*.vgm)");
	if (file.isNull()) return;
	if (!file.endsWith(".vgm")) file += ".vgm";	// For linux

	QProgressDialog progress(
				"Export to VGM",
				"Cancel",
				0,
				bt_->getAllStepCount(bt_->getCurrentSongNumber()) + 3
				);
	progress.setValue(0);
	progress.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	progress.setWindowFlag(Qt::WindowCloseButtonHint, false);
	progress.show();

	bt_->stopPlaySong();
	lockControls(false);
	stream_->stop();

	bool res = bt_->exportToVgm(file.toStdString(),
								diag.enabledGD3(),
								tag,
								[&progress]() -> bool {
									QApplication::processEvents();
									progress.setValue(progress.value() + 1);
									return progress.wasCanceled();
								});
	if (!res) QMessageBox::critical(this, "Error", "Failed to export to vgm file.");

	stream_->start();
}
