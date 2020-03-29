#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <algorithm>
#include <unordered_map>
#include <QString>
#include <QLineEdit>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QRegularExpression>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMimeData>
#include <QProgressDialog>
#include <QRect>
#include <QDesktopWidget>
#include <QAudioDeviceInfo>
#include <QMetaMethod>
#include <QScreen>
#include <QComboBox>
#include "jam_manager.hpp"
#include "song.hpp"
#include "track.hpp"
#include "instrument.hpp"
#include "bank.hpp"
#include "bank_io.hpp"
#include "file_io.hpp"
#include "version.hpp"
#include "gui/command/commands_qt.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "gui/instrument_editor/instrument_editor_adpcm_form.hpp"
#include "gui/module_properties_dialog.hpp"
#include "gui/groove_settings_dialog.hpp"
#include "gui/configuration_dialog.hpp"
#include "gui/comment_edit_dialog.hpp"
#include "gui/wave_export_settings_dialog.hpp"
#include "gui/vgm_export_settings_dialog.hpp"
#include "gui/instrument_selection_dialog.hpp"
#include "gui/s98_export_settings_dialog.hpp"
#include "gui/configuration_handler.hpp"
#include "gui/jam_layout.hpp"
#include "chips/scci/SCCIDefines.h"
#include "chips/c86ctl/c86ctl_wrapper.hpp"
#include "gui/file_history_handler.hpp"
#include "midi/midi.hpp"
#include "audio_stream_rtaudio.hpp"
#include "color_palette_handler.hpp"
#include "binary_container.hpp"
#include "wav_container.hpp"
#include "gui/shortcut_util.hpp"
#include "enum_hash.hpp"
#include "gui/go_to_dialog.hpp"

