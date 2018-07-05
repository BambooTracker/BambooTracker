#include "mainwindow.hpp"
#include <QString>
#include "ui_mainwindow.h"
#include "jam_manager.hpp"
#include "channel_attribute.hpp"
#include "./command/commands_qt.hpp"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	bt_(std::make_unique<BambooTracker>()),
	comStack_(std::make_unique<QUndoStack>(this)),
	isPlaySong_(false)
{
	ui->setupUi(this);

	// Audio stream
	stream_ = std::make_unique<AudioStream>(bt_->getStreamRate(), bt_->getStreamDuration());
	QObject::connect(stream_.get(), &AudioStream::nextStepArrived,
					 this, [&]() { bt_->readStep(); }, Qt::DirectConnection);
	QObject::connect(stream_.get(), &AudioStream::nextTickArrived,
					 this, [&]() { bt_->readTick(); }, Qt::DirectConnection);
	QObject::connect(stream_.get(), &AudioStream::bufferPrepared,
					 this, [&](int16_t *container, size_t nSamples) {
		bt_->getStreamSamples(container, nSamples);
	}, Qt::DirectConnection);

	// Instrument list
	auto& vl = ui->instrumentListWidget;
	vl->setContextMenuPolicy(Qt::CustomContextMenu);
	vl->setSelectionMode(QAbstractItemView::SingleSelection);
	vl->setFocusPolicy(Qt::NoFocus);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();

	if (event->modifiers().testFlag(Qt::ControlModifier)) {	// Pressed ctrl
		switch (key) {
		case Qt::Key_Z:	undo();	break;
		case Qt::Key_Y:	redo();	break;
		default:	break;
		}
	}
	else {
		switch (key) {
		// Common keys
		case Qt::Key_Space:		bt_->toggleJamMode();	break;
		case Qt::Key_Asterisk:	bt_->raiseOctave();		break;
		case Qt::Key_Slash:		bt_->lowerOctave();		break;
		case Qt::Key_F5:		startPlaySong();		break;
		case Qt::Key_F8:		stopPlaySong();			break;

	//********** dummy
		case Qt::Key_Left: if (channel_ != 0) bt_->selectChannel(--channel_); break;
		case Qt::Key_Right: if (channel_ != 8) bt_->selectChannel(++channel_); break;
	//***************

		default:
			if (bt_->isJamMode() && !event->isAutoRepeat()) {
				// Jam mode keys
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
			else {
				// Edit mode keys
				switch (key) {
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

	if (bt_->isJamMode() && !event->isAutoRepeat()) {
		// Jam mode keys
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


/********** Instrument list **********/
void MainWindow::addInstrument()
{
	auto& list = ui->instrumentListWidget;

	// Find free number
	int num = 0;
	for (int i = 0; i < list->count(); ++i, ++num) {
		if (list->item(i)->data(Qt::UserRole).toInt() != num) break;
	}

	QString name = "Instrument " + QString::number(num);
	bt_->addInstrument(num, name.toUtf8().toStdString());

	ChannelAttribute ch = bt_->getCurrentChannel();
	QString typestr;
	switch (ch.getSoundSource()) {
	case SoundSource::FM:	typestr = "[FM]";	break;
	case SoundSource::PSG:	typestr = "[PSG]";	break;
	}
	comStack_->push(new AddInstrumentQtCommand(list, num, typestr+name));
}

void MainWindow::removeInstrument()
{
	auto& list = ui->instrumentListWidget;
	int row = list->currentRow();
	int num = list->item(row)->data(Qt::UserRole).toInt();

	bt_->removeInstrument(num);
	comStack_->push(new RemoveInstrumentQtCommand(list, num, row));
}

void MainWindow::editInstrument()
{
	auto& list = ui->instrumentListWidget;
	qDebug() << list->currentItem()->data(Qt::UserRole);
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

/********** Play song **********/
void MainWindow::startPlaySong()
{
	bt_->startPlaySong();
	isPlaySong_ = stream_->startPlaySong();
}

void MainWindow::stopPlaySong()
{
	bt_->stopPlaySong();
	isPlaySong_ = stream_->stopPlaySong();
}

/******************************/
/********** Instrument list events **********/
void MainWindow::on_instrumentListWidget_customContextMenuRequested(const QPoint &pos)
{
	auto& list = ui->instrumentListWidget;
	QPoint globalPos = list->mapToGlobal(pos);
	QMenu menu;
	menu.addAction("Add instrument", this, &MainWindow::addInstrument);
	menu.addAction("Remove instrument", this, &MainWindow::removeInstrument);
	if (list->currentItem() == nullptr) menu.actions().at(1)->setEnabled(false);
	menu.addSeparator();
	menu.addAction("Load from file...")->setEnabled(false);
	menu.addAction("Save to file...")->setEnabled(false);
	menu.addSeparator();
	menu.addAction("Edit...", this, &MainWindow::editInstrument);
	if (list->currentItem() == nullptr) menu.actions().at(6)->setEnabled(false);

	menu.exec(globalPos);
}

void MainWindow::on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item)
{
	Q_UNUSED(item)
	editInstrument();
}
