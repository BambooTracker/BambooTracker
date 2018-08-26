#include "mainwindow.hpp"
#include <QString>
#include <QLineEdit>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QMessageBox>
#include "ui_mainwindow.h"
#include "jam_manager.hpp"
#include "song.hpp"
#include "track.hpp"
#include "instrument.hpp"
#include "./command/commands_qt.hpp"
#include "gui/module_settings_dialog.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	bt_(std::make_shared<BambooTracker>()),
	comStack_(std::make_shared<QUndoStack>(this)),
	isModifiedForNotCommand_(false)
{
	ui->setupUi(this);

	/* Command stack */
	QObject::connect(comStack_.get(), &QUndoStack::indexChanged,
					 this, [&](int idx) { setWindowModified(idx || isModifiedForNotCommand_); });

	/* Audio stream */
	stream_ = std::make_unique<AudioStream>(bt_->getStreamRate(),
											bt_->getStreamDuration(),
											bt_->getStreamInterruptRate());
	QObject::connect(stream_.get(), &AudioStream::streamInterrupted,
					 this, [&]() {
		if (!bt_->streamCountUp()) ui->patternEditor->updatePosition();
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

	/* Song number */
	QObject::connect(ui->songNumSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int num) {
		bt_->setCurrentSongNumber(num);
		loadSong();
	});

	/* Song settings */
	QObject::connect(ui->tickFreqSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int freq) {
		int curSong = bt_->getCurrentSongNumber();
		if (freq != bt_->getSongTickFrequency(curSong)) {
			bt_->setSongTickFrequency(curSong, freq);
			setModifiedTrue();
		}
	});
	QObject::connect(ui->tempoSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int tempo) {
		int curSong = bt_->getCurrentSongNumber();
		if (tempo != bt_->getSongtempo(curSong)) {
			bt_->setSongTempo(curSong, tempo);
			setModifiedTrue();
		}
	});
	QObject::connect(ui->stepSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int size) {
		int curSong = bt_->getCurrentSongNumber();
		if (size != bt_->getSongStepSize(curSong)) {
			bt_->setSongStepSize(curSong, size);
			setModifiedTrue();
		}
	});
	QObject::connect(ui->patternSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int size) {
		bt_->setDefaultPatternSize(bt_->getCurrentSongNumber(), size);
		ui->patternEditor->onDefaultPatternSizeChanged();
		setModifiedTrue();
	});

	/* Octave */
	ui->octaveSpinBox->setValue(bt_->getCurrentOctave());
	QObject::connect(ui->octaveSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
					 this, [&](int octave) { bt_->setCurrentOctave(octave); });

	/* Instrument list */
	ui->instrumentListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->instrumentListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->instrumentListWidget->setFocusPolicy(Qt::NoFocus);
	// Set core data to editor when add insrument
	QObject::connect(ui->instrumentListWidget->model(), &QAbstractItemModel::rowsInserted,
					 this, &MainWindow::onInstrumentListWidgetItemAdded);

	/* Pattern editor */
	ui->patternEditor->setCore(bt_);
	ui->patternEditor->setCommandStack(comStack_);
	QObject::connect(ui->patternEditor, &PatternEditor::currentTrackChanged,
					 ui->orderList, &OrderListEditor::setCurrentTrack);
	QObject::connect(ui->patternEditor, &PatternEditor::currentOrderChanged,
					 ui->orderList, &OrderListEditor::setCurrentOrder);

	/* Order List */
	ui->orderList->setCore(bt_);
	ui->orderList->setCommandStack(comStack_);
	ui->orderList->installEventFilter(this);
	QObject::connect(ui->orderList, &OrderListEditor::currentTrackChanged,
					 ui->patternEditor, &PatternEditor::setCurrentTrack);
	QObject::connect(ui->orderList, &OrderListEditor::currentOrderChanged,
					 ui->patternEditor, &PatternEditor::setCurrentOrder);
	QObject::connect(ui->orderList, &OrderListEditor::orderEdited,
					 ui->patternEditor, &PatternEditor::onOrderListEdited);

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
	if (event->modifiers().testFlag(Qt::ControlModifier)) {	// Pressed ctrl
		switch (key) {
		case Qt::Key_Z:	undo();	break;
		case Qt::Key_Y:	redo();	break;
		default:	break;
		}
	}
	else {
		switch (key) {
		case Qt::Key_Space:		toggleJamMode();			break;
		case Qt::Key_Asterisk:	changeOctave(true);			break;
		case Qt::Key_Slash:		changeOctave(false);		break;
		case Qt::Key_F5:		startPlaySong();			break;
		case Qt::Key_F6:		startPlayPattern();			break;
		case Qt::Key_F7:		startPlayFromCurrentStep();	break;
		case Qt::Key_F8:		stopPlaySong();				break;

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

void MainWindow::closeEvent(QCloseEvent *ce)
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
			// UNDONE: save file
			break;
		case QMessageBox::No:
			break;
		case QMessageBox::Cancel:
			ce->ignore();
			return;
		default:
			break;
		}
	}

	for (auto& pair : instFormMap_) {
		pair.second->close();
	}
	ce->accept();
}