MainWindow::MainWindow(std::weak_ptr<Configuration> config, QString filePath, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	config_(config),
	palette_(std::make_shared<ColorPalette>()),
	bt_(std::make_shared<BambooTracker>(config)),
	comStack_(std::make_shared<QUndoStack>(this)),
	fileHistory_(std::make_shared<FileHistory>()),
	scciDll_(std::make_unique<QLibrary>("scci")),
	c86ctlDll_(std::make_unique<QLibrary>("c86ctl")),
	instForms_(std::make_shared<InstrumentFormManager>()),
	isModifiedForNotCommand_(false),
	hasLockedWigets_(false),
	isEditedPattern_(true),
	isEditedOrder_(false),
	isEditedInstList_(false),
	isSelectedPattern_(false),
	isSelectedOrder_(false),
	hasShownOnce_(false),
	firstViewUpdateRequest_(false),
	effListDiag_(std::make_unique<EffectListDialog>()),
	shortcutsDiag_(std::make_unique<KeyboardShortcutListDialog>())
{
	ui->setupUi(this);

	if (config.lock()->getMainWindowX() == -1) {	// When unset
		QRect rec = geometry();
		rec.moveCenter(QGuiApplication::screens().front()->geometry().center());
		setGeometry(rec);
		config.lock()->setMainWindowX(x());
		config.lock()->setMainWindowY(y());
	}
	else {
		move(config.lock()->getMainWindowX(), config.lock()->getMainWindowY());
	}
	resize(config.lock()->getMainWindowWidth(), config.lock()->getMainWindowHeight());
	if (config.lock()->getMainWindowMaximized()) showMaximized();
	ui->action_Status_Bar->setChecked(config.lock()->getVisibleStatusBar());
	ui->statusBar->setVisible(config.lock()->getVisibleStatusBar());
	ui->actionFollow_Mode->setChecked(config.lock()->getFollowMode());
	ui->action_Instrument_Mask->setChecked(config.lock()->getInstrumentMask());
	ui->action_Volume_Mask->setChecked(config.lock()->getVolumeMask());
	ui->waveVisual->setVisible(config_.lock()->getShowWaveVisual());
	bt_->setFollowPlay(config.lock()->getFollowMode());
	if (config.lock()->getPatternEditorHeaderFont().empty()) {
		config.lock()->setPatternEditorHeaderFont(ui->patternEditor->getHeaderFont().toStdString());
	}
	if (config.lock()->getPatternEditorRowsFont().empty()) {
		config.lock()->setPatternEditorRowsFont(ui->patternEditor->getRowsFont().toStdString());
	}
	if (config.lock()->getOrderListHeaderFont().empty()) {
		config.lock()->setOrderListHeaderFont(ui->orderList->getHeaderFont().toStdString());
	}
	if (config.lock()->getOrderListRowsFont().empty()) {
		config.lock()->setOrderListRowsFont(ui->orderList->getRowsFont().toStdString());
	}
	ui->patternEditor->setConfiguration(config_.lock());
	ui->orderList->setConfiguration(config_.lock());
	updateFonts();
	ui->orderList->setHorizontalScrollMode(config.lock()->getMoveCursorByHorizontalScroll(), false);
	ui->patternEditor->setHorizontalScrollMode(config.lock()->getMoveCursorByHorizontalScroll(), false);
	ui->patternEditor->setCore(bt_);
	ui->orderList->setCore(bt_);
	ColorPaletteHandler::loadPalette(palette_);
	ui->patternEditor->setColorPallete(palette_);
	ui->orderList->setColorPallete(palette_);
	updateInstrumentListColors();
	ui->waveVisual->setColorPalette(palette_);
	setMidiConfiguration();

	/* Command stack */
	QObject::connect(comStack_.get(), &QUndoStack::indexChanged,
					 this, [&](int idx) {
		setWindowModified(idx || isModifiedForNotCommand_);
		ui->actionUndo->setEnabled(comStack_->canUndo());
		ui->actionRedo->setEnabled(comStack_->canRedo());
	});

	/* File history */
	FileHistoryHandler::loadFileHistory(fileHistory_);
	for (size_t i = 0; i < fileHistory_->size(); ++i) {
		// Leave Before Qt5.7.0 style due to windows xp
		QAction* action = ui->menu_Recent_Files->addAction(QString("&%1 %2").arg(i + 1).arg(fileHistory_->at(i)));
		action->setData(fileHistory_->at(i));
	}
	QObject::connect(ui->menu_Recent_Files, &QMenu::triggered, this, [&](QAction* action) {
		if (action != ui->actionClear) {
			if (isWindowModified()) {
				auto modTitleStd = bt_->getModuleTitle();
				QString modTitle = QString::fromUtf8(modTitleStd.c_str(), static_cast<int>(modTitleStd.length()));
				if (modTitle.isEmpty()) modTitle = tr("Untitled");
				QMessageBox dialog(QMessageBox::Warning,
								   "BambooTracker",
								   tr("Save changes to %1?").arg(modTitle),
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
			openModule(action->data().toString());
		}
	});

	/* Tool bar */
	auto octLab = new QLabel(tr("Octave"));
	octLab->setMargin(6);
	ui->subToolBar->addWidget(octLab);
	octave_ = new QSpinBox();
	octave_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	octave_->setMaximumWidth(80);
	octave_->setMinimum(0);
	octave_->setMaximum(7);
	octave_->setValue(bt_->getCurrentOctave());
	auto octFunc = [&](int octave) { bt_->setCurrentOctave(octave); };
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(octave_, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, octFunc);
	ui->subToolBar->addWidget(octave_);
	ui->subToolBar->addSeparator();
	ui->subToolBar->addAction(ui->actionFollow_Mode);
	ui->subToolBar->addSeparator();
	auto hlLab1 = new QLabel(tr("Step highlight 1st"));
	hlLab1->setMargin(6);
	ui->subToolBar->addWidget(hlLab1);
	highlight1_ = new QSpinBox();
	highlight1_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	highlight1_->setMaximumWidth(80);
	highlight1_->setMinimum(1);
	highlight1_->setMaximum(256);
	highlight1_->setValue(8);
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(highlight1_, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int count) {
		bt_->setModuleStepHighlight1Distance(static_cast<size_t>(count));
		ui->patternEditor->setPatternHighlight1Count(count);
	});
	ui->subToolBar->addWidget(highlight1_);
	auto hlLab2 = new QLabel(tr("2nd"));
	hlLab2->setMargin(6);
	ui->subToolBar->addWidget(hlLab2);
	highlight2_ = new QSpinBox();
	highlight2_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	highlight2_->setMaximumWidth(80);
	highlight2_->setMinimum(1);
	highlight2_->setMaximum(256);
	highlight2_->setValue(8);
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(highlight2_, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int count) {
		bt_->setModuleStepHighlight2Distance(static_cast<size_t>(count));
		ui->patternEditor->setPatternHighlight2Count(count);
	});
	ui->subToolBar->addWidget(highlight2_);
	auto& mainTbConfig = config.lock()->getMainToolbarConfiguration();
	if (mainTbConfig.getPosition() == Configuration::ToolbarConfiguration::FLOAT_POS) {
		ui->mainToolBar->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
		ui->mainToolBar->move(mainTbConfig.getX(), mainTbConfig.getY());
	}
	else {
		addToolBar(TB_POS_.at(mainTbConfig.getPosition()), ui->mainToolBar);
	}
	auto& subTbConfig = config.lock()->getSubToolbarConfiguration();
	if (subTbConfig.getPosition() == Configuration::ToolbarConfiguration::FLOAT_POS) {
		ui->subToolBar->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
		ui->subToolBar->move(subTbConfig.getX(), subTbConfig.getY());
	}
	else {
		auto pos = TB_POS_.at(subTbConfig.getPosition());
		if (subTbConfig.getNumber()) {
			if (subTbConfig.hasBreakBefore()) addToolBarBreak(pos);
			addToolBar(pos, ui->subToolBar);
		}
		else {
			if (mainTbConfig.getPosition() == subTbConfig.getPosition()) {
				insertToolBar(ui->mainToolBar, ui->subToolBar);
				if (mainTbConfig.hasBreakBefore()) insertToolBarBreak(ui->mainToolBar);
			}
			else {
				addToolBar(pos, ui->subToolBar);
			}
		}
	}
	ui->action_Toolbar->setChecked(config.lock()->getVisibleToolbar());
	ui->mainToolBar->setVisible(config.lock()->getVisibleToolbar());
	ui->subToolBar->setVisible(config.lock()->getVisibleToolbar());

	/* Splitter */
	ui->splitter->setStretchFactor(0, 0);
	ui->splitter->setStretchFactor(1, 1);

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

	/* Edit settings */
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->editableStepSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int n) {
		ui->patternEditor->setEditableStep(n);
		config_.lock()->setEditableStep(static_cast<size_t>(n));
	});
	ui->editableStepSpinBox->setValue(static_cast<int>(config.lock()->getEditableStep()));
	ui->patternEditor->setEditableStep(static_cast<int>(config.lock()->getEditableStep()));

	ui->keyRepeatCheckBox->setCheckState(config.lock()->getKeyRepetition() ? Qt::Checked : Qt::Unchecked);

	/* Song */
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->songComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, [&](int num) {
		if (num == -1) return;
		freezeViews();
		if (!timer_) stream_->stop();
		bt_->setCurrentSongNumber(num);
		loadSong();
		if (!timer_) stream_->start();
	});

	/* Song settings */
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->tempoSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int tempo) {
		int curSong = bt_->getCurrentSongNumber();
		if (tempo != bt_->getSongTempo(curSong)) {
			bt_->setSongTempo(curSong, tempo);
			setModifiedTrue();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->speedSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int speed) {
		int curSong = bt_->getCurrentSongNumber();
		if (speed != bt_->getSongSpeed(curSong)) {
			bt_->setSongSpeed(curSong, speed);
			setModifiedTrue();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->patternSizeSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int size) {
		bt_->setDefaultPatternSize(bt_->getCurrentSongNumber(), static_cast<size_t>(size));
		ui->patternEditor->onDefaultPatternSizeChanged();
		setModifiedTrue();
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->grooveSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int n) {
		bt_->setSongGroove(bt_->getCurrentSongNumber(), n);
		setModifiedTrue();
	});

	/* Instrument list */
	ui->instrumentListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	auto instToolBar = new QToolBar();
	instToolBar->setIconSize(QSize(16, 16));
	instToolBar->addAction(ui->actionNew_Instrument);
	instToolBar->addAction(ui->actionRemove_Instrument);
	instToolBar->addAction(ui->actionClone_Instrument);
	instToolBar->addSeparator();
	instToolBar->addAction(ui->actionLoad_From_File);
	instToolBar->addAction(ui->actionSave_To_File);
	instToolBar->addSeparator();
	instToolBar->addAction(ui->actionEdit);
	instToolBar->addSeparator();
	instToolBar->addAction(ui->actionRename_Instrument);
	ui->instrumentListGroupBox->layout()->addWidget(instToolBar);
	ui->instrumentListWidget->installEventFilter(this);

	/* Pattern editor */
	ui->patternEditor->setCommandStack(comStack_);
	ui->patternEditor->installEventFilter(this);
	QObject::connect(ui->patternEditor, &PatternEditor::currentTrackChanged,
					 ui->orderList, &OrderListEditor::setCurrentTrack);
	QObject::connect(ui->patternEditor, &PatternEditor::currentOrderChanged,
					 ui->orderList, &OrderListEditor::setCurrentOrder);
	QObject::connect(ui->patternEditor, &PatternEditor::focusIn,
					 this, &MainWindow::updateMenuByPattern);
	QObject::connect(ui->patternEditor, &PatternEditor::selected,
					 this, &MainWindow::updateMenuByPatternSelection);
	QObject::connect(ui->patternEditor, &PatternEditor::instrumentEntered,
					 this, [&](int num) {
		auto list = ui->instrumentListWidget;
		if (num != -1) {
			for (int i = 0; i < list->count(); ++i) {
				if (list->item(i)->data(Qt::UserRole).toInt() == num) {
					list->setCurrentRow(i);
					return ;
				}
			}
		}
	});
	QObject::connect(ui->patternEditor, &PatternEditor::effectEntered,
					 this, [&](QString text) { statusDetail_->setText(text); });
	QObject::connect(ui->patternEditor, &PatternEditor::currentTrackChanged,
					 this, &MainWindow::onCurrentTrackChanged);

	/* Order List */
	ui->orderList->setCommandStack(comStack_);
	ui->orderList->installEventFilter(this);
	QObject::connect(ui->orderList, &OrderListEditor::currentTrackChanged,
					 ui->patternEditor, &PatternEditor::setCurrentTrack);
	QObject::connect(ui->orderList, &OrderListEditor::currentOrderChanged,
					 ui->patternEditor, &PatternEditor::setCurrentOrder);
	QObject::connect(ui->orderList, &OrderListEditor::orderEdited,
					 ui->patternEditor, &PatternEditor::onOrderListEdited);
	QObject::connect(ui->orderList, &OrderListEditor::focusIn,
					 this, &MainWindow::updateMenuByOrder);
	QObject::connect(ui->orderList, &OrderListEditor::selected,
					 this, &MainWindow::updateMenuByOrderSelection);
	QObject::connect(ui->orderList, &OrderListEditor::currentTrackChanged,
					 this, &MainWindow::onCurrentTrackChanged);

	/* Visuals */
	visualTimer_.reset(new QTimer);
	visualTimer_->start(40);
	QObject::connect(visualTimer_.get(), &QTimer::timeout,
					 this, &MainWindow::updateVisuals);

	/* Status bar */
	statusDetail_ = new QLabel();
	statusDetail_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	statusStyle_ = new QLabel();
	statusStyle_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	statusInst_ = new QLabel();
	statusInst_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	statusOctave_ = new QLabel();
	statusOctave_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	statusIntr_ = new QLabel();
	statusIntr_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	statusMixer_ = new QLabel();
	statusMixer_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	statusBpm_ = new QLabel();
	statusBpm_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	statusPlayPos_ = new QLabel();
	statusPlayPos_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	ui->statusBar->addWidget(statusDetail_, 4);
	ui->statusBar->addPermanentWidget(statusStyle_, 1);
	ui->statusBar->addPermanentWidget(statusInst_, 1);
	ui->statusBar->addPermanentWidget(statusOctave_, 1);
	ui->statusBar->addPermanentWidget(statusIntr_, 1);
	ui->statusBar->addPermanentWidget(statusMixer_, 1);
	ui->statusBar->addPermanentWidget(statusBpm_, 1);
	ui->statusBar->addPermanentWidget(statusPlayPos_, 1);
	statusOctave_->setText(tr("Octave: %1").arg(bt_->getCurrentOctave()));
	statusIntr_->setText(QString::number(bt_->getModuleTickFrequency()) + QString("Hz"));

	/* Bookmark */
	bmManForm_ = std::make_unique<BookmarkManagerForm>(bt_, config_.lock()->getShowRowNumberInHex());
	QObject::connect(bmManForm_.get(), &BookmarkManagerForm::positionJumpRequested,
					 this, [&](int order, int step) {
		if (bt_->isPlaySong()) return;
		int song = bt_->getCurrentSongNumber();
		if (static_cast<int>(bt_->getOrderSize(song)) <= order) return;
		if (static_cast<int>(bt_->getPatternSizeFromOrderNumber(song, order)) <= step) return;
		bt_->setCurrentOrderNumber(order);
		bt_->setCurrentStepNumber(step);
		ui->orderList->updatePositionByPositionJump();
		ui->patternEditor->updatepositionByPositionJump();
		activateWindow();
	});
	QObject::connect(bmManForm_.get(), &BookmarkManagerForm::modified, this, &MainWindow::setModifiedTrue);

	// Shortcuts
	octUpSc_ = std::make_unique<QShortcut>(this);
	octUpSc_->setContext(Qt::ApplicationShortcut);
	QObject::connect(octUpSc_.get(), &QShortcut::activated, this, [&] { changeOctave(true); });
	octDownSc_ = std::make_unique<QShortcut>(this);
	octDownSc_->setContext(Qt::ApplicationShortcut);
	QObject::connect(octDownSc_.get(), &QShortcut::activated, this, [&] { changeOctave(false); });
	focusPtnSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(focusPtnSc_.get(), &QShortcut::activated, this, [&] { ui->patternEditor->setFocus(); });
	focusOdrSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(focusOdrSc_.get(), &QShortcut::activated, this, [&] { ui->orderList->setFocus(); });
	focusInstSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(focusInstSc_.get(), &QShortcut::activated, this, [&] {
		ui->instrumentListWidget->setFocus();
		updateMenuByInstrumentList();
	});
	playAndStopSc_ = std::make_unique<QShortcut>(this);
	playAndStopSc_->setContext(Qt::ApplicationShortcut);
	QObject::connect(playAndStopSc_.get(), &QShortcut::activated, this, [&] {
		if (bt_->isPlaySong()) stopPlaySong();
		else startPlaySong();
	});
	playStepSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(playStepSc_.get(), &QShortcut::activated, this, &MainWindow::playStep);
	ui->actionPlay_From_Start->setShortcutContext(Qt::ApplicationShortcut);
	ui->actionPlay_Pattern->setShortcutContext(Qt::ApplicationShortcut);
	ui->actionPlay_From_Cursor->setShortcutContext(Qt::ApplicationShortcut);
	ui->actionPlay_From_Marker->setShortcutContext(Qt::ApplicationShortcut);
	ui->actionStop->setShortcutContext(Qt::ApplicationShortcut);
	instAddSc_ = std::make_unique<QShortcut>(Qt::Key_Insert, ui->instrumentListWidget,
											 nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(instAddSc_.get(), &QShortcut::activated, this, &MainWindow::addInstrument);
	goPrevOdrSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(goPrevOdrSc_.get(), &QShortcut::activated, this, [&] {
		ui->orderList->onGoOrderRequested(false);
	});
	goNextOdrSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(goNextOdrSc_.get(), &QShortcut::activated, this, [&] {
		ui->orderList->onGoOrderRequested(true);
	});
	prevInstSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(prevInstSc_.get(), &QShortcut::activated, this, [&] {
		if (ui->instrumentListWidget->count()) {
			int row = ui->instrumentListWidget->currentRow();
			if (row == -1) ui->instrumentListWidget->setCurrentRow(0);
			else if (row > 0) ui->instrumentListWidget->setCurrentRow(row - 1);
		}
	});
	nextInstSc_ = std::make_unique<QShortcut>(this);
	QObject::connect(nextInstSc_.get(), &QShortcut::activated, this, [&] {
		int cnt = ui->instrumentListWidget->count();
		if (cnt) {
			int row = ui->instrumentListWidget->currentRow();
			if (row == -1) ui->instrumentListWidget->setCurrentRow(cnt - 1);
			else if (row < cnt - 1) ui->instrumentListWidget->setCurrentRow(row + 1);
		}
	});
	setShortcuts();

	/* Clipboard */
	QObject::connect(QApplication::clipboard(), &QClipboard::dataChanged,
					 this, [&]() {
		if (isEditedOrder_) updateMenuByOrder();
		else if (isEditedPattern_) updateMenuByPattern();
	});

	/* MIDI */
	midiKeyEventMethod_ = metaObject()->indexOfSlot("midiKeyEvent(uchar,uchar,uchar)");
	Q_ASSERT(midiKeyEventMethod_ != -1);
	midiProgramEventMethod_ = metaObject()->indexOfSlot("midiProgramEvent(uchar,uchar)");
	Q_ASSERT(midiProgramEventMethod_ != -1);
	MidiInterface::instance().installInputHandler(&midiThreadReceivedEvent, this);

	/* Audio stream */
	bool savedDeviceExists = false;
	for (QAudioDeviceInfo audioDevice : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
		if (audioDevice.deviceName().toUtf8().toStdString() == config.lock()->getSoundDevice()) {
			savedDeviceExists = true;
			break;
		}
	}
	if (!savedDeviceExists) {
		QString sndDev = QAudioDeviceInfo::defaultOutputDevice().deviceName();
		config.lock()->setSoundDevice(sndDev.toUtf8().toStdString());
	}
	stream_ = std::make_shared<AudioStreamRtAudio>();
	stream_->setTickUpdateCallback(+[](void* cbPtr) -> int {
		auto bt = reinterpret_cast<BambooTracker*>(cbPtr);
		return bt->streamCountUp();
	}, bt_.get());
	stream_->setGenerateCallback(+[](int16_t* container, size_t nSamples, void* cbPtr) {
		auto bt = reinterpret_cast<BambooTracker*>(cbPtr);
		bt->getStreamSamples(container, nSamples);
	}, bt_.get());
	QObject::connect(stream_.get(), &AudioStream::streamInterrupted, this, &MainWindow::onNewTickSignaled);
	bool streamState = stream_->initialize(
						   static_cast<uint32_t>(bt_->getStreamRate()),
						   static_cast<uint32_t>(bt_->getStreamDuration()),
						   bt_->getModuleTickFrequency(),
						   QString::fromUtf8(config.lock()->getSoundAPI().c_str(),
											 static_cast<int>(config.lock()->getSoundAPI().length())),
						   QString::fromUtf8(config.lock()->getSoundDevice().c_str(),
											 static_cast<int>(config.lock()->getSoundDevice().length())));
	if (!streamState) showStreamFailedDialog();
	RealChipInterface intf = config.lock()->getRealChipInterface();
	if (intf == RealChipInterface::NONE) {
		bt_->useSCCI(nullptr);
		bt_->useC86CTL(nullptr);
	}
	else {
		timer_ = std::make_unique<Timer>();
		timer_->setInterval(1000000 / bt_->getModuleTickFrequency());
		tickEventMethod_ = metaObject()->indexOfSlot("onNewTickSignaledRealChip()");
		Q_ASSERT(tickEventMethod_ != -1);
		timer_->setFunction([&]{
			QMetaMethod method = this->metaObject()->method(this->tickEventMethod_);
			method.invoke(this, Qt::QueuedConnection);
		});

		setRealChipInterface(intf);

		timer_->start();
	}

	/* Load module */
	if (filePath.isEmpty()) {
		loadModule();
		setInitialSelectedInstrument();
		assignADPCMSamples();
		if (!timer_) stream_->start();
	}
	else {
		openModule(filePath);	// If use emulation, stream stars
	}
}

MainWindow::~MainWindow()
{
	MidiInterface::instance().uninstallInputHandler(&midiThreadReceivedEvent, this);
	stream_->shutdown();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (auto fmForm = qobject_cast<InstrumentEditorFMForm*>(watched)) {
		// Change current instrument by activating FM editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(fmForm->getInstrumentNumber());
			ui->instrumentListWidget->setCurrentRow(row);
			return false;
		}
		else if (event->type() == QEvent::Resize) {
			config_.lock()->setInstrumentFMWindowWidth(fmForm->width());
			config_.lock()->setInstrumentFMWindowHeight(fmForm->height());
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
			config_.lock()->setInstrumentSSGWindowWidth(ssgForm->width());
			config_.lock()->setInstrumentSSGWindowHeight(ssgForm->height());
			return false;
		}
	}

	if (auto adpcmForm = qobject_cast<InstrumentEditorADPCMForm*>(watched)) {
		// Change current instrument by activating ADPCM editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(adpcmForm->getInstrumentNumber());
			ui->instrumentListWidget->setCurrentRow(row);
			return false;
		}
		else if (event->type() == QEvent::Resize) {
			config_.lock()->setInstrumentADPCMWindowWidth(adpcmForm->width());
			config_.lock()->setInstrumentADPCMWindowHeight(adpcmForm->height());
			return false;
		}
	}

	if (watched == ui->instrumentListWidget) {
		switch (event->type()) {
		case QEvent::FocusIn:
			updateMenuByInstrumentList();
			break;
		default:
			break;
		}
	}

	return false;
}

