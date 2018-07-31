#include "mainwindow.hpp"
#include <QString>
#include <QLineEdit>
#include <QClipboard>
#include "ui_mainwindow.h"
#include "jam_manager.hpp"
#include "channel_attribute.hpp"
#include "instrument.hpp"
#include "./command/commands_qt.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_psg_form.hpp"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	bt_(std::make_shared<BambooTracker>()),
	comStack_(std::make_unique<QUndoStack>(this))
{
	ui->setupUi(this);

	QApplication::clipboard()->clear();

	// Audio stream
	stream_ = std::make_unique<AudioStream>(bt_->getStreamRate(),
											bt_->getStreamDuration(),
											bt_->getStreamInterruptRate());
	QObject::connect(stream_.get(), &AudioStream::streamInterrupted,
					 this, [&]() { bt_->streamCountUp(); }, Qt::DirectConnection);
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

	return false;
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

void MainWindow::closeEvent(QCloseEvent *ce)
{
	for (auto& pair : instFormMap_) {
		pair.second->close();
	}
	ce->accept();
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
	comStack_->push(new AddInstrumentQtCommand(list, num, name, ch.getSoundSource(), instFormMap_));
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
		if (!form->property("Shown").toBool()) {	// Set data before first show
			switch (static_cast<SoundSource>(form->property("SoundSource").toInt())) {
			case SoundSource::FM:
			{
				auto&& fmForm = qobject_cast<InstrumentEditorFMForm*>(form.get());
				QObject::connect(fmForm, &InstrumentEditorFMForm::instrumentFMEnvelopeParameterChanged,
								 this, &MainWindow::onInstrumentFMEnvelopeChanged);
				QObject::connect(fmForm, &InstrumentEditorFMForm::jamKeyOnEvent,
								 this, &MainWindow::keyPressEvent, Qt::DirectConnection);
				QObject::connect(fmForm, &InstrumentEditorFMForm::jamKeyOffEvent,
								 this, &MainWindow::keyReleaseEvent, Qt::DirectConnection);
				fmForm->installEventFilter(this);
				fmForm->setCore(bt_);
				break;
			}
			case SoundSource::PSG:
			{
				auto&& psgForm = qobject_cast<InstrumentEditorPSGForm*>(form.get());
				break;
			}
			}
			form->setProperty("Shown", true);
		}

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

void MainWindow::copyInstrument()
{
    auto&& inst = bt_->getInstrument(ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt());
    QString tag = "";
    switch (inst->getSoundSource()) {
    case SoundSource::FM:   tag = "FM_INSTRUMENT:";     break;
    case SoundSource::PSG:  tag = "PSG_INSTRUMENT:";    break;
    }
    QApplication::clipboard()->setText(tag + QString::number(inst->getNumber()));
}

void MainWindow::pasteInstrument()
{
    QString newStr = QApplication::clipboard()->text();
    int row = ui->instrumentListWidget->currentRow();
    int num = ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
    auto&& inst = bt_->getInstrument(num);
    auto oldStr = QString::fromUtf8(inst->toString().c_str(), inst->toString().length());

    QRegularExpression re("^.+:(.+)$", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(str);
    if (str.startsWith("FM")) {
        bt_->pasteInstrument(ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt(),
                             match.captured(1).toUtf8().toStdString());
    }
    else if (str.startsWith("PSG")) {

    }
}

void MainWindow::cloneInstrument()
{
    // UNDONE
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
}

void MainWindow::stopPlaySong()
{
	bt_->stopPlaySong();
}

/******************************/
/********** Instrument list events **********/
void MainWindow::on_instrumentListWidget_customContextMenuRequested(const QPoint &pos)
{
	auto& list = ui->instrumentListWidget;
	QPoint globalPos = list->mapToGlobal(pos);
	QMenu menu;

    menu.addAction("Add", this, &MainWindow::addInstrument);
    menu.addAction("Remove", this, &MainWindow::removeInstrument);
    menu.addSeparator();
    menu.addAction("Edit name", this, &MainWindow::editInstrumentName);
    menu.addSeparator();
    menu.addAction("Copy", this, &MainWindow::copyInstrument);
    menu.addAction("Paste", this, &MainWindow::pasteInstrument);
    menu.addAction("Clone", this, &MainWindow::cloneInstrument);
    menu.addSeparator();
    menu.addAction("Load from file...")->setEnabled(false);
    menu.addAction("Save to file...")->setEnabled(false);
    menu.addSeparator();
    menu.addAction("Edit...", this, &MainWindow::editInstrument);

    if (list->count() == 128)    // Max size
        menu.actions().at(0)->setEnabled(false);    // "Add"
    if (list->currentItem() == nullptr) {    // Not selected
        menu.actions().at(1)->setEnabled(false);    // "Remove"
        menu.actions().at(3)->setEnabled(false);    // "Edit name"
        menu.actions().at(5)->setEnabled(false);    // "Copy"
        menu.actions().at(12)->setEnabled(false);   // "Edit"
    }
    if (QApplication::clipboard()->text().startsWith("FM_INSTRUMENT:")) {
        auto item = ui->instrumentListWidget->currentItem();
        if (item == nullptr
                || bt_->getInstrument(item->data(Qt::UserRole).toInt())->getSoundSource() != SoundSource::FM) {
            menu.actions().at(6)->setEnabled(false);    // "Paste"
            menu.actions().at(7)->setEnabled(false);    // "Clone"
        }
    }
    else if (QApplication::clipboard()->text().startsWith("PSG_INSTRUMENT:")) {
        auto item = ui->instrumentListWidget->currentItem();
        if (item == nullptr
                || bt_->getInstrument(item->data(Qt::UserRole).toInt())->getSoundSource() != SoundSource::PSG) {
            menu.actions().at(6)->setEnabled(false);    // "Paste"
            menu.actions().at(7)->setEnabled(false);    // "Clone"
        }
    }
    else {
        menu.actions().at(6)->setEnabled(false);    // "Paste"
        menu.actions().at(7)->setEnabled(false);    // "Clone"
    }

	menu.exec(globalPos);
}

void MainWindow::on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item)
{
	Q_UNUSED(item)
	editInstrument();
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
}