/********** Instrument list **********/
void MainWindow::addInstrument()
{
    auto& list = ui->instrumentListWidget;

	int num = bt_->findFirstFreeInstrumentNumber();
	QString name = "Instrument " + QString::number(num);
	bt_->addInstrument(num, name.toUtf8().toStdString());

	TrackAttribute attrib = bt_->getCurrentTrackAttribute();
	comStack_->push(new AddInstrumentQtCommand(list, num, name, attrib.source, instFormMap_));
}

void MainWindow::removeInstrument()
{
	auto& list = ui->instrumentListWidget;
	int row = list->currentRow();
	int num = list->item(row)->data(Qt::UserRole).toInt();

	bt_->removeInstrument(num);
	comStack_->push(new RemoveInstrumentQtCommand(list, num, row, instFormMap_));
}

void MainWindow::editInstrument()
{
	auto item = ui->instrumentListWidget->currentItem();
	int num = item->data(Qt::UserRole).toInt();
	auto& form = instFormMap_.at(num);

	if (form->isVisible()) {
		form->activateWindow();
	}
	else {
		// Set data before first show
		if (!form->property("Shown").toBool()) form->setProperty("Shown", true);
		form->show();
	}
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
    QString oldName = instFormMap_.at(num)->property("Name").toString();
    auto line = new QLineEdit(oldName);

	QObject::connect(line, &QLineEdit::editingFinished,
                     this, [&, item, list, num, oldName]() {
		QString newName = qobject_cast<QLineEdit*>(list->itemWidget(item))->text();
		list->removeItemWidget(item);
		bt_->setInstrumentName(num, newName.toUtf8().toStdString());
		int row = findRowFromInstrumentList(num);
        comStack_->push(new ChangeInstrumentNameQtCommand(list, num, row, instFormMap_, oldName, newName));
	});

    ui->instrumentListWidget->setItemWidget(item, line);

	line->selectAll();
	line->setFocus();
}

void MainWindow::cloneInstrument()
{
	int num = bt_->findFirstFreeInstrumentNumber();
	int refNum = ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	// KEEP CODE ORDER //
	bt_->cloneInstrument(num, refNum);
	comStack_->push(new CloneInstrumentQtCommand(
						ui->instrumentListWidget, num, refNum, instFormMap_));
	//----------//
}