void MainWindow::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)

	if (!hasShownOnce_)	{
		int y = config_.lock()->getMainWindowVerticalSplit();
		if (y == -1) {
			config_.lock()->setMainWindowVerticalSplit(ui->splitter->sizes().front());
		}
		else {
			ui->splitter->setSizes({ y, ui->splitter->height() - ui->splitter->handleWidth() - y });
		}
		hasShownOnce_ = true;
	}
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if (!event->isAutoRepeat()) {
		// Musical keyboard
		Qt::Key qtKey = static_cast<Qt::Key>(event->key());
		try {
			bt_->jamKeyOn(getJamKeyFromLayoutMapping(qtKey, config_));
		} catch (std::invalid_argument&) {}
	}
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	int key = event->key();

	if (!event->isAutoRepeat()) {
		// Musical keyboard
		Qt::Key qtKey = static_cast<Qt::Key>(key);
		try {
			bt_->jamKeyOff(getJamKeyFromLayoutMapping(qtKey, config_));
		} catch (std::invalid_argument&) {}
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	auto mime = event->mimeData();
	if (mime->hasUrls() && mime->urls().length() == 1) {
		switch (FileIO::judgeFileTypeFromExtension(
					QFileInfo(mime->urls().first().toLocalFile()).suffix().toStdString())) {
		case FileIO::FileType::Mod:
		case FileIO::FileType::Inst:
		case FileIO::FileType::Bank:
			event->acceptProposedAction();
			break;
		default:
			break;
		}
	}
}

void MainWindow::dropEvent(QDropEvent* event)
{
	QString file = event->mimeData()->urls().first().toLocalFile();

	switch (FileIO::judgeFileTypeFromExtension(QFileInfo(file).suffix().toStdString())) {
	case FileIO::FileType::Mod:
	{
		if (isWindowModified()) {
			auto modTitleStd = bt_->getModuleTitle();
			QString modTitle = QString::fromUtf8(modTitleStd.c_str(), static_cast<int>(modTitleStd.length()));
			if (modTitle.isEmpty()) modTitle = tr("Untitled");
			QMessageBox dialog(QMessageBox::Warning,
							   "BambooTracker",
							   tr("Save changes to %1?").arg(modTitle),
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
		lockWidgets(false);

		openModule(file);
		break;
	}
	case FileIO::FileType::Inst:
	{
		funcLoadInstrument(file);
		break;
	}
	case FileIO::FileType::Bank:
	{
		funcImportInstrumentsFromBank(file);
		break;
	}
	default:
		break;
	}
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	if (!isMaximized()) {	// Check previous size
		config_.lock()->setMainWindowWidth(event->oldSize().width());
		config_.lock()->setMainWindowHeight(event->oldSize().height());
	}
}

void MainWindow::moveEvent(QMoveEvent* event)
{
	QWidget::moveEvent(event);

	if (!isMaximized()) {	// Check previous position
		config_.lock()->setMainWindowX(event->oldPos().x());
		config_.lock()->setMainWindowY(event->oldPos().y());
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (isWindowModified()) {
		auto modTitleStd = bt_->getModuleTitle();
		QString modTitle = QString::fromUtf8(modTitleStd.c_str(), static_cast<int>(modTitleStd.length()));
		if (modTitle.isEmpty()) modTitle = tr("Untitled");
		QMessageBox dialog(QMessageBox::Warning,
						   "BambooTracker",
						   tr("Save changes to %1?").arg(modTitle),
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
		config_.lock()->setMainWindowMaximized(true);
	}
	else {
		config_.lock()->setMainWindowMaximized(false);
		config_.lock()->setMainWindowWidth(width());
		config_.lock()->setMainWindowHeight(height());
		config_.lock()->setMainWindowX(x());
		config_.lock()->setMainWindowY(y());
	}
	config_.lock()->setMainWindowVerticalSplit(ui->splitter->sizes().front());

	auto& mainTbConfig = config_.lock()->getMainToolbarConfiguration();
	auto& subTbConfig = config_.lock()->getSubToolbarConfiguration();
	auto mainTbArea = toolBarArea(ui->mainToolBar);
	auto subTbArea = toolBarArea(ui->subToolBar);
	if (ui->mainToolBar->isFloating()) {
		mainTbConfig.setPosition(ToolbarPos::FLOAT_POS);
		mainTbConfig.setX(ui->mainToolBar->x());
		mainTbConfig.setY(ui->mainToolBar->y());
	}
	else {
		mainTbConfig.setPosition(std::find_if(TB_POS_.begin(), TB_POS_.end(), [mainTbArea](auto pair) {
			return (pair.second == mainTbArea);
		})->first);
		mainTbConfig.setNumber(0);
		mainTbConfig.setBreakBefore(false);
	}
	if (ui->subToolBar->isFloating()) {
		subTbConfig.setPosition(ToolbarPos::FLOAT_POS);
		subTbConfig.setX(ui->subToolBar->x());
		subTbConfig.setY(ui->subToolBar->y());
	}
	else {
		subTbConfig.setPosition(std::find_if(TB_POS_.begin(), TB_POS_.end(), [subTbArea](auto pair) {
			return (pair.second == subTbArea);
		})->first);
		subTbConfig.setNumber(0);
		subTbConfig.setBreakBefore(false);
	}
	if (mainTbArea == subTbArea) {
		auto mainPos = ui->mainToolBar->pos();
		auto subPos = ui->subToolBar->pos();
		switch (mainTbArea) {
		case Qt::TopToolBarArea:
		{
			if (mainPos.x() == subPos.x()) {
				bool cond = (mainPos.y() < subPos.y());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(!cond);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(cond);
			}
			else {
				bool cond = (mainPos.x() < subPos.x());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(false);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(false);
			}
			break;
		}
		case Qt::BottomToolBarArea:
		{
			if (mainPos.x() == subPos.x()) {
				bool cond = (subPos.y() < mainPos.y());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(!cond);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(cond);
			}
			else {
				bool cond = (mainPos.x() < subPos.x());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(false);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(false);
			}
			break;
		}
		case Qt::LeftToolBarArea:
		{
			if (mainPos.x() == subPos.x()) {
				bool cond = (mainPos.y() < subPos.y());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(false);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(false);
			}
			else {
				bool cond = (mainPos.x() < subPos.x());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(!cond);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(cond);
			}
			break;
		}
		case Qt::RightToolBarArea:
		{
			if (mainPos.x() == subPos.x()) {
				bool cond = (mainPos.y() < subPos.y());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(false);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(false);
			}
			else {
				bool cond = (subPos.x() < mainPos.x());
				mainTbConfig.setNumber(cond ? 0 : 1);
				mainTbConfig.setBreakBefore(!cond);
				subTbConfig.setNumber(cond ? 1 : 0);
				subTbConfig.setBreakBefore(cond);
			}
			break;
		}
		default:
			break;
		}
	}
	config_.lock()->setVisibleToolbar(ui->mainToolBar->isVisible());
	config_.lock()->setVisibleStatusBar(ui->statusBar->isVisible());
	config_.lock()->setFollowMode(bt_->isFollowPlay());

	instForms_->closeAll();

	FileHistoryHandler::saveFileHistory(fileHistory_);

	effListDiag_->close();
	shortcutsDiag_->close();
	bmManForm_->close();

	event->accept();
}

void MainWindow::freezeViews()
{
	ui->orderList->freeze();
	ui->patternEditor->freeze();
}

void MainWindow::setShortcuts()
{
	octUpSc_->setKey(strToKeySeq(config_.lock()->getOctaveUpKey().c_str()));
	octDownSc_->setKey(strToKeySeq(config_.lock()->getOctaveDownKey().c_str()));
	focusPtnSc_->setKey(Qt::Key_F2);
	focusOdrSc_->setKey(Qt::Key_F3);
	focusInstSc_->setKey(Qt::Key_F4);
	playAndStopSc_->setKey(Qt::Key_Return);
	playStepSc_->setKey(Qt::CTRL + Qt::Key_Return);
	goPrevOdrSc_->setKey(Qt::CTRL + Qt::Key_Left);
	goNextOdrSc_->setKey(Qt::CTRL + Qt::Key_Right);
	prevInstSc_->setKey(Qt::ALT + Qt::Key_Left);
	nextInstSc_->setKey(Qt::ALT + Qt::Key_Right);

	ui->orderList->onShortcutUpdated();
	ui->patternEditor->onShortcutUpdated();
}

void MainWindow::updateInstrumentListColors()
{
	ui->instrumentListWidget->setStyleSheet(
				QString("QListWidget { color: %1; background: %2; }")
				.arg(palette_->ilistTextColor.name(QColor::HexArgb),
					 palette_->ilistBackColor.name(QColor::HexArgb))
				+ QString("QListWidget::item:hover { color: %1; background: %2; }")
				.arg(palette_->ilistTextColor.name(QColor::HexArgb),
					 palette_->ilistHovBackColor.name(QColor::HexArgb))
				+ QString("QListWidget::item:selected { color: %1; background: %2; }")
				.arg(palette_->ilistTextColor.name(QColor::HexArgb),
					 palette_->ilistSelBackColor.name(QColor::HexArgb))
				+ QString("QListWidget::item:selected:hover { color: %1; background: %2; }")
				.arg(palette_->ilistTextColor.name(QColor::HexArgb),
					 palette_->ilistHovSelBackColor.name(QColor::HexArgb)));
}

/********** MIDI **********/
void MainWindow::midiThreadReceivedEvent(double delay, const uint8_t *msg, size_t len, void *userData)
{
	MainWindow *self = reinterpret_cast<MainWindow *>(userData);

	Q_UNUSED(delay)

	// Note-On/Note-Off
	if (len == 3 && (msg[0] & 0xe0) == 0x80) {
		uint8_t status = msg[0];
		uint8_t key = msg[1];
		uint8_t velocity = msg[2];
		QMetaMethod method = self->metaObject()->method(self->midiKeyEventMethod_);
		method.invoke(self, Qt::QueuedConnection,
					  Q_ARG(uchar, status), Q_ARG(uchar, key), Q_ARG(uchar, velocity));
	}
	// Program change
	else if (len == 2 && (msg[0] & 0xf0) == 0xc0) {
		uint8_t status = msg[0];
		uint8_t program = msg[1];
		QMetaMethod method = self->metaObject()->method(self->midiProgramEventMethod_);
		method.invoke(self, Qt::QueuedConnection,
					  Q_ARG(uchar, status), Q_ARG(uchar, program));
	}
}

void MainWindow::midiKeyEvent(uchar status, uchar key, uchar velocity)
{
	bool release = ((status & 0xf0) == 0x80) || velocity == 0;
	int k = static_cast<int>(key) - 12;

	octave_->setValue(k / 12);

	int n = instForms_->checkActivatedFormNumber();
	if (n == -1) {
		bt_->jamKeyOff(k); // possibility to recover on stuck note
		if (!release) bt_->jamKeyOn(k);
	}
	else {
		SoundSource src = instForms_->getFormInstrumentSoundSource(n);
		bt_->jamKeyOffForced(k, src); // possibility to recover on stuck note
		if (!release) bt_->jamKeyOnForced(k, src);
	}
}

void MainWindow::midiProgramEvent(uchar status, uchar program)
{
	Q_UNUSED(status)
	int row = findRowFromInstrumentList(program);
	ui->instrumentListWidget->setCurrentRow(row);
}

/********** Instrument list **********/
void MainWindow::addInstrument()
{
	SoundSource src = bt_->getCurrentTrackAttribute().source;
	switch (src) {
	case SoundSource::FM:
	case SoundSource::SSG:
	case SoundSource::ADPCM:
	{
		auto& list = ui->instrumentListWidget;

		int num = bt_->findFirstFreeInstrumentNumber();
		if (num == -1) return;	// Maximum count check

		QString name = tr("Instrument %1").arg(num);
		bt_->addInstrument(num, name.toUtf8().toStdString());

		comStack_->push(new AddInstrumentQtCommand(
							list, num, name, src, instForms_, this, config_.lock()->getWriteOnlyUsedSamples()));
		ui->instrumentListWidget->setCurrentRow(num);
		break;
	}
	case SoundSource::DRUM:
		break;
	}
}

void MainWindow::removeInstrument(int row)
{
	if (row < 0) return;

	auto& list = ui->instrumentListWidget;
	int num = list->item(row)->data(Qt::UserRole).toInt();

	auto inst = bt_->getInstrument(num);

	bool updateRequest = false;
	if (config_.lock()->getWriteOnlyUsedSamples()){
		if (inst->getSoundSource() == SoundSource::ADPCM) {
			size_t size = bt_->getWaveformADPCMUsers(dynamic_cast<InstrumentADPCM*>(
														 inst.get())->getWaveformNumber()).size();
			if (size == 1) updateRequest = true;
		}
	}

	std::string name = inst->getName();
	SoundSource src = inst->getSoundSource();
	bt_->removeInstrument(num);
	comStack_->push(new RemoveInstrumentQtCommand(list, num, row,
												  QString::fromUtf8(name.c_str(), static_cast<int>(name.length())),
												  src, instForms_, this, updateRequest));
}

void MainWindow::openInstrumentEditor()
{
	auto item = ui->instrumentListWidget->currentItem();
	int num = item->data(Qt::UserRole).toInt();

	if (!instForms_->getForm(num)) {	// Create form
		std::shared_ptr<QWidget> form;
		auto inst = bt_->getInstrument(num);
		switch (inst->getSoundSource()) {
		case SoundSource::FM:		form = std::make_shared<InstrumentEditorFMForm>(num);		break;
		case SoundSource::SSG:		form = std::make_shared<InstrumentEditorSSGForm>(num);		break;
		case SoundSource::ADPCM:	form = std::make_shared<InstrumentEditorADPCMForm>(num);	break;
		default:	return;
		}

		switch (inst->getSoundSource()) {
		case SoundSource::FM:
		{
			auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form.get());
			fmForm->setCore(bt_);
			fmForm->setConfiguration(config_.lock());
			fmForm->setColorPalette(palette_);
			fmForm->resize(config_.lock()->getInstrumentFMWindowWidth(),
						   config_.lock()->getInstrumentFMWindowHeight());

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
							 this, [&](JamKey key) { bt_->jamKeyOnForced(key, SoundSource::FM); },
			Qt::DirectConnection);
			QObject::connect(fmForm, &InstrumentEditorFMForm::jamKeyOffEvent,
							 this, [&](JamKey key) { bt_->jamKeyOffForced(key, SoundSource::FM); },
			Qt::DirectConnection);
			QObject::connect(fmForm, &InstrumentEditorFMForm::modified,
							 this, &MainWindow::setModifiedTrue);

			fmForm->installEventFilter(this);

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
			ssgForm->setCore(bt_);
			ssgForm->setConfiguration(config_.lock());
			ssgForm->setColorPalette(palette_);
			ssgForm->resize(config_.lock()->getInstrumentSSGWindowWidth(),
							config_.lock()->getInstrumentSSGWindowHeight());

			QObject::connect(ssgForm, &InstrumentEditorSSGForm::waveformNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGWaveformNumberChanged);
			QObject::connect(ssgForm, &InstrumentEditorSSGForm::waveformParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGWaveformParameterChanged);
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
							 this, [&](JamKey key) { bt_->jamKeyOnForced(key, SoundSource::SSG); },
			Qt::DirectConnection);
			QObject::connect(ssgForm, &InstrumentEditorSSGForm::jamKeyOffEvent,
							 this, [&](JamKey key) { bt_->jamKeyOffForced(key, SoundSource::SSG); }
			, Qt::DirectConnection);
			QObject::connect(ssgForm, &InstrumentEditorSSGForm::modified,
							 this, &MainWindow::setModifiedTrue);

			ssgForm->installEventFilter(this);

			instForms_->onInstrumentSSGWaveformNumberChanged();
			instForms_->onInstrumentSSGToneNoiseNumberChanged();
			instForms_->onInstrumentSSGEnvelopeNumberChanged();
			instForms_->onInstrumentSSGArpeggioNumberChanged();
			instForms_->onInstrumentSSGPitchNumberChanged();
			break;
		}
		case SoundSource::ADPCM:
		{
			auto adpcmForm = qobject_cast<InstrumentEditorADPCMForm*>(form.get());
			adpcmForm->setCore(bt_);
			adpcmForm->setConfiguration(config_.lock());
			adpcmForm->setColorPalette(palette_);
			adpcmForm->resize(config_.lock()->getInstrumentADPCMWindowWidth(),
							config_.lock()->getInstrumentADPCMWindowHeight());

			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::waveformNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMWaveformNumberChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::waveformParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMWaveformParameterChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::waveformAssignRequested,
							 this, &MainWindow::assignADPCMSamples);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::envelopeNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMEnvelopeNumberChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::envelopeParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMEnvelopeParameterChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::arpeggioNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMArpeggioNumberChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::arpeggioParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMArpeggioParameterChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::pitchNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMPitchNumberChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::pitchParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMPitchParameterChanged);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::jamKeyOnEvent,
							 this, [&](JamKey key) { bt_->jamKeyOnForced(key, SoundSource::ADPCM); },
			Qt::DirectConnection);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::jamKeyOffEvent,
							 this, [&](JamKey key) { bt_->jamKeyOffForced(key, SoundSource::ADPCM); },
			Qt::DirectConnection);
			QObject::connect(adpcmForm, &InstrumentEditorADPCMForm::modified,
							 this, &MainWindow::setModifiedTrue);

			adpcmForm->installEventFilter(this);

			instForms_->onInstrumentADPCMWaveformNumberChanged();
			instForms_->onInstrumentADPCMEnvelopeNumberChanged();
			instForms_->onInstrumentADPCMArpeggioNumberChanged();
			instForms_->onInstrumentADPCMPitchNumberChanged();
			break;
		}
		default:
			break;
		}

		std::string name = inst->getName();
		instForms_->add(num, std::move(form),
						QString::fromUtf8(name.c_str(), static_cast<int>(name.length())),
						inst->getSoundSource());
	}

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

void MainWindow::renameInstrument()
{
	auto list = ui->instrumentListWidget;
	auto item = list->currentItem();
	int num = item->data(Qt::UserRole).toInt();
	auto inst = bt_->getInstrument(num);
	auto oldName = QString::fromUtf8(inst->getName().c_str(), static_cast<int>(inst->getName().length()));
	auto line = new QLineEdit(oldName);
	SoundSource src = inst->getSoundSource();

	QObject::connect(line, &QLineEdit::editingFinished,
					 this, [&, item, list, num, oldName, src] {
		QString newName = qobject_cast<QLineEdit*>(list->itemWidget(item))->text();
		list->removeItemWidget(item);
		bt_->setInstrumentName(num, newName.toUtf8().toStdString());
		int row = findRowFromInstrumentList(num);
		comStack_->push(new ChangeInstrumentNameQtCommand(list, num, row, src, instForms_, oldName, newName));
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
	auto inst = bt_->getInstrument(num);
	std::string name = inst->getName();
	comStack_->push(new CloneInstrumentQtCommand(
						ui->instrumentListWidget, num, inst->getSoundSource(),
						QString::fromUtf8(name.c_str(), static_cast<int>(name.length())), instForms_));
	//----------//
}

void MainWindow::deepCloneInstrument()
{
	int num = bt_->findFirstFreeInstrumentNumber();
	if (num == -1) return;

	int refNum = ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	// KEEP CODE ORDER //
	bt_->deepCloneInstrument(num, refNum);
	auto inst = bt_->getInstrument(num);
	std::string name = inst->getName();
	comStack_->push(new DeepCloneInstrumentQtCommand(
						ui->instrumentListWidget, num, inst->getSoundSource(),
						QString::fromUtf8(name.c_str(), static_cast<int>(name.length())), instForms_,
						this, config_.lock()->getWriteOnlyUsedSamples()));
	//----------//
}

void MainWindow::loadInstrument()
{
	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QStringList filters {
		tr("BambooTracker instrument (*.bti)"),
		tr("DefleMask preset (*.dmp)"),
		tr("TFM Music Maker instrument (*.tfi)"),
		tr("VGM Music Maker instrument (*.vgi)"),
		tr("WOPN instrument (*.opni)"),
		tr("Gens KMod dump (*.y12)"),
		tr("MVSTracker instrument (*.ins)")
	};
	QString defaultFilter = filters.at(config_.lock()->getInstrumentOpenFormat());

	QString file = QFileDialog::getOpenFileName(this, tr("Open instrument"), (dir.isEmpty() ? "./" : dir),
												filters.join(";;"), &defaultFilter);
	if (file.isNull()) return;

	int index = getSelectedFileFilter(file, filters);
	if (index != -1) config_.lock()->setInstrumentOpenFormat(index);


	funcLoadInstrument(file);
}

void MainWindow::funcLoadInstrument(QString file)
{
	int n = bt_->findFirstFreeInstrumentNumber();
	if (n == -1) {
		showFileIOErrorDialog(FileInputError(FileIO::FileType::Inst),
							  tr( "The number of instruments has reached the upper limit."));
		return;
	}

	try {
		QFile fp(file);
		if (!fp.open(QIODevice::ReadOnly)) throw FileInputError(FileIO::FileType::Inst);
		QByteArray array = fp.readAll();
		fp.close();

		BinaryContainer contaner;
		contaner.appendVector(std::vector<char>(array.begin(), array.end()));
		bt_->loadInstrument(contaner, file.toStdString(), n);

		auto inst = bt_->getInstrument(n);
		auto name = inst->getName();
		comStack_->push(new AddInstrumentQtCommand(
							ui->instrumentListWidget, n,
							QString::fromUtf8(name.c_str(), static_cast<int>(name.length())),
							inst->getSoundSource(), instForms_,
							this, config_.lock()->getWriteOnlyUsedSamples()));
		ui->instrumentListWidget->setCurrentRow(n);
		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileInputError(FileIO::FileType::Inst), "\n" + QString(e.what()));
	}
}

void MainWindow::saveInstrument()
{
	int n = ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	auto nameStd = bt_->getInstrument(n)->getName();
	QString name = QString::fromUtf8(nameStd.c_str(), static_cast<int>(nameStd.length()));

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString file = QFileDialog::getSaveFileName(
					   this, tr("Save instrument"),
					   QString("%1/%2.bti").arg(dir.isEmpty() ? "." : dir, name),
					   tr("BambooTracker instrument (*.bti)"));
	if (file.isNull()) return;
	if (!file.endsWith(".bti")) file += ".bti";	// For linux

	try {
		BinaryContainer container;
		bt_->saveInstrument(container, n);

		QFile fp(file);
		if (!fp.open(QIODevice::WriteOnly)) throw FileOutputError(FileIO::FileType::Inst);
		fp.write(container.getPointer(), container.size());
		fp.close();

		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileOutputError(FileIO::FileType::Inst), "\n" + QString(e.what()));
	}
}

void MainWindow::importInstrumentsFromBank()
{
	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QStringList filters {
		tr("BambooTracker bank (*.btb)"),
				tr("WOPN bank (*.wopn)"),
				tr("PMD PPC (*.ppc)"),
				tr("FMP PVI (*.pvi)")
	};
	QString defaultFilter = filters.at(config_.lock()->getBankOpenFormat());

	QString file = QFileDialog::getOpenFileName(this, tr("Open bank"), (dir.isEmpty() ? "./" : dir),
												filters.join(";;"), &defaultFilter);
	if (file.isNull()) {
		return;
	}
	else {
		int index = getSelectedFileFilter(file, filters);
		if (index != -1) config_.lock()->setBankOpenFormat(index);
	}

	funcImportInstrumentsFromBank(file);
}