void MainWindow::deepCloneInstrument()
{
	int num = bt_->findFirstFreeInstrumentNumber();
	int refNum = ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	// KEEP CODE ORDER //
	bt_->deepCloneInstrument(num, refNum);
	comStack_->push(new DeepCloneInstrumentQtCommand(
						ui->instrumentListWidget, num, refNum, instFormMap_));
	//----------//
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
	auto modTitle = bt_->getModuleTitle();
	ui->modTitleLineEdit->setText(QString::fromUtf8(modTitle.c_str(), modTitle.length()));
	auto modAuthor = bt_->getModuleAuthor();
	ui->authorLineEdit->setText(QString::fromUtf8(modAuthor.c_str(), modAuthor.length()));
	auto modCopyright = bt_->getModuleCopyright();
	ui->copyrightLineEdit->setText(QString::fromUtf8(modCopyright.c_str(), modCopyright.length()));
	ui->songNumSpinBox->setMaximum(bt_->getSongCount() - 1);

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
	switch (bt_->getSongStyle(bt_->getCurrentSongNumber()).type) {
	case SongType::STD:		ui->songStyleLineEdit->setText("Standard");			break;
	case SongType::FMEX:	ui->songStyleLineEdit->setText("FM3ch extension");	break;
	}
	ui->tickFreqSpinBox->setValue(bt_->getSongTickFrequency(bt_->getCurrentSongNumber()));
	ui->tempoSpinBox->setValue(bt_->getSongtempo(bt_->getCurrentSongNumber()));
	ui->stepSizeSpinBox->setValue(bt_->getSongStepSize(bt_->getCurrentSongNumber()));
	ui->patternSizeSpinBox->setValue(bt_->getDefaultPatternSize(bt_->getCurrentSongNumber()));
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

/********** Toggle jam mode **********/
void MainWindow::toggleJamMode()
{
	bt_->toggleJamMode();
	ui->orderList->changeEditable();
	ui->patternEditor->changeEditable();
}

/******************************/
void MainWindow::setWindowTitle()
{
	int n = bt_->getCurrentSongNumber();
	auto modTitleStd = bt_->getModuleTitle();
	auto songTitleStd = bt_->getSongTitle(n);
	QString modTitle = QString::fromUtf8(modTitleStd.c_str(), modTitleStd.length());
	if (modTitle.isEmpty()) modTitle = "Untitled";
	QString songTitle = QString::fromUtf8(songTitleStd.c_str(), songTitleStd.length());
	if (songTitle.isEmpty()) songTitle = "Untitled";
	QMainWindow::setWindowTitle(QString("%1[*] [#%2 %3] - BambooTracker")
								.arg(modTitle).arg(QString::number(n)).arg(songTitle));
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
	QAction* remove = menu.addAction("Remove", this, &MainWindow::removeInstrument);
    menu.addSeparator();
	QAction* name = menu.addAction("Edit name", this, &MainWindow::editInstrumentName);
    menu.addSeparator();
	QAction* clone = menu.addAction("Clone", this, &MainWindow::cloneInstrument);
	QAction* dClone = menu.addAction("Deep clone", this, &MainWindow::deepCloneInstrument);
    menu.addSeparator();
	QAction* ldFile = menu.addAction("Load from file...");
	QAction* svFile = menu.addAction("Save to file...");
    menu.addSeparator();
	QAction* edit = menu.addAction("Edit...", this, &MainWindow::editInstrument);

	// UNDONE --------------
	ldFile->setEnabled(false);
	svFile->setEnabled(false);
	// ---------------------

	if (bt_->findFirstFreeInstrumentNumber() == -1)    // Max size
		add->setEnabled(false);
	auto item = list->currentItem();
	if (item == nullptr) {    // Not selected
		remove->setEnabled(false);
		name->setEnabled(false);
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
	auto& form = instFormMap_.at(ui->instrumentListWidget->item(start)->data(Qt::UserRole).toInt());
	switch (static_cast<SoundSource>(form->property("SoundSource").toInt())) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form.get());
		QObject::connect(fmForm, &InstrumentEditorFMForm::instrumentFMEnvelopeParameterChanged,
						 this, &MainWindow::onInstrumentFMEnvelopeChanged);
		QObject::connect(fmForm, &InstrumentEditorFMForm::jamKeyOnEvent,
						 this, &MainWindow::keyPressEvent, Qt::DirectConnection);
		QObject::connect(fmForm, &InstrumentEditorFMForm::jamKeyOffEvent,
						 this, &MainWindow::keyReleaseEvent, Qt::DirectConnection);
		QObject::connect(fmForm, &InstrumentEditorFMForm::octaveChanged,
						 this, &MainWindow::changeOctave, Qt::DirectConnection);
		fmForm->installEventFilter(this);
		fmForm->setCore(bt_);
		break;
	}
	case SoundSource::SSG:
	{
		auto ssgForm = qobject_cast<InstrumentEditorSSGForm*>(form.get());
		break;
	}
	}
}

void MainWindow::on_instrumentListWidget_itemSelectionChanged()
{
	int num = (ui->instrumentListWidget->currentRow() == -1)
			  ? -1
			  : ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	bt_->setCurrentInstrument(num);
}

void MainWindow::onInstrumentFMEnvelopeChanged(int envNum, int fromInstNum)
{
	for (auto& pair : instFormMap_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("SoundSource").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->checkEnvelopeChange(envNum);
		}
	}

	setModifiedTrue();
}

void MainWindow::on_modSetDialogOpenToolButton_clicked()
{
	ModuleSettingsDialog dialog(bt_);
	if (dialog.exec() == QDialog::Accepted) {
		loadModule();
		setModifiedTrue();
		setWindowTitle();
	}
}