void MainWindow::funcImportInstrumentsFromBank(QString file)
{
	std::unique_ptr<AbstractBank> bank;
	try {
		QFile fp(file);
		if (!fp.open(QIODevice::ReadOnly)) throw FileInputError(FileIO::FileType::Bank);
		QByteArray array = fp.readAll();
		fp.close();

		BinaryContainer container;
		container.appendVector(std::vector<char>(array.begin(), array.end()));

		bank.reset(BankIO::loadBank(container, file.toStdString()));
		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
		return;
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileInputError(FileIO::FileType::Bank), "\n" + QString(e.what()));
		return;
	}

	InstrumentSelectionDialog dlg(*bank, tr("Select instruments to load:"), this);
	if (dlg.exec() != QDialog::Accepted)
		return;

	QVector<size_t> selection = dlg.currentInstrumentSelection();
	if (selection.empty()) return;

	try {
		bool sampleRestoreRequested = false;
		int lastNum = ui->instrumentListWidget->currentRow();
		for (size_t index : selection) {
			int n = bt_->findFirstFreeInstrumentNumber();
			if (n == -1){
				showFileIOErrorDialog(FileInputError(FileIO::FileType::Inst),
									  tr( "The number of instruments has reached the upper limit."));
				ui->instrumentListWidget->setCurrentRow(lastNum);
				return;
			}

			bt_->importInstrument(*bank, index, n);

			auto inst = bt_->getInstrument(n);
			auto name = inst->getName();
			comStack_->push(new AddInstrumentQtCommand(
								ui->instrumentListWidget, n,
								QString::fromUtf8(name.c_str(), static_cast<int>(name.length())),
								inst->getSoundSource(), instForms_,
								this, config_.lock()->getWriteOnlyUsedSamples(), true));
			lastNum = n;

			sampleRestoreRequested |= (inst->getSoundSource() == SoundSource::ADPCM);
		}
		ui->instrumentListWidget->setCurrentRow(lastNum);

		if (sampleRestoreRequested) assignADPCMSamples();	// Store only once
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileInputError(FileIO::FileType::Bank), "\n" + QString(e.what()));
	}
}

void MainWindow::exportInstrumentsToBank()
{
	std::vector<int> ids = bt_->getInstrumentIndices();
	std::shared_ptr<BtBank> bank(std::make_shared<BtBank>(ids, bt_->getInstrumentNames()));

	InstrumentSelectionDialog dlg(*bank, tr("Select instruments to save:"), this);
	if (dlg.exec() != QDialog::Accepted)
		return;

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString file = QFileDialog::getSaveFileName(this, tr("Save bank"), (dir.isEmpty() ? "./" : dir),
												tr("BambooTracker bank (*.btb)"));
	if (file.isNull()) return;

	QVector<size_t> selection = dlg.currentInstrumentSelection();
	if (selection.empty()) return;
	std::vector<int> sel;
	std::transform(selection.begin(), selection.end(), std::back_inserter(sel),
				   [&ids](size_t i) { return ids.at(i); });
	std::sort(sel.begin(), sel.end());

	try {
		BinaryContainer container;
		bt_->exportInstruments(container, sel);

		QFile fp(file);
		if (!fp.open(QIODevice::WriteOnly)) throw FileOutputError(FileIO::FileType::Bank);
		fp.write(container.getPointer(), container.size());
		fp.close();

		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileOutputError(FileIO::FileType::Bank), "\n" + QString(e.what()));
	}
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
	on_instrumentListWidget_itemSelectionChanged();

	auto modTitle = bt_->getModuleTitle();
	ui->modTitleLineEdit->setText(
				QString::fromUtf8(modTitle.c_str(), static_cast<int>(modTitle.length())));
	ui->modTitleLineEdit->setCursorPosition(0);
	auto modAuthor = bt_->getModuleAuthor();
	ui->authorLineEdit->setText(
				QString::fromUtf8(modAuthor.c_str(), static_cast<int>(modAuthor.length())));
	ui->authorLineEdit->setCursorPosition(0);
	auto modCopyright = bt_->getModuleCopyright();
	ui->copyrightLineEdit->setText(
				QString::fromUtf8(modCopyright.c_str(), static_cast<int>(modCopyright.length())));
	ui->copyrightLineEdit->setCursorPosition(0);
	ui->songComboBox->clear();
	for (size_t i = 0; i < bt_->getSongCount(); ++i) {
		std::string srcTitle = bt_->getSongTitle(static_cast<int>(i));
		QString title = QString::fromUtf8(srcTitle.c_str(), static_cast<int>(srcTitle.length()));
		if (title.isEmpty()) title = tr("Untitled");
		ui->songComboBox->addItem(QString("#%1 %2").arg(i).arg(title));
	}
	highlight1_->setValue(static_cast<int>(bt_->getModuleStepHighlight1Distance()));
	highlight2_->setValue(static_cast<int>(bt_->getModuleStepHighlight2Distance()));

	for (auto& idx : bt_->getInstrumentIndices()) {
		auto inst = bt_->getInstrument(idx);
		auto name = inst->getName();
		comStack_->push(new AddInstrumentQtCommand(
							ui->instrumentListWidget, idx,
							QString::fromUtf8(name.c_str(), static_cast<int>(name.length())),
							inst->getSoundSource(), instForms_,
							this, config_.lock()->getWriteOnlyUsedSamples(), true));
	}

	isSavedModBefore_ = false;

	loadSong();

	// Set tick frequency
	stream_->setInterruption(bt_->getModuleTickFrequency());
	if (timer_) timer_->setInterval(1000000 / bt_->getModuleTickFrequency());
	statusIntr_->setText(QString::number(bt_->getModuleTickFrequency()) + QString("Hz"));

	// Set mixer
	QString text;
	switch (bt_->getModuleMixerType()) {
	case MixerType::UNSPECIFIED:
		bt_->setMasterVolumeFM(config_.lock()->getMixerVolumeFM());
		bt_->setMasterVolumeSSG(config_.lock()->getMixerVolumeSSG());
		text = tr("-");
		break;
	case MixerType::CUSTOM:
		bt_->setMasterVolumeFM(bt_->getModuleCustomMixerFMLevel());
		bt_->setMasterVolumeSSG(bt_->getModuleCustomMixerSSGLevel());
		text = tr("Custom");
		break;
	case MixerType::PC_9821_PC_9801_86:
		bt_->setMasterVolumeFM(0);
		bt_->setMasterVolumeSSG(-5.5);
		text = tr("PC-9821 with PC-9801-86");
		break;
	case MixerType::PC_9821_SPEAK_BOARD:
		bt_->setMasterVolumeFM(0);
		bt_->setMasterVolumeSSG(-3.0);
		text = tr("PC-9821 with Speak Board");
		break;
	case MixerType::PC_8801_VA2:
		bt_->setMasterVolumeFM(0);
		bt_->setMasterVolumeSSG(1.5);
		text = tr("PC-88VA2");
		break;
	case MixerType::PC_8801_MKII_SR:
		bt_->setMasterVolumeFM(0);
		bt_->setMasterVolumeSSG(2.5);
		text = tr("NEC PC-8801mkIISR");
		break;
	}
	statusMixer_->setText(text);

	// Clear records
	QApplication::clipboard()->clear();
	comStack_->clear();
	bt_->clearCommandHistory();
}

void MainWindow::openModule(QString file)
{
	try {
		freezeViews();
		if (timer_) timer_->stop();
		else stream_->stop();

		BinaryContainer container;
		QFile fp(file);
		if (!fp.open(QIODevice::ReadOnly)) throw FileInputError(FileIO::FileType::Mod);
		QByteArray array = fp.readAll();
		fp.close();

		container.appendVector(std::vector<char>(array.begin(), array.end()));
		bt_->loadModule(container);
		bt_->setModulePath(file.toStdString());

		loadModule();

		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
		changeFileHistory(file);
	}
	catch (std::exception& e) {
		if (auto ef = dynamic_cast<FileIOError*>(&e)) {
			showFileIOErrorDialog(*ef);
		}
		else {
			showFileIOErrorDialog(FileInputError(FileIO::FileType::Mod), "\n" + QString(e.what()));
		}
		// Init module
		freezeViews();
		bt_->makeNewModule();
		loadModule();
	}

	isModifiedForNotCommand_ = false;
	setWindowModified(false);
	if (timer_) timer_->start();
	else stream_->start();
	setInitialSelectedInstrument();
	assignADPCMSamples();
}

void MainWindow::loadSong()
{
	// Init position
	int songCnt = static_cast<int>(bt_->getSongCount());
	if (ui->songComboBox->currentIndex() >= songCnt)
		bt_->setCurrentSongNumber(songCnt - 1);
	else
		bt_->setCurrentSongNumber(bt_->getCurrentSongNumber());
	bt_->setCurrentOrderNumber(0);
	bt_->setCurrentTrack(0);
	bt_->setCurrentStepNumber(0);

	// Init ui
	ui->orderList->unfreeze();
	ui->patternEditor->unfreeze();
	ui->orderList->onSongLoaded();
	ui->orderListGroupBox->setMaximumWidth(
				ui->orderListGroupBox->contentsMargins().left()
				+ ui->orderListGroupBox->layout()->contentsMargins().left()
				+ ui->orderList->maximumWidth()
				+ ui->orderListGroupBox->layout()->contentsMargins().right()
				+ ui->orderListGroupBox->contentsMargins().right());
	ui->patternEditor->onSongLoaded();

	int curSong = bt_->getCurrentSongNumber();
	ui->songComboBox->setCurrentIndex(curSong);
	ui->tempoSpinBox->setValue(bt_->getSongTempo(curSong));
	ui->speedSpinBox->setValue(bt_->getSongSpeed(curSong));
	ui->patternSizeSpinBox->setValue(static_cast<int>(bt_->getDefaultPatternSize(curSong)));
	ui->grooveSpinBox->setValue(bt_->getSongGroove(curSong));
	ui->grooveSpinBox->setMaximum(static_cast<int>(bt_->getGrooveCount()) - 1);
	if (bt_->isUsedTempoInSong(curSong)) {
		ui->tempoSpinBox->setEnabled(true);
		ui->speedSpinBox->setEnabled(true);
		ui->grooveCheckBox->setChecked(false);
		ui->grooveSpinBox->setEnabled(false);
	}
	else {
		ui->tempoSpinBox->setEnabled(false);
		ui->speedSpinBox->setEnabled(false);
		ui->grooveCheckBox->setChecked(true);
		ui->grooveSpinBox->setEnabled(true);
	}
	onCurrentTrackChanged();

	setWindowTitle();
	switch (bt_->getSongStyle(bt_->getCurrentSongNumber()).type) {
	case SongType::Standard:		statusStyle_->setText(tr("Standard"));			break;
	case SongType::FM3chExpanded:	statusStyle_->setText(tr("FM3ch expanded"));	break;
	}
	statusPlayPos_->setText(config_.lock()->getShowRowNumberInHex() ? "00/00" : "000/000");

	bmManForm_->onCurrentSongNumberChanged();
}

void MainWindow::assignADPCMSamples()
{
	bt_->stopPlaySong();
	lockWidgets(false);
	if (timer_) timer_->stop();
	else stream_->stop();
	bt_->assignWaveformADPCMSamples();	// Mutex register
	instForms_->onInstrumentADPCMSampleMemoryUpdated();
	if (timer_) timer_->start();
	else stream_->start();
}

/********** Play song **********/
void MainWindow::startPlaySong()
{
	bt_->startPlaySong();
	lockWidgets(true);
	firstViewUpdateRequest_ = true;
}

void MainWindow::startPlayFromStart()
{
	bt_->startPlayFromStart();
	lockWidgets(true);
	firstViewUpdateRequest_ = true;
}

void MainWindow::startPlayPattern()
{
	bt_->startPlayPattern();
	lockWidgets(true);
	firstViewUpdateRequest_ = true;
}

void MainWindow::startPlayFromCurrentStep()
{
	bt_->startPlayFromCurrentStep();
	lockWidgets(true);
	firstViewUpdateRequest_ = true;
}

void MainWindow::startPlayFromMarker()
{
	if (bt_->startPlayFromMarker()) {
		lockWidgets(true);
		firstViewUpdateRequest_ = true;
	}
}

void MainWindow::playStep()
{
	if (!bt_->isPlaySong()) {
		bt_->playStep();
		firstViewUpdateRequest_ = true;
		ui->patternEditor->onPlayStepPressed();
	}
}

void MainWindow::stopPlaySong()
{
	bt_->stopPlaySong();
	lockWidgets(false);
	ui->patternEditor->onStoppedPlaySong();
	ui->orderList->onStoppedPlaySong();
}

void MainWindow::lockWidgets(bool isLock)
{
	hasLockedWigets_ = isLock;
	ui->songComboBox->setEnabled(!isLock);
}

/********** Octave change **********/
void MainWindow::changeOctave(bool upFlag)
{
	if (upFlag) octave_->stepUp();
	else octave_->stepDown();

	statusOctave_->setText(tr("Octave: %1").arg(bt_->getCurrentOctave()));
}

/********** Configuration change **********/
void MainWindow::changeConfiguration()
{
	// Real chip interface
	RealChipInterface intf = config_.lock()->getRealChipInterface();
	if (intf == RealChipInterface::NONE) {
		timer_.reset();
		bt_->useSCCI(nullptr);
		bt_->useC86CTL(nullptr);
		bool streamState = stream_->initialize(
							   config_.lock()->getSampleRate(),
							   config_.lock()->getBufferLength(),
							   bt_->getModuleTickFrequency(),
							   QString::fromUtf8(config_.lock()->getSoundAPI().c_str(),
												 static_cast<int>(config_.lock()->getSoundAPI().length())),
							   QString::fromUtf8(config_.lock()->getSoundDevice().c_str(),
												 static_cast<int>(config_.lock()->getSoundDevice().length())));
		if (!streamState) showStreamFailedDialog();
		stream_->start();
	}
	else {
		stream_->stop();
		if (timer_) {
			timer_->stop();
		}
		else {
			timer_ = std::make_unique<Timer>();
			timer_->setInterval(1000000 / bt_->getModuleTickFrequency());
			tickEventMethod_ = metaObject()->indexOfSlot("onNewTickSignaledRealChip()");
			Q_ASSERT(tickEventMethod_ != -1);
			timer_->setFunction([&]{
				QMetaMethod method = this->metaObject()->method(this->tickEventMethod_);
				method.invoke(this, Qt::QueuedConnection);
			});
		}

		setRealChipInterface(intf);

		timer_->start();
	}

	setMidiConfiguration();
	updateFonts();
	ui->orderList->setHorizontalScrollMode(config_.lock()->getMoveCursorByHorizontalScroll());
	ui->patternEditor->setHorizontalScrollMode(config_.lock()->getMoveCursorByHorizontalScroll());
	instForms_->updateByConfiguration();

	bt_->changeConfiguration(config_);

	setShortcuts();
	ui->waveVisual->setVisible(config_.lock()->getShowWaveVisual());
	updateInstrumentListColors();
	bmManForm_->onConfigurationChanged(config_.lock()->getShowRowNumberInHex());

	update();
}

void MainWindow::setRealChipInterface(RealChipInterface intf)
{
	if (intf == bt_->getRealChipinterface()) return;

	if (isWindowModified()
			&& QMessageBox::warning(this, tr("Warning"),
									tr("The module has been changed. Do you want to save it?"),
									QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
		on_actionSave_As_triggered();
	}

	switch (intf) {
	case RealChipInterface::SCCI:
		bt_->useC86CTL(nullptr);
		scciDll_->load();
		if (scciDll_->isLoaded()) {
			auto getManager = reinterpret_cast<scci::SCCIFUNC>(
								  scciDll_->resolve("getSoundInterfaceManager"));
			bt_->useSCCI(getManager ? getManager() : nullptr);
		}
		else {
			bt_->useSCCI(nullptr);
		}
		break;
	case RealChipInterface::C86CTL:
		bt_->useSCCI(nullptr);
		c86ctlDll_->load();
		if (c86ctlDll_->isLoaded()) {
			bt_->useC86CTL(new C86ctlBase(c86ctlDll_->resolve("CreateInstance")));
		}
		else {
			bt_->useC86CTL(nullptr);
		}
		break;
	default:
		break;
	}

	bt_->assignWaveformADPCMSamples();	// Mutex register
	instForms_->onInstrumentADPCMSampleMemoryUpdated();
}

void MainWindow::setMidiConfiguration()
{
	MidiInterface &midiIntf = MidiInterface::instance();
	std::string midiInPortName = config_.lock()->getMidiInputPort();

	if (!midiIntf.hasInitializedInput()) {
		QMessageBox::critical(this, tr("Error"),
							  tr("Could not initialize MIDI input."),
							  QMessageBox::Ok, QMessageBox::Ok);
	}

	if (!midiInPortName.empty())
		midiIntf.openInputPortByName(midiInPortName);
	else if (midiIntf.supportsVirtualPort())
		midiIntf.openInputPort(~0u);
}

void MainWindow::updateFonts()
{
	ui->patternEditor->setFonts(
				QString::fromUtf8(config_.lock()->getPatternEditorHeaderFont().c_str(),
								  static_cast<int>(config_.lock()->getPatternEditorHeaderFont().length())),
				config_.lock()->getPatternEditorHeaderFontSize(),
				QString::fromUtf8(config_.lock()->getPatternEditorRowsFont().c_str(),
								  static_cast<int>(config_.lock()->getPatternEditorRowsFont().length())),
				config_.lock()->getPatternEditorRowsFontSize());
	ui->orderList->setFonts(
				QString::fromUtf8(config_.lock()->getOrderListHeaderFont().c_str(),
								  static_cast<int>(config_.lock()->getOrderListHeaderFont().length())),
				config_.lock()->getOrderListHeaderFontSize(),
				QString::fromUtf8(config_.lock()->getOrderListRowsFont().c_str(),
								  static_cast<int>(config_.lock()->getOrderListRowsFont().length())),
				config_.lock()->getOrderListRowsFontSize());
}

/********** History change **********/
void MainWindow::changeFileHistory(QString file)
{
	fileHistory_->addFile(file);
	for (int i = ui->menu_Recent_Files->actions().count() - 1; 1 < i; --i)
		ui->menu_Recent_Files->removeAction(ui->menu_Recent_Files->actions().at(i));
	for (size_t i = 0; i < fileHistory_->size(); ++i) {
		// Leave Before Qt5.7.0 style due to windows xp
		QAction* action = ui->menu_Recent_Files->addAction(QString("&%1 %2").arg(i + 1).arg(fileHistory_->at(i)));
		action->setData(fileHistory_->at(i));
	}
}

/********** Backup **********/
bool MainWindow::backupModule(QString srcFile)
{
	if (!isSavedModBefore_ && config_.lock()->getBackupModules()) {
		bool err = false;
		QString backup = srcFile + ".bak";
		if (QFile::exists(backup)) err = !QFile::remove(backup);
		if (err || !QFile::copy(srcFile, backup)) {
			QMessageBox::critical(this, tr("Error"), tr("Failed to backup module."));
			return false;
		}
	}
	return true;
}

/******************************/
void MainWindow::setWindowTitle()
{
	int n = bt_->getCurrentSongNumber();
	auto filePathStd = bt_->getModulePath();
	auto songTitleStd = bt_->getSongTitle(n);
	QString filePath = QString::fromStdString(filePathStd);
	QString fileName = filePath.isEmpty() ? tr("Untitled") : QFileInfo(filePath).fileName();
	QString songTitle = QString::fromUtf8(songTitleStd.c_str(), static_cast<int>(songTitleStd.length()));
	if (songTitle.isEmpty()) songTitle = tr("Untitled");
	QMainWindow::setWindowTitle(QString("%1[*] [#%2 %3] - BambooTracker")
								.arg(fileName).arg(QString::number(n)).arg(songTitle));
}

void MainWindow::setModifiedTrue()
{
	isModifiedForNotCommand_ = true;
	setWindowModified(true);
}

void MainWindow::setInitialSelectedInstrument()
{
	if (bt_->getInstrumentIndices().empty()) {
		bt_->setCurrentInstrument(-1);
		statusInst_->setText(tr("No instrument"));
	}
	else {
		ui->instrumentListWidget->setCurrentRow(0);
	}
}

QString MainWindow::getModuleFileBaseName() const
{
	auto filePathStd = bt_->getModulePath();
	QString filePath = QString::fromStdString(filePathStd);
	return (filePath.isEmpty() ? tr("Untitled") : QFileInfo(filePath).completeBaseName());
}

int MainWindow::getSelectedFileFilter(QString& file, QStringList& filters) const
{
	QRegularExpression re(R"(\(\*\.(.+)\))");
	QString ex = QFileInfo(file).suffix();
	for (int i = 0; i < filters.size(); ++i)
		if (ex == re.match(filters[i]).captured(1)) return i;
	return -1;
}

void MainWindow::showFileIOErrorDialog(const FileIOError& e, const QString sub)
{
	const FileIOError *err = &e;
	QString text, type;

	switch (err->getFileType()) {
	case FileIO::FileType::Mod:		type = tr("module");		break;
	case FileIO::FileType::S98:		type = tr("s98");			break;
	case FileIO::FileType::VGM:		type = tr("vgm");			break;
	case FileIO::FileType::WAV:		type = tr("wav");			break;
	case FileIO::FileType::Bank:	type = tr("bank");			break;
	case FileIO::FileType::Inst:	type = tr("instrument");	break;
	default:	break;
	}

	if (dynamic_cast<const FileInputError*>(err)) {
		text = tr("Failed to load the %1.").arg(type);
	}
	else if (dynamic_cast<const FileOutputError*>(err)) {
		switch (err->getFileType()) {
		case FileIO::FileType::S98:
		case FileIO::FileType::VGM:
		case FileIO::FileType::WAV:
			text = tr("Failed to export to %1.");
			break;
		default:
			text = tr("Failed to save the %1.");
			break;
		}
		text = text.arg(type);
	}
	else if (dynamic_cast<const FileVersionError*>(err)) {
		text = tr("Could not load the %1 properly. "
				  "Please make sure that you have the latest version of BambooTracker.").arg(type);
	}
	else if (dynamic_cast<const FileCorruptionError*>(err)) {
		text = tr("Could not load the %1. It may be corrupted.").arg(type);
	}

	QMessageBox::critical(this, tr("Error"), text + sub);
}

/******************************/
/********** Instrument list events **********/
void MainWindow::on_instrumentListWidget_customContextMenuRequested(const QPoint &pos)
{
	auto& list = ui->instrumentListWidget;
	QPoint globalPos = list->mapToGlobal(pos);
	QMenu menu;

	// Leave Before Qt5.7.0 style due to windows xp
	menu.addAction(ui->actionNew_Instrument);
	menu.addAction(ui->actionRemove_Instrument);
	menu.addSeparator();
	menu.addAction(ui->actionRename_Instrument);
	menu.addSeparator();
	menu.addAction(ui->actionClone_Instrument);
	menu.addAction(ui->actionDeep_Clone_Instrument);
	menu.addSeparator();
	menu.addAction(ui->actionLoad_From_File);
	menu.addAction(ui->actionSave_To_File);
	menu.addSeparator();
	menu.addAction(ui->actionImport_From_Bank_File);
	menu.addAction(ui->actionExport_To_Bank_File);
	menu.addSeparator();
	menu.addAction(ui->actionEdit);

	menu.exec(globalPos);
}

void MainWindow::on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item)
{
	Q_UNUSED(item)
	openInstrumentEditor();
}

void MainWindow::on_instrumentListWidget_itemSelectionChanged()
{
	int num = (ui->instrumentListWidget->currentRow() == -1)
			  ? -1
			  : ui->instrumentListWidget->currentItem()->data(Qt::UserRole).toInt();
	bt_->setCurrentInstrument(num);

	if (num == -1) statusInst_->setText(tr("No instrument"));
	else statusInst_->setText(
				tr("Instrument: %1").arg(QString("%1").arg(num, 2, 16, QChar('0')).toUpper()));

	bool canAdd = (bt_->findFirstFreeInstrumentNumber() != -1);
	ui->actionLoad_From_File->setEnabled(canAdd);
	ui->actionImport_From_Bank_File->setEnabled(canAdd);

	bool isSelected = (num != -1);
	ui->actionRemove_Instrument->setEnabled(isSelected);
	ui->actionClone_Instrument->setEnabled(isSelected);
	ui->actionDeep_Clone_Instrument->setEnabled(isSelected);
	ui->actionSave_To_File->setEnabled(isSelected);
	ui->actionExport_To_Bank_File->setEnabled(isSelected);
	ui->actionRename_Instrument->setEnabled(isSelected);
	ui->actionEdit->setEnabled(isSelected);
}

void MainWindow::on_grooveCheckBox_stateChanged(int arg1)
{
	if (arg1 == Qt::Checked) {
		ui->tempoSpinBox->setEnabled(false);
		ui->speedSpinBox->setEnabled(false);
		ui->grooveSpinBox->setEnabled(true);
		bt_->toggleTempoOrGrooveInSong(bt_->getCurrentSongNumber(), false);
	}
	else {
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

void MainWindow::on_actionDelete_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onDeletePressed();
	else if (isEditedOrder_) ui->orderList->deleteOrder();
	else if (isEditedInstList_) on_actionRemove_Instrument_triggered();
}

void MainWindow::updateMenuByPattern()
{
	isEditedPattern_ = true;
	isEditedOrder_ = false;
	isEditedInstList_ = false;

	if (bt_->isJamMode()) {
		// Edit
		ui->actionPaste->setEnabled(false);
		ui->actionMix->setEnabled(false);
		ui->actionOverwrite->setEnabled(false);
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
		ui->actionMix->setEnabled(enabled);
		ui->actionOverwrite->setEnabled(enabled);
		ui->actionDelete->setEnabled(true);
		// Pattern
		ui->actionInterpolate->setEnabled(isSelectedPattern_);
		ui->actionReverse->setEnabled(isSelectedPattern_);
		ui->actionReplace_Instrument->setEnabled(
					isSelectedPattern_ && ui->instrumentListWidget->currentRow() != -1);
		ui->actionExpand->setEnabled(isSelectedPattern_);
		ui->actionShrink->setEnabled(isSelectedPattern_);
		ui->actionDecrease_Note->setEnabled(true);
		ui->actionIncrease_Note->setEnabled(true);
		ui->actionDecrease_Octave->setEnabled(true);
		ui->actionIncrease_Octave->setEnabled(true);
	}

	updateMenuByPatternSelection(isSelectedPattern_);
}

void MainWindow::updateMenuByOrder()
{
	isEditedPattern_ = false;
	isEditedOrder_ = true;
	isEditedInstList_ = false;

	// Edit
	bool enabled = QApplication::clipboard()->text().startsWith("ORDER_");
	ui->actionCut->setEnabled(false);
	ui->actionPaste->setEnabled(enabled);
	ui->actionMix->setEnabled(false);
	ui->actionOverwrite->setEnabled(false);
	ui->actionDelete->setEnabled(true);
	// Song
	bool canAdd = bt_->canAddNewOrder(bt_->getCurrentSongNumber());
	ui->actionInsert_Order->setEnabled(canAdd);
	//ui->actionRemove_Order->setEnabled(true);
	ui->actionDuplicate_Order->setEnabled(canAdd);
	//ui->actionMove_Order_Up->setEnabled(true);
	//ui->actionMove_Order_Down->setEnabled(true);
	ui->actionClone_Patterns->setEnabled(canAdd);
	ui->actionClone_Order->setEnabled(canAdd);
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

	updateMenuByOrderSelection(isSelectedOrder_);
}

void MainWindow::onCurrentTrackChanged()
{
	bool canAdd = ((bt_->getCurrentTrackAttribute().source != SoundSource::DRUM)
				   && (bt_->findFirstFreeInstrumentNumber() != -1));
	ui->actionNew_Instrument->setEnabled(canAdd);
}

void MainWindow::updateMenuByInstrumentList()
{
	isEditedPattern_ = false;
	isEditedOrder_ = false;
	isEditedInstList_ = true;

	// Edit
	ui->actionPaste->setEnabled(false);
	ui->actionMix->setEnabled(false);
	ui->actionOverwrite->setEnabled(false);
	ui->actionDelete->setEnabled(true);

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

void MainWindow::updateMenuByPatternSelection(bool isSelected)
{
	isSelectedPattern_ = isSelected;

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

void MainWindow::updateMenuByOrderSelection(bool isSelected)
{
	isSelectedOrder_ = isSelected;

	// Edit
	ui->actionCopy->setEnabled(isSelected);
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
	ui->orderList->insertOrderBelow();
}

void MainWindow::on_actionRemove_Order_triggered()
{
	ui->orderList->deleteOrder();
}

void MainWindow::on_actionModule_Properties_triggered()
{
	ModulePropertiesDialog dialog(bt_, config_.lock()->getMixerVolumeFM(), config_.lock()->getMixerVolumeSSG());
	if (dialog.exec() == QDialog::Accepted
			&& showUndoResetWarningDialog(tr("Do you want to change song properties?"))) {
		int instRow = ui->instrumentListWidget->currentRow();
		bt_->stopPlaySong();
		lockWidgets(false);
		dialog.onAccepted();
		freezeViews();
		if (!timer_) stream_->stop();
		loadModule();
		setModifiedTrue();
		setWindowTitle();
		ui->instrumentListWidget->setCurrentRow(instRow);
		if (!timer_) stream_->start();
		assignADPCMSamples();
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
	openInstrumentEditor();
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

	bool editable = !bt_->isJamMode();
	statusDetail_->setText(editable ? tr("Change to edit mode") : tr("Change to jam mode"));
	ui->action_Toggle_Bookmark->setEnabled(editable);
}

void MainWindow::on_actionToggle_Track_triggered()
{
	ui->patternEditor->onToggleTrackPressed();
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
	QMessageBox box(QMessageBox::NoIcon,
					tr("About"),
					QString("<h2>BambooTracker v%1</h2>").arg(
						QString::fromStdString(Version::ofApplicationInString()))
					+ tr("<b>YM2608 (OPNA) Music Tracker<br>"
						 "Copyright (C) 2018-2020 Rerrah</b><br>"
						 "<hr>"
						 "Libraries:<br>"
						 "- C86CTL by (C) honet (BSD 3-Clause)<br>"
						 "- libOPNMIDI by (C) Vitaly Novichkov (MIT License part)<br>"
						 "- MAME (MAME License)<br>"
						 "- Nuked OPN-MOD by (C) Alexey Khokholov (Nuke.YKT)<br>"
						 "and (C) Jean Pierre Cimalando (LGPL v2.1)<br>"
						 "- RtAudio by (C) Gary P. Scavone (RtAudio License)<br>"
						 "- RtMidi by (C) Gary P. Scavone (RtMidi License)<br>"
						 "- SCCI by (C) gasshi (SCCI License)<br>"
						 "- Silk icon set 1.3 by (C) Mark James (CC BY 2.5)<br>"
						 "- Qt (GPL v2+ or LGPL v3)<br>"
						 "- VGMPlay by (C) Valley Bell (GPL v2)<br>"
						 "<br>"
						 "Also see changelog which lists contributors."),
					QMessageBox::Ok,
					this);
	box.setIconPixmap(QIcon(":/icon/app_icon").pixmap(QSize(44, 44)));
	box.exec();
}

void MainWindow::on_actionFollow_Mode_triggered()
{
	bt_->setFollowPlay(ui->actionFollow_Mode->isChecked());
	config_.lock()->setFollowMode(ui->actionFollow_Mode->isChecked());

	ui->orderList->onFollowModeChanged();
	ui->patternEditor->onFollowModeChanged();
}

void MainWindow::on_actionGroove_Settings_triggered()
{
	std::vector<std::vector<int>> seqs(bt_->getGrooveCount());
	std::generate(seqs.begin(), seqs.end(), [&, i = 0]() mutable { return bt_->getGroove(i++); });

	GrooveSettingsDialog diag;
	diag.setGrooveSquences(seqs);
	if (diag.exec() == QDialog::Accepted) {
		bt_->stopPlaySong();
		lockWidgets(false);
		bt_->setGrooves(diag.getGrooveSequences());
		ui->grooveSpinBox->setMaximum(static_cast<int>(bt_->getGrooveCount()) - 1);
		setModifiedTrue();
	}
}

void MainWindow::on_actionConfiguration_triggered()
{
	ConfigurationDialog diag(config_.lock(), palette_, stream_->getCurrentBackend(), stream_->getAvailableBackends());
	QObject::connect(&diag, &ConfigurationDialog::applyPressed, this, &MainWindow::changeConfiguration);

	if (diag.exec() == QDialog::Accepted) {
		bt_->stopPlaySong();
		changeConfiguration();
		ConfigurationHandler::saveConfiguration(config_.lock());
		ColorPaletteHandler::savePalette(palette_);
		lockWidgets(false);
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
		QString modTitle = QString::fromUtf8(modTitleStd.c_str(), static_cast<int>(modTitleStd.length()));
		if (modTitle.isEmpty()) modTitle = tr("Untitled");
		QMessageBox dialog(QMessageBox::Warning,
						   "BambooTracker",
						   tr("Save changes to %1?").arg(modTitle),
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
	lockWidgets(false);
	freezeViews();
	if (!timer_) stream_->stop();
	bt_->makeNewModule();
	loadModule();
	setInitialSelectedInstrument();
	isModifiedForNotCommand_ = false;
	setWindowModified(false);
	if (!timer_) stream_->start();
	assignADPCMSamples();
}

void MainWindow::on_actionComments_triggered()
{
	auto comment = bt_->getModuleComment();
	CommentEditDialog diag(QString::fromUtf8(comment.c_str(), static_cast<int>(comment.length())));
	if (diag.exec() == QDialog::Accepted) {
		bt_->setModuleComment(diag.getComment().toUtf8().toStdString());
		setModifiedTrue();
	}
}

bool MainWindow::on_actionSave_triggered()
{
	auto path = QString::fromStdString(bt_->getModulePath());
	if (!path.isEmpty() && QFileInfo::exists(path) && QFileInfo(path).isFile()) {
		if (!backupModule(path)) return false;

		try {
			BinaryContainer container;
			bt_->saveModule(container);

			QFile fp(path);
			if (!fp.open(QIODevice::WriteOnly)) throw FileOutputError(FileIO::FileType::Mod);
			fp.write(container.getPointer(), container.size());
			fp.close();

			isModifiedForNotCommand_ = false;
			isSavedModBefore_ = true;
			setWindowModified(false);
			setWindowTitle();
			return true;
		}
		catch (FileIOError& e) {
			showFileIOErrorDialog(e);
			return false;
		}
		catch (std::exception& e) {
			showFileIOErrorDialog(FileOutputError(FileIO::FileType::Mod), "\n" + QString(e.what()));
			return false;
		}
	}
	else {
		return on_actionSave_As_triggered();
	}
}

bool MainWindow::on_actionSave_As_triggered()
{
	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString file = QFileDialog::getSaveFileName(
					   this, tr("Save module"),
					   QString("%1/%2.btm").arg(dir.isEmpty() ? "." : dir, getModuleFileBaseName()),
					   tr("BambooTracker module (*.btm)"));
	if (file.isNull()) return false;
	if (!file.endsWith(".btm")) file += ".btm";	// For linux

	if (QFile::exists(file)) {	// Backup if the module already exists
		if (!backupModule(file)) return false;
	}

	bt_->setModulePath(file.toStdString());
	try {
		BinaryContainer container;
		bt_->saveModule(container);

		QFile fp(file);
		if (!fp.open(QIODevice::WriteOnly)) throw FileOutputError(FileIO::FileType::Mod);
		fp.write(container.getPointer(), container.size());
		fp.close();

		isModifiedForNotCommand_ = false;
		isSavedModBefore_ = true;
		setWindowModified(false);
		setWindowTitle();
		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
		changeFileHistory(file);
		return true;
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
		return false;
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileOutputError(FileIO::FileType::Mod), "\n" + QString(e.what()));
		return false;
	}
}

void MainWindow::on_actionOpen_triggered()
{
	if (isWindowModified()) {
		auto modTitleStd = bt_->getModuleTitle();
		QString modTitle = QString::fromUtf8(modTitleStd.c_str(), static_cast<int>(modTitleStd.length()));
		if (modTitle.isEmpty()) modTitle = tr("Untitled");
		QMessageBox dialog(QMessageBox::Warning,
						   "BambooTracker",
						   tr("Save changes to %1?").arg(modTitle),
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

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString file = QFileDialog::getOpenFileName(this, tr("Open module"), (dir.isEmpty() ? "./" : dir),
												tr("BambooTracker module (*.btm)"));
	if (file.isNull()) return;

	bt_->stopPlaySong();
	lockWidgets(false);

	openModule(file);
}

void MainWindow::on_actionLoad_From_File_triggered()
{
	loadInstrument();
}

void MainWindow::on_actionSave_To_File_triggered()
{
	saveInstrument();
}

void MainWindow::on_actionImport_From_Bank_File_triggered()
{
	importInstrumentsFromBank();
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
	if (showUndoResetWarningDialog(tr("Do you want to remove all unused instruments?"))) {
		bt_->stopPlaySong();
		lockWidgets(false);

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
	if (showUndoResetWarningDialog(tr("Do you want to remove all unused patterns?"))) {
		bt_->stopPlaySong();
		lockWidgets(false);

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

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString path = QFileDialog::getSaveFileName(
					   this, tr("Export to WAV"),
					   QString("%1/%2.wav").arg(dir.isEmpty() ? "." : dir, getModuleFileBaseName()),
					   tr("WAV signed 16-bit PCM (*.wav)"));
	if (path.isNull()) return;
	if (!path.endsWith(".wav")) path += ".wav";	// For linux

	QProgressDialog progress(
				tr("Export to WAV"),
				tr("Cancel"),
				0,
				static_cast<int>(bt_->getAllStepCount(bt_->getCurrentSongNumber(), diag.getLoopCount())) + 3
				);
	progress.setValue(0);
	progress.setWindowFlags(progress.windowFlags()
							& ~Qt::WindowContextHelpButtonHint
							& ~Qt::WindowCloseButtonHint);
	progress.show();

	bt_->stopPlaySong();
	lockWidgets(false);
	stream_->stop();

	try {
		WavContainer container(0, static_cast<uint32_t>(diag.getSampleRate()));
		auto bar = [&progress]() -> bool {
				   QApplication::processEvents();
				   progress.setValue(progress.value() + 1);
				   return progress.wasCanceled();
	};

		bool res = bt_->exportToWav(container, diag.getLoopCount(), bar);
		if (res) {
			QFile fp(path);
			if (!fp.open(QIODevice::WriteOnly)) throw FileOutputError(FileIO::FileType::WAV);
			BinaryContainer bc = container.createWavBinary();
			fp.write(bc.getPointer(), bc.size());
			fp.close();
			bar();

			config_.lock()->setWorkingDirectory(QFileInfo(path).dir().path().toStdString());
		}
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileOutputError(FileIO::FileType::WAV), "\n" + QString(e.what()));
	}

	stream_->start();
}

void MainWindow::on_actionVGM_triggered()
{
	VgmExportSettingsDialog diag;
	if (diag.exec() != QDialog::Accepted) return;
	GD3Tag tag = diag.getGD3Tag();

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString path = QFileDialog::getSaveFileName(
					   this, tr("Export to VGM"),
					   QString("%1/%2.vgm").arg(dir.isEmpty() ? "." : dir, getModuleFileBaseName()),
					   tr("VGM file (*.vgm)"));
	if (path.isNull()) return;
	if (!path.endsWith(".vgm")) path += ".vgm";	// For linux

	QProgressDialog progress(
				tr("Export to VGM"),
				tr("Cancel"),
				0,
				static_cast<int>(bt_->getAllStepCount(bt_->getCurrentSongNumber(), 1)) + 3
				);
	progress.setValue(0);
	progress.setWindowFlags(progress.windowFlags()
							& ~Qt::WindowContextHelpButtonHint
							& ~Qt::WindowCloseButtonHint);
	progress.show();

	bt_->stopPlaySong();
	lockWidgets(false);
	stream_->stop();

	try {
		BinaryContainer container;
		auto bar = [&progress]() -> bool {
				   QApplication::processEvents();
				   progress.setValue(progress.value() + 1);
				   return progress.wasCanceled();
	};

		bool res = bt_->exportToVgm(container, diag.getExportTarget(), diag.enabledGD3(), tag, bar);
		if (res) {
			QFile fp(path);
			if (!fp.open(QIODevice::WriteOnly)) throw FileOutputError(FileIO::FileType::VGM);
			fp.write(container.getPointer(), container.size());
			fp.close();
			bar();

			config_.lock()->setWorkingDirectory(QFileInfo(path).dir().path().toStdString());
		}
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileOutputError(FileIO::FileType::VGM), "\n" + QString(e.what()));
	}

	stream_->start();
}

void MainWindow::on_actionS98_triggered()
{
	S98ExportSettingsDialog diag;
	if (diag.exec() != QDialog::Accepted) return;
	S98Tag tag = diag.getS98Tag();

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString path = QFileDialog::getSaveFileName(
					   this, tr("Export to S98"),
					   QString("%1/%2.s98").arg(dir.isEmpty() ? "." : dir, getModuleFileBaseName()),
					   tr("S98 file (*.s98)"));
	if (path.isNull()) return;
	if (!path.endsWith(".s98")) path += ".s98";	// For linux

	QProgressDialog progress(
				tr("Export to S98"),
				tr("Cancel"),
				0,
				static_cast<int>(bt_->getAllStepCount(bt_->getCurrentSongNumber(), 1)) + 3
				);
	progress.setValue(0);
	progress.setWindowFlags(progress.windowFlags()
							& ~Qt::WindowContextHelpButtonHint
							& ~Qt::WindowCloseButtonHint);
	progress.show();

	bt_->stopPlaySong();
	lockWidgets(false);
	stream_->stop();

	try {
		BinaryContainer container;
		auto bar = [&progress]() -> bool {
				   QApplication::processEvents();
				   progress.setValue(progress.value() + 1);
				   return progress.wasCanceled();
	};

		bool res = bt_->exportToS98(container, diag.getExportTarget(), diag.enabledTag(),
									tag, diag.getResolution(), bar);
		if (res) {
			QFile fp(path);
			if (!fp.open(QIODevice::WriteOnly)) throw FileOutputError(FileIO::FileType::S98);
			fp.write(container.getPointer(), container.size());
			fp.close();
			bar();

			config_.lock()->setWorkingDirectory(QFileInfo(path).dir().path().toStdString());
		}
	}
	catch (FileIOError& e) {
		showFileIOErrorDialog(e);
	}
	catch (std::exception& e) {
		showFileIOErrorDialog(FileOutputError(FileIO::FileType::S98), "\n" + QString(e.what()));
	}

	stream_->start();
}

void MainWindow::on_actionMix_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onPasteMixPressed();
}

void MainWindow::on_actionOverwrite_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onPasteOverwritePressed();
}

void MainWindow::onNewTickSignaledRealChip()
{
	onNewTickSignaled(bt_->streamCountUp());
}

void MainWindow::onNewTickSignaled(int state)
{
	if (!state) {	// New step
		int order = bt_->getPlayingOrderNumber();
		if (order > -1) {	// Playing
			if (isVisible() && !isMinimized()) {
				ui->orderList->updatePositionByOrderUpdate(firstViewUpdateRequest_);
				ui->patternEditor->updatePositionByStepUpdate(firstViewUpdateRequest_);
				firstViewUpdateRequest_ = false;
			}

			int width, base;
			if (config_.lock()->getShowRowNumberInHex()) {
				width = 2;
				base = 16;
			}
			else {
				width = 3;
				base = 10;
			}
			statusPlayPos_->setText(
						QString("%1/%2")
						.arg(order, width, base, QChar('0'))
						.arg(bt_->getPlayingStepNumber(), width, base, QChar('0')).toUpper());
		}
	}
	else if (state == -1) {
		if (hasLockedWigets_) lockWidgets(false);
	}

	// Update BPM status
	if (bt_->getStreamGrooveEnabled()) {
		statusBpm_->setText(tr("Groove"));
	}
	else {
		// BPM = tempo * 6 / speed * 4 / 1st highlight
		double bpm = 24.0 * bt_->getStreamTempo() / bt_->getStreamSpeed() / highlight1_->value();
		statusBpm_->setText(QString::number(bpm, 'f', 2) + QString(" BPM"));
	}
}

void MainWindow::on_actionClear_triggered()
{
	fileHistory_->clearHistory();
	for (int i = ui->menu_Recent_Files->actions().count() - 1; 1 < i; --i)
		ui->menu_Recent_Files->removeAction(ui->menu_Recent_Files->actions().at(i));
}

void MainWindow::on_keyRepeatCheckBox_stateChanged(int arg1)
{
	config_.lock()->setKeyRepetition(arg1 == Qt::Checked);
}

void MainWindow::updateVisuals()
{
	int16_t wave[2 * OPNAController::OUTPUT_HISTORY_SIZE];
	bt_->getOutputHistory(wave);

	ui->waveVisual->setStereoSamples(wave, OPNAController::OUTPUT_HISTORY_SIZE);
}

void MainWindow::on_action_Effect_List_triggered()
{
	if (effListDiag_->isVisible()) effListDiag_->activateWindow();
	else effListDiag_->show();
}

void MainWindow::on_actionShortcuts_triggered()
{
	if (shortcutsDiag_->isVisible()) shortcutsDiag_->activateWindow();
	else shortcutsDiag_->show();
}

void MainWindow::on_actionExport_To_Bank_File_triggered()
{
	exportInstrumentsToBank();
}

void MainWindow::on_actionRemove_Duplicate_Instruments_triggered()
{
	if (showUndoResetWarningDialog(tr("Do you want to remove all duplicate instruments?"))) {
		bt_->stopPlaySong();
		lockWidgets(false);

		std::vector<std::vector<int>> duplicates = bt_->checkDuplicateInstruments();
		auto list = ui->instrumentListWidget;
		for (auto& group : duplicates) {
			for (size_t i = 1; i < group.size(); ++i) {
				for (int j = 0; j < list->count(); ++j) {
					if (list->item(j)->data(Qt::UserRole).toInt() == group[i])
						removeInstrument(j);
				}
			}
		}
		bt_->replaceDuplicateInstrumentsInPatterns(duplicates);
		bt_->clearUnusedInstrumentProperties();
		bt_->clearCommandHistory();
		comStack_->clear();
		ui->patternEditor->onDuplicateInstrumentsRemoved();
		setModifiedTrue();
	}
}

void MainWindow::on_actionRename_Instrument_triggered()
{
	renameInstrument();
}

void MainWindow::on_action_Bookmark_Manager_triggered()
{
	if (bmManForm_->isVisible()) bmManForm_->activateWindow();
	else bmManForm_->show();
}

void MainWindow::on_actionFine_Decrease_Values_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onChangeValuesPressed(false, false);
}

void MainWindow::on_actionFine_Increase_Values_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onChangeValuesPressed(false, true);
}

void MainWindow::on_actionCoarse_D_ecrease_Values_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onChangeValuesPressed(true, false);
}

void MainWindow::on_actionCoarse_I_ncrease_Values_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onChangeValuesPressed(true, true);
}

void MainWindow::on_action_Toggle_Bookmark_triggered()
{
	bmManForm_->onBookmarkToggleRequested(bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());
}

void MainWindow::on_action_Next_Bookmark_triggered()
{
	bmManForm_->onBookmarkJumpRequested(true, bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());
}

void MainWindow::on_action_Previous_Bookmark_triggered()
{
	bmManForm_->onBookmarkJumpRequested(false, bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());
}

void MainWindow::on_action_Instrument_Mask_triggered()
{
	config_.lock()->setInstrumentMask(ui->action_Instrument_Mask->isChecked());
}

void MainWindow::on_action_Volume_Mask_triggered()
{
	config_.lock()->setVolumeMask(ui->action_Volume_Mask->isChecked());
}

void MainWindow::on_actionSet_Ro_w_Marker_triggered()
{
	bt_->setMarker(bt_->getCurrentOrderNumber(), bt_->getCurrentStepNumber());
	ui->patternEditor->changeMarker();
}

void MainWindow::on_actionPlay_From_Marker_triggered()
{
	startPlayFromMarker();
}

void MainWindow::on_action_Go_To_triggered()
{
	GoToDialog diag(bt_);
	if (diag.exec() == QDialog::Accepted) {
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
			bt_->setCurrentOrderNumber(diag.getOrder());
			bt_->setCurrentStepNumber(diag.getStep());
			bt_->setCurrentTrack(diag.getTrack());
			ui->orderList->updatePositionByPositionJump(true);
			ui->patternEditor->updatepositionByPositionJump(true);
		}
	}
}

void MainWindow::on_actionRemove_Unused_ADPCM_Samples_triggered()
{
	if (showUndoResetWarningDialog(tr("Do you want to remove all unused ADPCM samples?"))) {
		bt_->stopPlaySong();
		lockWidgets(false);

		bt_->clearUnusedADPCMSamples();
		assignADPCMSamples();
		bt_->clearCommandHistory();
		comStack_->clear();
		setModifiedTrue();
	}
}

void MainWindow::on_action_Status_Bar_triggered()
{
	ui->statusBar->setVisible(ui->action_Status_Bar->isChecked());
}

void MainWindow::on_action_Toolbar_triggered()
{
	bool visible = ui->action_Toolbar->isChecked();
	ui->mainToolBar->setVisible(visible);
	ui->subToolBar->setVisible(visible);
}
