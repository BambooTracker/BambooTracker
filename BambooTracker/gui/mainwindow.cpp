/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <algorithm>
#include <unordered_map>
#include <array>
#include <numeric>
#include <QString>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMimeData>
#include <QProgressDialog>
#include <QRect>
#include <QMetaMethod>
#include <QScreen>
#include <QComboBox>
#include <QToolButton>
#include <QSignalBlocker>
#include <QTextCodec>
#include "jamming.hpp"
#include "song.hpp"
#include "track.hpp"
#include "instrument.hpp"
#include "bank.hpp"
#include "io/io_file_type.hpp"
#include "io/module_io.hpp"
#include "io/instrument_io.hpp"
#include "io/bank_io.hpp"
#include "io/binary_container.hpp"
#include "io/wav_container.hpp"
#include "version.hpp"
#include "gui/command/instrument/instrument_commands_qt.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "gui/instrument_editor/instrument_editor_adpcm_form.hpp"
#include "gui/instrument_editor/instrument_editor_drumkit_form.hpp"
#include "gui/module_properties_dialog.hpp"
#include "gui/groove_settings_dialog.hpp"
#include "gui/configuration_dialog.hpp"
#include "gui/wave_export_settings_dialog.hpp"
#include "gui/vgm_export_settings_dialog.hpp"
#include "gui/instrument_selection_dialog.hpp"
#include "gui/s98_export_settings_dialog.hpp"
#include "gui/configuration_handler.hpp"
#include "gui/jam_layout.hpp"
#include "chip/scci/scci_wrapper.hpp"
#include "chip/c86ctl/c86ctl_wrapper.hpp"
#include "midi/midi.hpp"
#include "audio/audio_stream_rtaudio.hpp"
#include "enum_hash.hpp"
#include "gui/go_to_dialog.hpp"
#include "gui/transpose_song_dialog.hpp"
#include "gui/swap_tracks_dialog.hpp"
#include "gui/hide_tracks_dialog.hpp"
#include "gui/file_io_error_message_box.hpp"
#include "gui/note_name_manager.hpp"
#include "gui/gui_utils.hpp"
#include "utils.hpp"

namespace
{
const std::unordered_map<Configuration::ToolbarPosition, Qt::ToolBarArea> TB_POS_ = {
	{ Configuration::ToolbarPosition::TopPosition, Qt::TopToolBarArea },
	{ Configuration::ToolbarPosition::BottomPosition, Qt::BottomToolBarArea },
	{ Configuration::ToolbarPosition::LeftPosition, Qt::LeftToolBarArea },
	{ Configuration::ToolbarPosition::RightPosition, Qt::RightToolBarArea }
};

constexpr int STATUS_DISPLAY_TIMEOUT = 0;

class ModuleSaveCheckDialog : public QMessageBox
{
	Q_OBJECT
public:
	explicit ModuleSaveCheckDialog(const std::string& name, QWidget* parent = nullptr)
		: QMessageBox(QMessageBox::Warning, "BambooTracker", "",
					  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, parent)
	{
		setText(tr("Save changes to %1?").arg(name.empty() ? tr("Untitled") : gui_utils::utf8ToQString(name)));
	}
};
}

MainWindow::MainWindow(std::weak_ptr<Configuration> config, QString filePath, bool isFirstLaunch, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	config_(config),
	palette_(std::make_shared<ColorPalette>()),
	bt_(std::make_shared<BambooTracker>(config)),
	comStack_(std::make_shared<QUndoStack>(this)),
	fileHistory_(std::make_shared<FileHistory>()),
	scciDll_(std::make_shared<QLibrary>("scci")),
	c86ctlDll_(std::make_shared<QLibrary>("c86ctl")),
	instForms_(std::make_shared<InstrumentFormManager>()),
	renamingInstItem_(nullptr),
	renamingInstEdit_(nullptr),
	isModifiedForNotCommand_(false),
	hasLockedWigets_(false),
	isEditedPattern_(true),
	isEditedOrder_(false),
	isEditedInstList_(false),
	isSelectedPattern_(false),
	isSelectedOrder_(false),
	hasShownOnce_(false),
	firstViewUpdateRequest_(false),
	octUpSc_(nullptr),
	octDownSc_(nullptr),
	focusPtnSc_(this),
	focusOdrSc_(this),
	focusInstSc_(this),
	playAndStopSc_(nullptr),
	playStepSc_(nullptr),
	goPrevOdrSc_(nullptr),
	goNextOdrSc_(nullptr),
	prevInstSc_(nullptr),
	nextInstSc_(nullptr),
	incPtnSizeSc_(nullptr),
	decPtnSizeSc_(nullptr),
	incEditStepSc_(nullptr),
	decEditStepSc_(nullptr),
	prevSongSc_(nullptr),
	nextSongSc_(nullptr),
	jamVolUpSc_(nullptr),
	jamVolDownSc_(nullptr),
	bankJamMidiCtrl_(false)
{
	ui->setupUi(this);

	if (config.lock()->getMainWindowX() == -1) {	// When unset
		QRect rec = geometry();
		rec.moveCenter(QGuiApplication::screens().at(0)->geometry().center());
		setGeometry(rec);
		config.lock()->setMainWindowX(x());
		config.lock()->setMainWindowY(y());
	}
	else {
		move(config.lock()->getMainWindowX(), config.lock()->getMainWindowY());
	}
	if (isFirstLaunch) {
		const QHash<QString, NoteNotationSystem> defNoteSysMap = {
			{ "English", NoteNotationSystem::ENGLISH },
			{ "German", NoteNotationSystem::GERMAN }
		};
		//: Set the name of suitable notation system (English or German)
		config.lock()->setNotationSystem(defNoteSysMap[tr("English", "Default notation system")]);
	}
	resize(config.lock()->getMainWindowWidth(), config.lock()->getMainWindowHeight());
	if (config.lock()->getMainWindowMaximized()) showMaximized();
	ui->action_Status_Bar->setChecked(config.lock()->getVisibleStatusBar());
	ui->statusBar->setVisible(config.lock()->getVisibleStatusBar());
	ui->actionFollow_Mode->setChecked(config.lock()->getFollowMode());
	ui->action_Instrument_Mask->setChecked(config.lock()->getInstrumentMask());
	ui->action_Volume_Mask->setChecked(config.lock()->getVolumeMask());
	ui->action_Wave_View->setChecked(config.lock()->getVisibleWaveView());
	ui->waveVisual->setVisible(config.lock()->getVisibleWaveView());
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
	NoteNameManager::getManager().setNotationSystem(config.lock()->getNotationSystem());
	ui->patternEditor->setConfiguration(config_.lock());
	ui->orderList->setConfiguration(config_.lock());
	updateFonts();
	ui->orderList->setHorizontalScrollMode(config.lock()->getMoveCursorByHorizontalScroll(), false);
	ui->patternEditor->setHorizontalScrollMode(config.lock()->getMoveCursorByHorizontalScroll(), false);
	ui->patternEditor->setCore(bt_);
	ui->orderList->setCore(bt_);
	io::loadPalette(palette_.get());
	ui->patternEditor->setColorPallete(palette_);
	ui->orderList->setColorPallete(palette_);
	updateInstrumentListColors();
	ui->waveVisual->setColorPalette(palette_);

	/* Command stack */
	QObject::connect(comStack_.get(), &QUndoStack::indexChanged,
					 this, [&](int idx) {
		setWindowModified(idx || isModifiedForNotCommand_);
		ui->actionUndo->setEnabled(comStack_->canUndo());
		ui->actionRedo->setEnabled(comStack_->canRedo());
	});

	/* File history */
	io::loadFileHistory(fileHistory_);
	for (size_t i = 0; i < fileHistory_->size(); ++i) {
		// Leave Before Qt5.7.0 style due to windows xp
		QAction* action = ui->menu_Recent_Files->addAction(QString("&%1 %2").arg(i + 1).arg(fileHistory_->at(i)));
		action->setData(fileHistory_->at(i));
	}
	QObject::connect(ui->menu_Recent_Files, &QMenu::triggered, this, [&](QAction* action) {
		if (action != ui->actionClear) {
			if (isWindowModified()) {
				switch (ModuleSaveCheckDialog(bt_->getModuleTitle(), this).exec()) {
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

	/* Menus */
	pasteModeGroup_ = std::make_unique<QActionGroup>(this);
	pasteModeGroup_->addAction(ui->action_Cursor);
	QObject::connect(ui->action_Cursor, &QAction::triggered, this, [&](bool checked) {
		if (checked) config_.lock()->setPasteMode(Configuration::PasteMode::Cursor);
	});
	pasteModeGroup_->addAction(ui->action_Selection);
	QObject::connect(ui->action_Selection, &QAction::triggered, this, [&](bool checked) {
		if (checked) config_.lock()->setPasteMode(Configuration::PasteMode::Selection);
	});
	pasteModeGroup_->addAction(ui->action_Fill);
	QObject::connect(ui->action_Fill, &QAction::triggered, this, [&](bool checked) {
		if (checked) config_.lock()->setPasteMode(Configuration::PasteMode::Fill);
	});
	switch (config.lock()->getPasteMode()) {
	case Configuration::PasteMode::Cursor:		ui->action_Cursor->setChecked(true);	break;
	case Configuration::PasteMode::Selection:	ui->action_Selection->setChecked(true);	break;
	case Configuration::PasteMode::Fill:		ui->action_Fill->setChecked(true);		break;
	}

	/* Tool bars */
	auto octLab = new QLabel(tr("Octave"));
	octLab->setMargin(6);
	ui->subToolBar->addWidget(octLab);
	octave_ = new QSpinBox();
	octave_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	octave_->setMaximumWidth(80);
	octave_->setMinimum(0);
	octave_->setMaximum(7);
	octave_->setValue(bt_->getCurrentOctave());
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(octave_, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int octave) {
		bt_->setCurrentOctave(octave);
		statusOctave_->setText(tr("Octave: %1").arg(octave));
	});
	ui->subToolBar->addWidget(octave_);
	auto volLab = new QLabel(tr("Volume"));
	volLab->setMargin(6);
	ui->subToolBar->addWidget(volLab);
	volume_ = new QSpinBox();
	volume_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	volume_->setMaximumWidth(100);
	volume_->setMinimum(0);
	volume_->setMaximum(255);
	volume_->setDisplayIntegerBase(16);
	volume_->setValue(bt_->getCurrentVolume());
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(volume_, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
					 this, [&](int volume) { bt_->setCurrentVolume(volume); });
	ui->subToolBar->addWidget(volume_);
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
	ui->subToolBar->addSeparator();
	auto& mainTbConfig = config.lock()->getMainToolbarConfiguration();
	if (mainTbConfig.getPosition() == Configuration::ToolbarPosition::FloatPorition) {
		ui->mainToolBar->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
		ui->mainToolBar->move(mainTbConfig.getX(), mainTbConfig.getY());
	}
	else {
		addToolBar(TB_POS_.at(mainTbConfig.getPosition()), ui->mainToolBar);
	}
	auto& subTbConfig = config.lock()->getSubToolbarConfiguration();
	if (subTbConfig.getPosition() == Configuration::ToolbarPosition::FloatPorition) {
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
		if (!tickTimerForRealChip_) stream_->stop();
		bt_->setCurrentSongNumber(num);
		loadSong();
		if (!tickTimerForRealChip_) stream_->start();
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
	auto instToolBar = new QToolBar();
	instToolBar->setIconSize(QSize(16, 16));
	auto addMenu = new QMenu();
	addMenu->addActions({ ui->actionNew_Instrument, ui->actionNew_Drumki_t });
	auto addTB = new QToolButton();
	addTB->setPopupMode(QToolButton::MenuButtonPopup);
	addTB->setMenu(addMenu);
	addTB->setDefaultAction(ui->actionNew_Instrument);
	instToolBar->addWidget(addTB);
	instToolBar->addActions({ ui->actionRemove_Instrument, ui->actionClone_Instrument });
	instToolBar->addSeparator();
	instToolBar->addActions({ ui->actionLoad_From_File, ui->actionSave_To_File });
	instToolBar->addSeparator();
	instToolBar->addAction(ui->actionEdit);
	instToolBar->addSeparator();
	instToolBar->addAction(ui->actionRename_Instrument);
	ui->instrumentListGroupBox->layout()->addWidget(instToolBar);
	ui->instrumentList->installEventFilter(this);
	QObject::connect(ui->instrumentList, &DropDetectListWidget::itemDroppedAtItemIndex,
					 this, &MainWindow::swapInstruments);

	/* Pattern editor */
	ui->patternEditor->setCommandStack(comStack_);
	ui->patternEditor->installEventFilter(this);
	QObject::connect(ui->patternEditor, &PatternEditor::currentTrackChanged,
					 ui->orderList, &OrderListEditor::onPatternEditorCurrentTrackChanged);
	QObject::connect(ui->patternEditor, &PatternEditor::currentOrderChanged,
					 ui->orderList, &OrderListEditor::onPatternEditorCurrentOrderChanged);
	QObject::connect(ui->patternEditor, &PatternEditor::focusIn,
					 this, &MainWindow::updateMenuByPattern);
	QObject::connect(ui->patternEditor, &PatternEditor::selected,
					 this, &MainWindow::updateMenuByPatternSelection);
	QObject::connect(ui->patternEditor, &PatternEditor::instrumentEntered,
					 this, [&](int num) {
		auto list = ui->instrumentList;
		if (num != -1) {
			for (int i = 0; i < list->count(); ++i) {
				if (list->item(i)->data(Qt::UserRole).toInt() == num) {
					list->setCurrentRow(i);
					return ;
				}
			}
		}
	});
	QObject::connect(ui->patternEditor, &PatternEditor::volumeEntered,
					 this, [&](int volume) { volume_->setValue(volume); });
	QObject::connect(ui->patternEditor, &PatternEditor::effectEntered,
					 this, [&](QString text) { ui->statusBar->showMessage(text, STATUS_DISPLAY_TIMEOUT); });
	QObject::connect(ui->patternEditor, &PatternEditor::currentTrackChanged,
					 this, &MainWindow::onCurrentTrackChanged);

	/* Order List */
	ui->orderList->setCommandStack(comStack_);
	ui->orderList->installEventFilter(this);
	QObject::connect(ui->orderList, &OrderListEditor::currentTrackChanged,
					 ui->patternEditor, &PatternEditor::onOrderListCurrentTrackChanged);
	QObject::connect(ui->orderList, &OrderListEditor::currentOrderChanged,
					 ui->patternEditor, &PatternEditor::onOrderListCrrentOrderChanged);
	QObject::connect(ui->orderList, &OrderListEditor::orderEdited,
					 ui->patternEditor, &PatternEditor::onOrderListEdited);
	QObject::connect(ui->orderList, &OrderListEditor::focusIn,
					 this, &MainWindow::updateMenuByOrder);
	QObject::connect(ui->orderList, &OrderListEditor::selected,
					 this, &MainWindow::updateMenuByOrderSelection);
	QObject::connect(ui->orderList, &OrderListEditor::currentTrackChanged,
					 this, &MainWindow::onCurrentTrackChanged);

	/* Wave view */
	visualTimer_.reset(new QTimer);
	QObject::connect(visualTimer_.get(), &QTimer::timeout, this, &MainWindow::updateVisuals);
	if (config.lock()->getVisibleWaveView())
		visualTimer_->start(static_cast<int>(std::round(1000. / config.lock()->getWaveViewFrameRate())));

	/* Status bar */
	statusStyle_ = new QLabel();
	statusStyle_->setAlignment(Qt::AlignRight);
	statusInst_ = new QLabel();
	statusInst_->setFixedWidth(110);
	statusOctave_ = new QLabel();
	statusOctave_->setFixedWidth(90);
	statusIntr_ = new QLabel();
	statusIntr_->setFixedWidth(50);
	statusMixer_ = new QLabel();
	statusMixer_->setFixedWidth(200);
	statusBpm_ = new QLabel();
	statusBpm_->setFixedWidth(90);
	statusPlayPos_ = new QLabel();
	statusPlayPos_->setFixedWidth(40);
	ui->statusBar->addPermanentWidget(statusStyle_);
	ui->statusBar->addPermanentWidget(statusInst_);
	ui->statusBar->addPermanentWidget(statusOctave_);
	ui->statusBar->addPermanentWidget(statusIntr_);
	ui->statusBar->addPermanentWidget(statusMixer_);
	ui->statusBar->addPermanentWidget(statusBpm_);
	ui->statusBar->addPermanentWidget(statusPlayPos_);
	statusOctave_->setText(tr("Octave: %1").arg(bt_->getCurrentOctave()));
	statusIntr_->setText(QString::number(bt_->getModuleTickFrequency()) + QString("Hz"));
	ui->statusBar->showMessage(tr("Welcome to BambooTracker v%1!").arg(QString::fromStdString(Version::ofApplicationInString())),
							   STATUS_DISPLAY_TIMEOUT);

	/* Bookmark */
	bmManForm_ = std::make_unique<BookmarkManagerForm>(bt_, config_.lock()->getShowRowNumberInHex());
	auto posJumpFunc = [&](int order, int step) {
		if (bt_->isPlaySong()) return;
		int song = bt_->getCurrentSongNumber();
		if (static_cast<int>(bt_->getOrderSize(song)) <= order) return;
		if (static_cast<int>(bt_->getPatternSizeFromOrderNumber(song, order)) <= step) return;
		bt_->setCurrentOrderNumber(order);
		bt_->setCurrentStepNumber(step);
		ui->orderList->updatePositionByPositionJump();
		ui->patternEditor->updatepositionByPositionJump();
		activateWindow();
	};
	QObject::connect(bmManForm_.get(), &BookmarkManagerForm::positionJumpRequested, this, posJumpFunc);
	QObject::connect(bmManForm_.get(), &BookmarkManagerForm::modified, this, &MainWindow::setModifiedTrue);

	/* Key signature */
	ksManForm_ = std::make_unique<KeySignatureManagerForm>(bt_, config_.lock()->getShowRowNumberInHex());
	QObject::connect(ksManForm_.get(), &KeySignatureManagerForm::positionJumpRequested, this, posJumpFunc);
	QObject::connect(ksManForm_.get(), &KeySignatureManagerForm::modified, this, [&] {
		ui->patternEditor->onPatternDataGlobalChanged();
		setModifiedTrue();
	});

	// Shortcuts
	auto linkShortcut = [&](QAction* ptr) {
		ptr->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		ui->orderList->addAction(ptr);
		ui->patternEditor->addAction(ptr);
	};

	linkShortcut(&octUpSc_);
	QObject::connect(&octUpSc_, &QAction::triggered, this, [&] { changeOctave(true); });
	linkShortcut(&octDownSc_);
	QObject::connect(&octDownSc_, &QAction::triggered, this, [&] { changeOctave(false); });
	QObject::connect(&focusPtnSc_, &QShortcut::activated, this, [&] { ui->patternEditor->setFocus(); });
	QObject::connect(&focusOdrSc_, &QShortcut::activated, this, [&] { ui->orderList->setFocus(); });
	QObject::connect(&focusInstSc_, &QShortcut::activated, this, [&] {
		ui->instrumentList->setFocus();
		updateMenuByInstrumentList();
	});
	auto playLinkShortcut = [&](QAction* ptr) {
		ptr->setShortcutContext(Qt::WidgetShortcut);
		ui->instrumentList->addAction(ptr);
		ui->orderList->addActionToPanel(ptr);
		ui->patternEditor->addActionToPanel(ptr);
	};
	playLinkShortcut(ui->actionPlay);
	playLinkShortcut(&playAndStopSc_);
	QObject::connect(&playAndStopSc_, &QAction::triggered, this, [&] {
		if (bt_->isPlaySong()) stopPlaySong();
		else startPlaySong();
	});
	playLinkShortcut(&playStepSc_);
	QObject::connect(&playStepSc_, &QAction::triggered, this, &MainWindow::playStep);
	playLinkShortcut(ui->actionPlay_From_Start);
	playLinkShortcut(ui->actionPlay_Pattern);
	playLinkShortcut(ui->actionPlay_From_Cursor);
	playLinkShortcut(ui->actionPlay_From_Marker);
	playLinkShortcut(ui->actionStop);
	instAddSc_ = std::make_unique<QShortcut>(Qt::Key_Insert, ui->instrumentList,
											 nullptr, nullptr, Qt::WidgetShortcut);
	QObject::connect(instAddSc_.get(), &QShortcut::activated, this, &MainWindow::addInstrument);
	linkShortcut(&goPrevOdrSc_);
	QObject::connect(&goPrevOdrSc_, &QAction::triggered, this, [&] {
		ui->orderList->onGoOrderRequested(false);
	});
	linkShortcut(&goNextOdrSc_);
	QObject::connect(&goNextOdrSc_, &QAction::triggered, this, [&] {
		ui->orderList->onGoOrderRequested(true);
	});
	linkShortcut(&prevInstSc_);
	QObject::connect(&prevInstSc_, &QAction::triggered, this, [&] {
		if (ui->instrumentList->count()) {
			int row = ui->instrumentList->currentRow();
			if (row == -1) ui->instrumentList->setCurrentRow(0);
			else if (row > 0) ui->instrumentList->setCurrentRow(row - 1);
		}
	});
	linkShortcut(&nextInstSc_);
	QObject::connect(&nextInstSc_, &QAction::triggered, this, [&] {
		int cnt = ui->instrumentList->count();
		if (cnt) {
			int row = ui->instrumentList->currentRow();
			if (row == -1) ui->instrumentList->setCurrentRow(cnt - 1);
			else if (row < cnt - 1) ui->instrumentList->setCurrentRow(row + 1);
		}
	});
	linkShortcut(&incPtnSizeSc_);
	QObject::connect(&incPtnSizeSc_, &QAction::triggered, this, [&] {
		ui->patternSizeSpinBox->setValue(ui->patternSizeSpinBox->value() + 1);
	});
	linkShortcut(&decPtnSizeSc_);
	QObject::connect(&decPtnSizeSc_, &QAction::triggered, this, [&] {
		ui->patternSizeSpinBox->setValue(ui->patternSizeSpinBox->value() - 1);
	});
	linkShortcut(&incEditStepSc_);
	QObject::connect(&incEditStepSc_, &QAction::triggered, this, [&] {
		ui->editableStepSpinBox->setValue(ui->editableStepSpinBox->value() + 1);
	});
	linkShortcut(&decEditStepSc_);
	QObject::connect(&decEditStepSc_, &QAction::triggered, this, [&] {
		ui->editableStepSpinBox->setValue(ui->editableStepSpinBox->value() - 1);
	});
	linkShortcut(&prevSongSc_);
	QObject::connect(&prevSongSc_, &QAction::triggered, this, [&] {
		if (ui->songComboBox->isEnabled()) {
			ui->songComboBox->setCurrentIndex(std::max(ui->songComboBox->currentIndex() - 1, 0));
		}
	});
	linkShortcut(&nextSongSc_);
	QObject::connect(&nextSongSc_, &QAction::triggered, this, [&] {
		if (ui->songComboBox->isEnabled()) {
			ui->songComboBox->setCurrentIndex(std::min(ui->songComboBox->currentIndex() + 1,
													   ui->songComboBox->count() - 1));
		}
	});
	linkShortcut(&jamVolUpSc_);
	QObject::connect(&jamVolUpSc_, &QAction::triggered, this, [&] {
		volume_->setValue(volume_->value() + 1);
	});
	linkShortcut(&jamVolDownSc_);
	QObject::connect(&jamVolDownSc_, &QAction::triggered, this, [&] {
		volume_->setValue(volume_->value() - 1);
	});
	setShortcuts();

	/* Clipboard */
	QObject::connect(QApplication::clipboard(), &QClipboard::dataChanged,
					 this, [&]() {
		if (isEditedOrder_) updateMenuByOrder();
		else if (isEditedPattern_) updateMenuByPattern();
	});

	/* Welcome dialog */
	{
		const QString text =
				"<h2 style='text-align: center;'>" + tr("Welcome to BambooTracker!") + "</h2>" +
				"<div style='text-align: center;'>"
				"<img src='qrc:/icon/app_icon' width=128 height=128>"
				"</div>"
				"<h3>" + tr("Don't know where to start?") + "</h3>" +
				"<p>" + tr("Check the demo modules and instruments included "
						   "with your download of BambooTracker.") + "</p>" +
				"<h3>" + tr("Need a list of effects and shortcuts?") + "</h3>" +
				"<p>" + tr("Check the Help menu at the top of the window.") + "</p>" +
				"<h3>" + tr("Still lost?") + "</h3>" +
				"<p>" + tr("The README.md has a link to our Discord server.") + "</p>" +
				"<h3>" + tr("Think you've found a bug? Missing a feature?") + "</h3>" +
				"<p>" + tr("BambooTracker is still in development, bugs and missing "
						   "features are to be expected. So we need your help!") +
				"<ul style='margin-left: 16px; -qt-list-indent: 0;'>" +
				//: %1 is the link to the issue submission page in GitHub.
				"<li>" + tr("Please report any bugs you find and requests and features "
							"you'd like to see on our Discord server or our bug tracker (%1).")
				.arg(R"(<a href="https://github.com/BambooTracker/BambooTracker/issues/new/choose">
					 https://github.com/BambooTracker/BambooTracker/issues/new/choose</a>)") + "</li>" +
				"<li>" + tr("If you're a developer yourself or would like to start being one, "
							"consider contributing to the project yourself. "
							"Any help would be appreciated!") + "</li>" +
				"</ul></p>";
		welcomeDialog_ = std::make_unique<QMessageBox>(QMessageBox::NoIcon, tr("Welcome"), text, QMessageBox::Ok, this);
		welcomeDialog_->setWindowModality(Qt::ApplicationModal);
		welcomeDialog_->setWindowFlags(welcomeDialog_->windowFlags() & ~Qt::WindowContextHelpButtonHint);
		welcomeDialog_->setTextFormat(Qt::RichText);
		if (isFirstLaunch) on_action_Welcome_triggered();
	}

	/* MIDI */
	setMidiConfiguration();
	midiKeyEventMethod_ = metaObject()->indexOfSlot("midiKeyEvent(uchar,uchar,uchar)");
	Q_ASSERT(midiKeyEventMethod_ != -1);
	midiProgramEventMethod_ = metaObject()->indexOfSlot("midiProgramEvent(uchar,uchar)");
	Q_ASSERT(midiProgramEventMethod_ != -1);
	MidiInterface::getInstance().installInputHandler(&midiThreadReceivedEvent, this);

	/* Audio stream */
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
	QString audioApi = gui_utils::utf8ToQString(config.lock()->getSoundAPI());
	if (isFirstLaunch) {
		bool streamState = false;
		QString streamErr;
		for (const QString& audioApi : stream_->getAvailableBackends()) {
			config.lock()->setSoundAPI(audioApi.toUtf8().toStdString());
			QString audioDevice = stream_->getDefaultOutputDevice(audioApi);
			config.lock()->setSoundDevice(audioDevice.toUtf8().toStdString());
			streamState = stream_->initialize(
							  static_cast<uint32_t>(bt_->getStreamRate()),
							  static_cast<uint32_t>(bt_->getStreamDuration()),
							  bt_->getModuleTickFrequency(), audioApi, audioDevice, &streamErr);
			if (streamState) break;
		}
		if (streamState) {
			uint32_t sr = stream_->getStreamRate();
			if (config.lock()->getSampleRate() != sr) {
				showStreamRateWarningDialog(sr);
				bt_->setStreamRate(sr);
			}
		}
		else {
			showStreamFailedDialog(streamErr);
		}
	}
	else {	// Ordinary launch
		bool savedApiExists = false;
		const std::vector<QString> audioApis = stream_->getAvailableBackends();
		for (const QString& api : audioApis) {
			if (api.toUtf8().toStdString() == config.lock()->getSoundAPI()) {
				savedApiExists = true;
				break;
			}
		}
		if (!savedApiExists) {
			audioApi = audioApis.front();
			config.lock()->setSoundAPI(audioApi.toUtf8().toStdString());
		}
		QString audioDevice = gui_utils::utf8ToQString(config.lock()->getSoundDevice());
		bool savedDeviceExists = false;
		for (const QString& device : stream_->getAvailableDevices(audioApi)) {
			if (device.toUtf8().toStdString() == config.lock()->getSoundDevice()) {
				savedDeviceExists = true;
				break;
			}
		}
		if (!savedDeviceExists) {
			audioDevice = stream_->getDefaultOutputDevice(audioApi);
			config.lock()->setSoundDevice(audioDevice.toUtf8().toStdString());
		}
		QString streamErr;
		bool streamState = stream_->initialize(
							   static_cast<uint32_t>(bt_->getStreamRate()),
							   static_cast<uint32_t>(bt_->getStreamDuration()),
							   bt_->getModuleTickFrequency(), audioApi, audioDevice, &streamErr);
		if (streamState) {
			uint32_t sr = stream_->getStreamRate();
			if (config.lock()->getSampleRate() != sr) {
				showStreamRateWarningDialog(sr);
				bt_->setStreamRate(sr);
			}
		}
		else {
			showStreamFailedDialog(streamErr);
		}
	}
	RealChipInterfaceType intf = config.lock()->getRealChipInterface();
	if (intf != RealChipInterfaceType::NONE) {
		tickTimerForRealChip_ = std::make_unique<PreciseTimer>();
		tickTimerForRealChip_->setInterval(1000000 / bt_->getModuleTickFrequency());
		tickEventMethod_ = metaObject()->indexOfSlot("onNewTickSignaledRealChip()");
		Q_ASSERT(tickEventMethod_ != -1);
		tickTimerForRealChip_->setFunction([&]{
			QMetaMethod method = this->metaObject()->method(this->tickEventMethod_);
			method.invoke(this, Qt::QueuedConnection);
		});

		setRealChipInterface(intf);

		tickTimerForRealChip_->start();
	}

	/* Load module */
	if (filePath.isEmpty()) {
		loadModule();
		setInitialSelectedInstrument();
		assignADPCMSamples();
		if (!tickTimerForRealChip_) stream_->start();
	}
	else {
		openModule(filePath);	// If use emulation, stream starts
	}
}

MainWindow::~MainWindow()
{
	MidiInterface::getInstance().uninstallInputHandler(&midiThreadReceivedEvent, this);
	stream_->shutdown();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (auto fmForm = qobject_cast<InstrumentEditorFMForm*>(watched)) {
		// Change current instrument by activating FM editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(fmForm->getInstrumentNumber());
			ui->instrumentList->setCurrentRow(row);
		}
		else if (event->type() == QEvent::Resize) {
			config_.lock()->setInstrumentFMWindowWidth(fmForm->width());
			config_.lock()->setInstrumentFMWindowHeight(fmForm->height());
		}
		return false;
	}

	if (auto ssgForm = qobject_cast<InstrumentEditorSSGForm*>(watched)) {
		// Change current instrument by activating SSG editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(ssgForm->getInstrumentNumber());
			ui->instrumentList->setCurrentRow(row);
		}
		else if (event->type() == QEvent::Resize) {
			config_.lock()->setInstrumentSSGWindowWidth(ssgForm->width());
			config_.lock()->setInstrumentSSGWindowHeight(ssgForm->height());
		}
		return false;
	}

	if (auto adpcmForm = qobject_cast<InstrumentEditorADPCMForm*>(watched)) {
		// Change current instrument by activating ADPCM editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(adpcmForm->getInstrumentNumber());
			ui->instrumentList->setCurrentRow(row);
		}
		else if (event->type() == QEvent::Resize) {
			config_.lock()->setInstrumentADPCMWindowWidth(adpcmForm->width());
			config_.lock()->setInstrumentADPCMWindowHeight(adpcmForm->height());
		}
		return false;
	}

	if (auto kitForm = qobject_cast<InstrumentEditorDrumkitForm*>(watched)) {
		// Change current instrument by activating drumkit editor
		if (event->type() == QEvent::WindowActivate) {
			int row = findRowFromInstrumentList(kitForm->getInstrumentNumber());
			ui->instrumentList->setCurrentRow(row);
		}
		else if (event->type() == QEvent::Resize) {
			config_.lock()->setInstrumentDrumkitWindowWidth(kitForm->width());
			config_.lock()->setInstrumentDrumkitWindowHeight(kitForm->height());
		}
		return false;
	}

	if (watched == ui->instrumentList) {
		if (event->type() == QEvent::FocusIn) updateMenuByInstrumentList();
		return false;
	}

	if (watched == renamingInstEdit_) {
		if (event->type() == QEvent::FocusOut) finishRenamingInstrument();
		return false;
	}

	return false;
}

void MainWindow::showEvent(QShowEvent*)
{
	if (!hasShownOnce_)	{
		int y = config_.lock()->getMainWindowVerticalSplit();
		if (y == -1) {
			config_.lock()->setMainWindowVerticalSplit(ui->splitter->sizes().at(0));
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
			bt_->jamKeyOn(getJamKeyFromLayoutMapping(qtKey, config_),
						  !config_.lock()->getFixJammingVolume());
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
	if (mime->hasUrls()) {
		const auto urls = mime->urls();
		for (auto& url : urls) {
			const std::string ext = QFileInfo(url.toLocalFile()).suffix().toLower().toStdString();
			if (io::ModuleIO::getInstance().testLoadableFormat(ext)
					|| io::BankIO::getInstance().testLoadableFormat(ext)) {
				if (urls.size() == 1) event->acceptProposedAction();
			}
			if (io::InstrumentIO::getInstance().testLoadableFormat(ext)) {
				continue;
			}
			return;
		}
		if (!urls.empty()) event->acceptProposedAction();	// For instruments
	}
}

void MainWindow::dropEvent(QDropEvent* event)
{
	const auto urls = event->mimeData()->urls();
	for (const auto& url : urls) {
		const QString file = url.toLocalFile();
		const std::string ext = QFileInfo(file).suffix().toLower().toStdString();
		if (io::ModuleIO::getInstance().testLoadableFormat(ext)) {
			if (isWindowModified()) {
				switch (ModuleSaveCheckDialog(bt_->getModuleTitle(), this).exec()) {
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
			return;
		}
		else if (io::InstrumentIO::getInstance().testLoadableFormat(ext)) {
			funcLoadInstrument(file);
		}
		if (io::BankIO::getInstance().testLoadableFormat(ext)) {
			funcImportInstrumentsFromBank(file);
			return;
		}
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
		switch (ModuleSaveCheckDialog(bt_->getModuleTitle(), this).exec()) {
		case QMessageBox::Yes:
			if (!on_actionSave_triggered()) {
				event->ignore();
				return;
			}
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
	config_.lock()->setMainWindowVerticalSplit(ui->splitter->sizes().at(0));

	auto& mainTbConfig = config_.lock()->getMainToolbarConfiguration();
	auto& subTbConfig = config_.lock()->getSubToolbarConfiguration();
	auto mainTbArea = toolBarArea(ui->mainToolBar);
	auto subTbArea = toolBarArea(ui->subToolBar);
	if (ui->mainToolBar->isFloating()) {
		mainTbConfig.setPosition(Configuration::ToolbarPosition::FloatPorition);
		mainTbConfig.setX(ui->mainToolBar->x());
		mainTbConfig.setY(ui->mainToolBar->y());
	}
	else {
		mainTbConfig.setPosition(utils::findMapValue(TB_POS_, mainTbArea)->first);
		mainTbConfig.setNumber(0);
		mainTbConfig.setBreakBefore(false);
	}
	if (ui->subToolBar->isFloating()) {
		subTbConfig.setPosition(Configuration::ToolbarPosition::FloatPorition);
		subTbConfig.setX(ui->subToolBar->x());
		subTbConfig.setY(ui->subToolBar->y());
	}
	else {
		subTbConfig.setPosition(utils::findMapValue(TB_POS_, mainTbArea)->first);
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

	io::saveFileHistory(fileHistory_);

	bmManForm_->close();
	ksManForm_->close();

	event->accept();
}

void MainWindow::freezeViews()
{
	ui->orderList->freeze();
	ui->patternEditor->freeze();
}

void MainWindow::unfreezeViews()
{
	ui->orderList->unfreeze();
	ui->patternEditor->unfreeze();
}

void MainWindow::setShortcuts()
{
	auto shortcuts = config_.lock()->getShortcuts();
	octUpSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::OctaveUp)));
	octDownSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::OctaveDown)));
	focusPtnSc_.setKey(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::FocusOnPattern)));
	focusOdrSc_.setKey(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::FocusOnOrder)));
	focusInstSc_.setKey(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::FocusOnInstrument)));
	playAndStopSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PlayAndStop)));
	ui->actionPlay->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::Play)));
	ui->actionPlay_From_Start->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PlayFromStart)));
	ui->actionPlay_Pattern->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PlayPattern)));
	ui->actionPlay_From_Cursor->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PlayFromCursor)));
	ui->actionPlay_From_Marker->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PlayFromMarker)));
	playStepSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PlayStep)));
	ui->actionStop->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::Stop)));
	ui->actionEdit_Mode->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::ToggleEditJam)));
	ui->actionSet_Ro_w_Marker->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::SetMarker)));
	ui->actionMix->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PasteMix)));
	ui->actionOverwrite->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PasteOverwrite)));
	ui->action_Insert->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PasteInsert)));
	ui->actionAll->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::SelectAll)));
	ui->actionNone->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::Deselect)));
	ui->actionRow->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::SelectRow)));
	ui->actionColumn->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::SelectColumn)));
	ui->actionPattern->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::SelectColumn)));
	ui->actionOrder->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::SelectOrder)));
	ui->action_Go_To->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::GoToStep)));
	ui->actionToggle_Track->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::ToggleTrack)));
	ui->actionSolo_Track->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::SoloTrack)));
	ui->actionInterpolate->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::Interpolate)));
	goPrevOdrSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::GoToPrevOrder)));
	goNextOdrSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::GoToNextOrder)));
	ui->action_Toggle_Bookmark->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::ToggleBookmark)));
	ui->action_Previous_Bookmark->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PrevBookmark)));
	ui->action_Next_Bookmark->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::NextBookmark)));
	ui->actionDecrease_Note->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::DecreaseNote)));
	ui->actionIncrease_Note->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::IncreaseNote)));
	ui->actionDecrease_Octave->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::DecreaseOctave)));
	ui->actionIncrease_Octave->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::IncreaseOctave)));
	prevInstSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PrevInstrument)));
	nextInstSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::NextInstrument)));
	ui->action_Instrument_Mask->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::MaskInstrument)));
	ui->action_Volume_Mask->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::MaskVolume)));
	ui->actionEdit->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::EditInstrument)));
	ui->actionFollow_Mode->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::FollowMode)));
	ui->actionDuplicate_Order->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::DuplicateOrder)));
	ui->actionClone_Patterns->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::ClonePatterns)));
	ui->actionClone_Order->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::CloneOrder)));
	ui->actionReplace_Instrument->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::ReplaceInstrument)));
	ui->actionExpand->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::ExpandPattern)));
	ui->actionShrink->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::ShrinkPattern)));
	ui->actionFine_Decrease_Values->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::FineDecreaseValues)));
	ui->actionFine_Increase_Values->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::FineIncreaseValues)));
	ui->actionCoarse_D_ecrease_Values->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::CoarseDecreaseValues)));
	ui->actionCoarse_I_ncrease_Values->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::CoarseIncreaseValuse)));
	incPtnSizeSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::IncreasePatternSize)));
	decPtnSizeSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::DecreasePatternSize)));
	incEditStepSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::IncreaseEditStep)));
	decEditStepSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::DecreaseEditStep)));
	ui->action_Effect_List->setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::DisplayEffectList)));
	prevSongSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::PreviousSong)));
	nextSongSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::NextSong)));
	jamVolUpSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::JamVolumeUp)));
	jamVolDownSc_.setShortcut(gui_utils::strToKeySeq(shortcuts.at(Configuration::ShortcutAction::JamVolumeDown)));

	ui->orderList->onShortcutUpdated();
	ui->patternEditor->onShortcutUpdated();
}

void MainWindow::updateInstrumentListColors()
{
	ui->instrumentList->setStyleSheet(
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

void MainWindow::setOrderListGroupMaximumWidth()
{
	ui->orderListGroupBox->setMaximumWidth(
				ui->orderListGroupBox->contentsMargins().left()
				+ ui->orderListGroupBox->layout()->contentsMargins().left()
				+ ui->orderList->maximumWidth()
				+ ui->orderListGroupBox->layout()->contentsMargins().right()
				+ ui->orderListGroupBox->contentsMargins().right());
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

	if (importBankDialog_) {
		if (bankJamMidiCtrl_.load()) return;
		importBankDialog_->onJamKeyOffByMidi(k);
		if (!release) importBankDialog_->onJamKeyOnByMidi(k);
		return;
	}

	int n = instForms_->checkActivatedFormNumber();
	if (n == -1) {
		bt_->jamKeyOff(k); // possibility to recover on stuck note
		if (!release) bt_->jamKeyOn(k, !config_.lock()->getFixJammingVolume());
	}
	else {
		SoundSource src = instForms_->getFormInstrumentSoundSource(n);
		bt_->jamKeyOffForced(k, src); // possibility to recover on stuck note
		if (!release) bt_->jamKeyOnForced(k, src, !config_.lock()->getFixJammingVolume());
	}
}

void MainWindow::midiProgramEvent(uchar status, uchar program)
{
	Q_UNUSED(status)
	int row = findRowFromInstrumentList(program);
	ui->instrumentList->setCurrentRow(row);
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
		std::unordered_map<SoundSource, InstrumentType> map = {
			{ SoundSource::FM, InstrumentType::FM },
			{ SoundSource::SSG, InstrumentType::SSG },
			{ SoundSource::ADPCM, InstrumentType::ADPCM },
		};
		auto& list = ui->instrumentList;

		int num = bt_->findFirstFreeInstrumentNumber();
		if (num == -1) return;	// Maximum count check

		QString name = tr("Instrument %1").arg(num);
		bt_->addInstrument(num, map.at(src), name.toUtf8().toStdString());

		comStack_->push(new AddInstrumentQtCommand(
							list, num, name, map.at(src), instForms_, this,
							config_.lock()->getWriteOnlyUsedSamples()));
		ui->instrumentList->setCurrentRow(num);
		break;
	}
	case SoundSource::RHYTHM:
		break;
	}
}

void MainWindow::addDrumkit()
{
	auto& list = ui->instrumentList;

	int num = bt_->findFirstFreeInstrumentNumber();
	if (num == -1) return;	// Maximum count check

	QString name = tr("Instrument %1").arg(num);
	bt_->addInstrument(num, InstrumentType::Drumkit, name.toUtf8().toStdString());

	comStack_->push(new AddInstrumentQtCommand(
						list, num, name, InstrumentType::Drumkit, instForms_, this,
						config_.lock()->getWriteOnlyUsedSamples()));
	ui->instrumentList->setCurrentRow(num);
}

void MainWindow::removeInstrument(int row)
{
	if (row < 0) return;

	auto& list = ui->instrumentList;
	int num = list->item(row)->data(Qt::UserRole).toInt();

	auto inst = bt_->getInstrument(num);

	bool updateRequest = false;
	if (config_.lock()->getWriteOnlyUsedSamples()){
		if (inst->getSoundSource() == SoundSource::ADPCM) {
			size_t size = bt_->getSampleADPCMUsers(dynamic_cast<InstrumentADPCM*>(
													   inst.get())->getSampleNumber()).size();
			if (size == 1) updateRequest = true;
		}
	}

	bt_->removeInstrument(num);

	// Cancel renaming instrument
	if (list->item(row) == renamingInstItem_) {
		list->removeItemWidget(renamingInstItem_);
		renamingInstItem_ = nullptr;
		renamingInstEdit_ = nullptr;
	}

	comStack_->push(new RemoveInstrumentQtCommand(list, num, row, gui_utils::utf8ToQString(inst->getName()),
												  inst->getType(), instForms_, this, updateRequest));
}

void MainWindow::openInstrumentEditor()
{
	auto item = ui->instrumentList->currentItem();
	int num = item->data(Qt::UserRole).toInt();

	if (!instForms_->getForm(num)) {	// Create form
		std::shared_ptr<QWidget> form;
		auto inst = bt_->getInstrument(num);

		switch (inst->getType()) {
		case InstrumentType::FM:
		{
			auto fmForm = std::make_shared<InstrumentEditorFMForm>(num);
			form = fmForm;
			fmForm->setCore(bt_);
			fmForm->setConfiguration(config_.lock());
			fmForm->setColorPalette(palette_);
			fmForm->resize(config_.lock()->getInstrumentFMWindowWidth(),
						   config_.lock()->getInstrumentFMWindowHeight());

			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::envelopeNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMEnvelopeNumberChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::envelopeParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMEnvelopeParameterChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::lfoNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMLFONumberChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::lfoParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMLFOParameterChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::operatorSequenceNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMOperatorSequenceNumberChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::operatorSequenceParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMOperatorSequenceParameterChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::arpeggioNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMArpeggioNumberChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::arpeggioParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMArpeggioParameterChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::pitchNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMPitchNumberChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::pitchParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentFMPitchParameterChanged);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::jamKeyOnEvent,
							 this, [&](JamKey key) {
				bt_->jamKeyOnForced(key, SoundSource::FM, !config_.lock()->getFixJammingVolume());
			}, Qt::DirectConnection);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::jamKeyOffEvent,
							 this, [&](JamKey key) { bt_->jamKeyOffForced(key, SoundSource::FM); },
			Qt::DirectConnection);
			QObject::connect(fmForm.get(), &InstrumentEditorFMForm::modified,
							 this, &MainWindow::setModifiedTrue);

			fmForm->installEventFilter(this);

			instForms_->onInstrumentFMEnvelopeNumberChanged();
			instForms_->onInstrumentFMLFONumberChanged();
			instForms_->onInstrumentFMOperatorSequenceNumberChanged();
			instForms_->onInstrumentFMArpeggioNumberChanged();
			instForms_->onInstrumentFMPitchNumberChanged();
			break;
		}
		case InstrumentType::SSG:
		{
			auto ssgForm = std::make_shared<InstrumentEditorSSGForm>(num);
			form = ssgForm;
			ssgForm->setCore(bt_);
			ssgForm->setConfiguration(config_.lock());
			ssgForm->setColorPalette(palette_);
			ssgForm->resize(config_.lock()->getInstrumentSSGWindowWidth(),
							config_.lock()->getInstrumentSSGWindowHeight());

			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::waveformNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGWaveformNumberChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::waveformParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGWaveformParameterChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::toneNoiseNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGToneNoiseNumberChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::toneNoiseParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGToneNoiseParameterChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::envelopeNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGEnvelopeNumberChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::envelopeParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGEnvelopeParameterChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::arpeggioNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGArpeggioNumberChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::arpeggioParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGArpeggioParameterChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::pitchNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGPitchNumberChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::pitchParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentSSGPitchParameterChanged);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::jamKeyOnEvent,
							 this, [&](JamKey key) {
				bt_->jamKeyOnForced(key, SoundSource::SSG, !config_.lock()->getFixJammingVolume());
			}, Qt::DirectConnection);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::jamKeyOffEvent,
							 this, [&](JamKey key) { bt_->jamKeyOffForced(key, SoundSource::SSG); }
			, Qt::DirectConnection);
			QObject::connect(ssgForm.get(), &InstrumentEditorSSGForm::modified,
							 this, &MainWindow::setModifiedTrue);

			ssgForm->installEventFilter(this);

			instForms_->onInstrumentSSGWaveformNumberChanged();
			instForms_->onInstrumentSSGToneNoiseNumberChanged();
			instForms_->onInstrumentSSGEnvelopeNumberChanged();
			instForms_->onInstrumentSSGArpeggioNumberChanged();
			instForms_->onInstrumentSSGPitchNumberChanged();
			break;
		}
		case InstrumentType::ADPCM:
		{
			auto adpcmForm = std::make_shared<InstrumentEditorADPCMForm>(num);
			form = adpcmForm;
			adpcmForm->setCore(bt_);
			adpcmForm->setConfiguration(config_.lock());
			adpcmForm->setColorPalette(palette_);
			adpcmForm->resize(config_.lock()->getInstrumentADPCMWindowWidth(),
							  config_.lock()->getInstrumentADPCMWindowHeight());

			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::sampleNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMSampleNumberChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::sampleParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMSampleParameterChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::sampleAssignRequested,
							 this, &MainWindow::assignADPCMSamples);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::sampleMemoryChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMSampleMemoryUpdated);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::envelopeNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMEnvelopeNumberChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::envelopeParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMEnvelopeParameterChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::arpeggioNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMArpeggioNumberChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::arpeggioParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMArpeggioParameterChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::pitchNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMPitchNumberChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::pitchParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMPitchParameterChanged);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::jamKeyOnEvent,
							 this, [&](JamKey key) {
				bt_->jamKeyOnForced(key, SoundSource::ADPCM, !config_.lock()->getFixJammingVolume());
			}, Qt::DirectConnection);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::jamKeyOffEvent,
							 this, [&](JamKey key) { bt_->jamKeyOffForced(key, SoundSource::ADPCM); },
			Qt::DirectConnection);
			QObject::connect(adpcmForm.get(), &InstrumentEditorADPCMForm::modified,
							 this, &MainWindow::setModifiedTrue);

			adpcmForm->installEventFilter(this);

			instForms_->onInstrumentADPCMSampleNumberChanged();
			instForms_->onInstrumentADPCMEnvelopeNumberChanged();
			instForms_->onInstrumentADPCMArpeggioNumberChanged();
			instForms_->onInstrumentADPCMPitchNumberChanged();
			break;
		}
		case InstrumentType::Drumkit:
		{
			auto kitForm = std::make_shared<InstrumentEditorDrumkitForm>(num);
			form = kitForm;
			kitForm->setCore(bt_);
			kitForm->setConfiguration(config_.lock());
			kitForm->setColorPalette(palette_);
			kitForm->resize(config_.lock()->getInstrumentDrumkitWindowWidth(),
							config_.lock()->getInstrumentDrumkitWindowHeight());

			QObject::connect(kitForm.get(), &InstrumentEditorDrumkitForm::sampleNumberChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMSampleNumberChanged);
			QObject::connect(kitForm.get(), &InstrumentEditorDrumkitForm::sampleParameterChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMSampleParameterChanged);
			QObject::connect(kitForm.get(), &InstrumentEditorDrumkitForm::sampleAssignRequested,
							 this, &MainWindow::assignADPCMSamples);
			QObject::connect(kitForm.get(), &InstrumentEditorDrumkitForm::sampleMemoryChanged,
							 instForms_.get(), &InstrumentFormManager::onInstrumentADPCMSampleMemoryUpdated);
			QObject::connect(kitForm.get(), &InstrumentEditorDrumkitForm::jamKeyOnEvent,
							 this, [&](JamKey key) {
				bt_->jamKeyOnForced(key, SoundSource::ADPCM, !config_.lock()->getFixJammingVolume());
			}, Qt::DirectConnection);
			QObject::connect(kitForm.get(), &InstrumentEditorDrumkitForm::jamKeyOffEvent,
							 this, [&](JamKey key) { bt_->jamKeyOffForced(key, SoundSource::ADPCM); },
			Qt::DirectConnection);
			QObject::connect(kitForm.get(), &InstrumentEditorDrumkitForm::modified,
							 this, &MainWindow::setModifiedTrue);

			kitForm->installEventFilter(this);

			instForms_->onInstrumentADPCMSampleNumberChanged();
			break;
		}
		default:
			throw std::invalid_argument("Invalid instrument type");
		}

		form->addActions({ &octUpSc_, &octDownSc_, &jamVolUpSc_, &jamVolDownSc_ });

		instForms_->add(num, std::move(form), inst->getSoundSource(), inst->getType());
	}

	instForms_->showForm(num);
}

int MainWindow::findRowFromInstrumentList(int instNum)
{
	auto& list = ui->instrumentList;
	int row = 0;
	for (; row < list->count(); ++row) {
		auto item = list->item(row);
		if (item->data(Qt::UserRole).toInt() == instNum) break;
	}
	return row;
}

void MainWindow::renameInstrument()
{
	auto list = ui->instrumentList;
	auto item = list->currentItem();
	// Finish current edit
	if (item == renamingInstItem_) {
		finishRenamingInstrument();
		return;
	}
	else if (renamingInstItem_) {
		finishRenamingInstrument();
	}
	renamingInstItem_ = item;

	int num = item->data(Qt::UserRole).toInt();
	renamingInstEdit_ = new QLineEdit(gui_utils::utf8ToQString(bt_->getInstrument(num)->getName()));
	QObject::connect(renamingInstEdit_, &QLineEdit::editingFinished, this, &MainWindow::finishRenamingInstrument);
	renamingInstEdit_->installEventFilter(this);
	ui->instrumentList->setItemWidget(item, renamingInstEdit_);
	renamingInstEdit_->selectAll();
	renamingInstEdit_->setFocus();
}

void MainWindow::finishRenamingInstrument()
{
	if (!renamingInstItem_ || !renamingInstEdit_) return;
	bool hasFocus = renamingInstEdit_->hasFocus();
	auto list = ui->instrumentList;
	int num = renamingInstItem_->data(Qt::UserRole).toInt();
	int row = findRowFromInstrumentList(num);
	auto oldName = gui_utils::utf8ToQString(bt_->getInstrument(num)->getName());
	QString newName = renamingInstEdit_->text();
	list->removeItemWidget(renamingInstItem_);
	if (newName != oldName) {
		bt_->setInstrumentName(num, newName.toUtf8().toStdString());
		comStack_->push(new ChangeInstrumentNameQtCommand(list, num, row, instForms_, oldName, newName));
	}
	renamingInstItem_ = nullptr;
	renamingInstEdit_ = nullptr;
	if (hasFocus) ui->instrumentList->setFocus();
}

void MainWindow::cloneInstrument()
{
	int num = bt_->findFirstFreeInstrumentNumber();
	if (num == -1) return;

	int refNum = ui->instrumentList->currentItem()->data(Qt::UserRole).toInt();
	// KEEP CODE ORDER //
	bt_->cloneInstrument(num, refNum);
	auto inst = bt_->getInstrument(num);
	comStack_->push(new CloneInstrumentQtCommand(ui->instrumentList, num, inst->getType(),
												 gui_utils::utf8ToQString(inst->getName()), instForms_));
	//----------//
}

void MainWindow::deepCloneInstrument()
{
	int num = bt_->findFirstFreeInstrumentNumber();
	if (num == -1) return;

	int refNum = ui->instrumentList->currentItem()->data(Qt::UserRole).toInt();
	// KEEP CODE ORDER //
	bt_->deepCloneInstrument(num, refNum);
	auto inst = bt_->getInstrument(num);
	comStack_->push(new DeepCloneInstrumentQtCommand(
						ui->instrumentList, num, inst->getType(), gui_utils::utf8ToQString(inst->getName()),
						instForms_, this, config_.lock()->getWriteOnlyUsedSamples()));
	//----------//
}

void MainWindow::loadInstrument()
{
	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	std::vector<std::string> orgFilters = io::InstrumentIO::getInstance().getLoadFilter();
	QStringList filters;
	std::transform(orgFilters.begin(), orgFilters.end(), std::back_inserter(filters),
				   [](const std::string& f) { return QString::fromStdString(f); });
	QString selectedFilter = filters.at(config_.lock()->getInstrumentOpenFormat());
	const QStringList files = QFileDialog::getOpenFileNames(this, tr("Open instrument"), (dir.isEmpty() ? "./" : dir),
															filters.join(";;"), &selectedFilter
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
															, QFileDialog::DontUseNativeDialog
#endif
															);
	if (files.empty()) return;

	int index = std::distance(filters.begin(), utils::find(filters, selectedFilter));
	config_.lock()->setInstrumentOpenFormat(index);

	for (const QString& file : files) funcLoadInstrument(file);
}

void MainWindow::funcLoadInstrument(QString file)
{
	int n = bt_->findFirstFreeInstrumentNumber();
	if (n == -1) {
		FileIOErrorMessageBox(file, true, io::FileType::Inst,
							  tr("The number of instruments has reached the upper limit."), this).exec();
		return;
	}

	try {
		io::BinaryContainer container;
		{
			QFile fp(file);
			if (!fp.open(QIODevice::ReadOnly)) {
				FileIOErrorMessageBox::openError(file, true, io::FileType::Inst, this);
				return;
			}
			QByteArray&& array = fp.readAll();
			fp.close();
			std::move(array.begin(), array.end(), std::back_inserter(container));
		}
		bt_->loadInstrument(container, file.toStdString(), n);

		auto inst = bt_->getInstrument(n);
		comStack_->push(new AddInstrumentQtCommand(
							ui->instrumentList, n, gui_utils::utf8ToQString(inst->getName()), inst->getType(),
							instForms_, this, config_.lock()->getWriteOnlyUsedSamples()));
		ui->instrumentList->setCurrentRow(n);
		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(file, true, e, this).exec();
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(file, true, io::FileType::Inst, QString(e.what()), this).exec();
	}
}

void MainWindow::saveInstrument()
{
	int n = ui->instrumentList->currentItem()->data(Qt::UserRole).toInt();
	QString name = gui_utils::utf8ToQString(bt_->getInstrument(n)->getName());

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	std::vector<std::string> orgFilters = io::InstrumentIO::getInstance().getSaveFilter();
	QStringList filters;
	std::transform(orgFilters.begin(), orgFilters.end(), std::back_inserter(filters),
				   [](const std::string& f) { return QString::fromStdString(f); });
	QString defaultFilter = filters.front();	// bti
	QString file = QFileDialog::getSaveFileName(
					   this, tr("Save instrument"),
					   QString("%1/%2.bti").arg(dir.isEmpty() ? "." : dir, name),
					   filters.join(";;"), &defaultFilter
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
					   , QFileDialog::DontUseNativeDialog
#endif
					   );
	if (file.isNull()) return;
	if (!file.endsWith(".bti")) file += ".bti";	// For linux

	try {
		QByteArray bytes;
		{
			io::BinaryContainer container;
			bt_->saveInstrument(container, n);
			bytes.reserve(container.size());
			std::move(container.begin(), container.end(), std::back_inserter(bytes));
		}
		QFile fp(file);
		if (!fp.open(QIODevice::WriteOnly)) {
			FileIOErrorMessageBox::openError(file, false, io::FileType::Inst, this);
			return;
		}
		fp.write(bytes);
		fp.close();

		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(file, false, e, this).exec();
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(file, false, io::FileType::Inst, QString(e.what()), this).exec();
	}
}

void MainWindow::importInstrumentsFromBank()
{
	stopPlaySong();

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	std::vector<std::string> orgFilters = io::BankIO::getInstance().getLoadFilter();
	QStringList filters;
	std::transform(orgFilters.begin(), orgFilters.end(), std::back_inserter(filters),
				   [](const std::string& f) { return QString::fromStdString(f); });
	QString selectedFilter = filters.at(config_.lock()->getBankOpenFormat());
	QString file = QFileDialog::getOpenFileName(this, tr("Open bank"), (dir.isEmpty() ? "./" : dir),
												filters.join(";;"), &selectedFilter
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
												, QFileDialog::DontUseNativeDialog
#endif
												);
	if (file.isNull()) return;

	int index = std::distance(filters.begin(), utils::find(filters, selectedFilter));
	config_.lock()->setBankOpenFormat(index);

	funcImportInstrumentsFromBank(file);
}

void MainWindow::funcImportInstrumentsFromBank(QString file)
{
	stopPlaySong();

	std::unique_ptr<AbstractBank> bank;
	try {
		io::BinaryContainer container;
		{
			QFile fp(file);
			if (!fp.open(QIODevice::ReadOnly)) {
				FileIOErrorMessageBox::openError(file, true, io::FileType::Bank, this);
				return;
			}
			QByteArray&& array = fp.readAll();
			fp.close();
			std::move(array.begin(), array.end(), std::back_inserter(container));
		}

		bank.reset(io::BankIO::getInstance().loadBank(container, file.toStdString()));
		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(file, true, e, this).exec();
		return;
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(file, true, io::FileType::Bank, QString(e.what()), this).exec();
		return;
	}

	// Change text codec
	if (auto ff = dynamic_cast<FfBank*>(bank.get())) {
		QTextCodec* codec = QTextCodec::codecForName("Shift-JIS");
		for (size_t i = 0; i < ff->getNumInstruments(); ++i) {
			std::string sjis = ff->getInstrumentName(i);
			std::string utf8 = codec->toUnicode(sjis.c_str(), sjis.length()).toStdString();
			ff->setInstrumentName(i, utf8);
		}
	}
	else if (auto mu88 = dynamic_cast<Mucom88Bank*>(bank.get())) {
		QTextCodec* codec = QTextCodec::codecForName("Shift-JIS");
		for (size_t i = 0; i < mu88->getNumInstruments(); ++i) {
			std::string sjis = mu88->getInstrumentName(i);
			std::string utf8 = codec->toUnicode(sjis.c_str(), sjis.length()).toStdString();
			mu88->setInstrumentName(i, utf8);
		}
	}

	size_t jamId = 128;	// Dummy
	std::shared_ptr<AbstractInstrument> jamInst;
	importBankDialog_ = std::make_unique<InstrumentSelectionDialog>(*bank, tr("Select instruments to load:"), config_, this);
	auto bankMan = std::make_shared<InstrumentsManager>(true);
	auto updateInst = [&] (size_t id) {
		if (id != jamId) {
			bankJamMidiCtrl_.store(true);
			jamId = id;
			bankMan->clearAll();
			jamInst.reset(bank->loadInstrument(id, bankMan, 0));
			jamInst->setNumber(128);	// Special number
			std::unordered_map<int, std::array<size_t, 2>> sampNums;
			bt_->assignADPCMBeforeForcedJamKeyOn(jamInst, sampNums);
			for (const auto& pairs : sampNums) {
				bankMan->setSampleADPCMStartAddress(pairs.first, pairs.second[0]);
				bankMan->setSampleADPCMStopAddress(pairs.first, pairs.second[1]);
			}
			bankJamMidiCtrl_.store(false);
		}
	};
	QObject::connect(importBankDialog_.get(), &InstrumentSelectionDialog::jamKeyOnEvent,
					 this, [&](size_t id, JamKey key) {
		updateInst(id);
		bt_->jamKeyOnForced(key, jamInst->getSoundSource(),
							!config_.lock()->getFixJammingVolume(), jamInst);
	},
	Qt::DirectConnection);
	QObject::connect(importBankDialog_.get(), &InstrumentSelectionDialog::jamKeyOnMidiEvent,
					 this, [&](size_t id, int key) {
		updateInst(id);
		bt_->jamKeyOnForced(key, jamInst->getSoundSource(),
							!config_.lock()->getFixJammingVolume(), jamInst);
	},
	Qt::DirectConnection);
	QObject::connect(importBankDialog_.get(), &InstrumentSelectionDialog::jamKeyOffEvent,
					 this, [&](JamKey key) { bt_->jamKeyOffForced(key, jamInst->getSoundSource()); },
	Qt::DirectConnection);
	QObject::connect(importBankDialog_.get(), &InstrumentSelectionDialog::jamKeyOffMidiEvent,
					 this, [&](int key) { if (jamInst) bt_->jamKeyOffForced(key, jamInst->getSoundSource()); },
	Qt::DirectConnection);
	importBankDialog_->addActions({ &octUpSc_, &octDownSc_ });

	if (importBankDialog_->exec() != QDialog::Accepted) {
		assignADPCMSamples();	// Restore
		importBankDialog_.reset();
		return;
	}

	const QVector<size_t> selection = importBankDialog_->currentInstrumentSelection();
	importBankDialog_.reset();
	if (selection.empty()) return;

	try {
		bool sampleRestoreRequested = false;
		int lastNum = ui->instrumentList->currentRow();
		for (const size_t& index : selection) {
			int n = bt_->findFirstFreeInstrumentNumber();
			if (n == -1){
				FileIOErrorMessageBox(file, true, io::FileType::Inst,
									  tr("The number of instruments has reached the upper limit."), this).exec();
				ui->instrumentList->setCurrentRow(lastNum);
				return;
			}

			bt_->importInstrument(*bank, index, n);

			auto inst = bt_->getInstrument(n);
			comStack_->push(new AddInstrumentQtCommand(
								ui->instrumentList, n, gui_utils::utf8ToQString(inst->getName()),
								inst->getType(), instForms_, this, config_.lock()->getWriteOnlyUsedSamples(), true));
			lastNum = n;

			sampleRestoreRequested |= (inst->getSoundSource() == SoundSource::ADPCM);
		}
		ui->instrumentList->setCurrentRow(lastNum);

		if (sampleRestoreRequested) assignADPCMSamples();	// Store only once
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(file, true, e, this).exec();
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(file, true, io::FileType::Bank, QString(e.what()), this).exec();
	}
}

void MainWindow::exportInstrumentsToBank()
{
	std::vector<int> ids = bt_->getInstrumentIndices();
	std::shared_ptr<BtBank> bank(std::make_shared<BtBank>(ids, bt_->getInstrumentNames()));

	InstrumentSelectionDialog dlg(*bank, tr("Select instruments to save:"), config_, this);

	if (dlg.exec() != QDialog::Accepted)
		return;

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	std::vector<std::string> orgFilters = io::BankIO::getInstance().getSaveFilter();
	QStringList filters;
	std::transform(orgFilters.begin(), orgFilters.end(), std::back_inserter(filters),
				   [](const std::string& f) { return QString::fromStdString(f); });
	QString defaultFilter = filters.front();	// btb
	QString file = QFileDialog::getSaveFileName(this, tr("Save bank"), (dir.isEmpty() ? "./" : dir),
												filters.join(";;"), &defaultFilter
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
												, QFileDialog::DontUseNativeDialog
#endif
												);
	if (file.isNull()) return;

	QVector<size_t> selection = dlg.currentInstrumentSelection();
	if (selection.empty()) return;
	std::vector<int> sel;
	std::transform(selection.begin(), selection.end(), std::back_inserter(sel),
				   [&ids](size_t i) { return ids.at(i); });
	std::sort(sel.begin(), sel.end());

	try {
		QByteArray bytes;
		{
			io::BinaryContainer container;
			bt_->exportInstruments(container, sel);
			bytes.reserve(container.size());
			std::move(container.begin(), container.end(), std::back_inserter(bytes));
		}
		QFile fp(file);
		if (!fp.open(QIODevice::WriteOnly)) {
			FileIOErrorMessageBox::openError(file, false, io::FileType::Bank, this);
			return;
		}fp.write(bytes);
		fp.close();

		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(file, false, e, this).exec();
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(file, false, io::FileType::Bank, QString(e.what()), this).exec();
	}
}

void MainWindow::swapInstruments(int row1, int row2)
{
	if (row1 == row2) return;

	// KEEP CODE ORDER //
	int num1 = ui->instrumentList->item(row1)->data(Qt::UserRole).toInt();
	int num2 = ui->instrumentList->item(row2)->data(Qt::UserRole).toInt();
	QString name1 = gui_utils::utf8ToQString(bt_->getInstrument(num1)->getName());
	QString name2 = gui_utils::utf8ToQString(bt_->getInstrument(num2)->getName());

	bt_->swapInstruments(num1, num2, config_.lock()->getReflectInstrumentNumberChange());
	comStack_->push(new SwapInstrumentsQtCommand(
						ui->instrumentList, row1, row2, name1, name2, instForms_, ui->patternEditor));
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
	instForms_->clearAll();
	ui->instrumentList->clear();
	on_instrumentList_itemSelectionChanged();

	ui->modTitleLineEdit->setText(gui_utils::utf8ToQString(bt_->getModuleTitle()));
	ui->modTitleLineEdit->setCursorPosition(0);
	ui->authorLineEdit->setText(gui_utils::utf8ToQString(bt_->getModuleAuthor()));
	ui->authorLineEdit->setCursorPosition(0);
	ui->copyrightLineEdit->setText(gui_utils::utf8ToQString(bt_->getModuleCopyright()));
	ui->copyrightLineEdit->setCursorPosition(0);
	{
		QSignalBlocker blocker(ui->songComboBox);	// Prevent duplicated call "loadSong"
		ui->songComboBox->clear();
		for (size_t i = 0; i < bt_->getSongCount(); ++i) {
			QString title = gui_utils::utf8ToQString(bt_->getSongTitle(static_cast<int>(i)));
			if (title.isEmpty()) title = tr("Untitled");
			ui->songComboBox->addItem(QString("#%1 %2").arg(i).arg(title));
		}
	}
	highlight1_->setValue(static_cast<int>(bt_->getModuleStepHighlight1Distance()));
	highlight2_->setValue(static_cast<int>(bt_->getModuleStepHighlight2Distance()));

	for (auto& idx : bt_->getInstrumentIndices()) {
		auto inst = bt_->getInstrument(idx);
		comStack_->push(new AddInstrumentQtCommand(
							ui->instrumentList, idx, gui_utils::utf8ToQString(inst->getName()), inst->getType(),
							instForms_, this, config_.lock()->getWriteOnlyUsedSamples(), true));
	}

	isSavedModBefore_ = false;

	loadSong();

	// Set tick frequency
	stream_->setInterruption(bt_->getModuleTickFrequency());
	if (tickTimerForRealChip_) tickTimerForRealChip_->setInterval(1000000 / bt_->getModuleTickFrequency());
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

	// Set comment
	if (commentDialog_) commentDialog_->setComment(gui_utils::utf8ToQString(bt_->getModuleComment()));

	// Clear records
	QApplication::clipboard()->clear();
	comStack_->clear();
	bt_->clearCommandHistory();
}

void MainWindow::openModule(QString file)
{
	try {
		freezeViews();
		if (tickTimerForRealChip_) tickTimerForRealChip_->stop();
		else stream_->stop();

		QFile fp(file);
		if (fp.open(QIODevice::ReadOnly)) {
			io::BinaryContainer container;
			{
				QByteArray&& array = fp.readAll();
				fp.close();
				std::move(array.begin(), array.end(), std::back_inserter(container));
			}

			bt_->loadModule(container);
			bt_->setModulePath(file.toStdString());

			loadModule();

			config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
			changeFileHistory(file);

			goto AFTER_MOD_LOADING;	// Skip error handling section
		}
		else {
			FileIOErrorMessageBox::openError(file, true, io::FileType::Mod, this);
		}
	}
	catch (std::exception& e) {
		if (auto ef = dynamic_cast<io::FileIOError*>(&e)) {
			FileIOErrorMessageBox(file, true, *ef, this).exec();
		}
		else {
			FileIOErrorMessageBox(file, true, io::FileType::Mod, QString(e.what()), this).exec();
		}
	}

	// Init module as a plain when something is wrong
	freezeViews();
	bt_->makeNewModule();
	loadModule();

AFTER_MOD_LOADING:	// Post process of module loading
	isModifiedForNotCommand_ = false;
	setWindowModified(false);
	if (tickTimerForRealChip_) tickTimerForRealChip_->start();
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
	ui->orderList->onSongLoaded();
	setOrderListGroupMaximumWidth();
	ui->patternEditor->onSongLoaded();
	unfreezeViews();

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
	ksManForm_->onCurrentSongNumberChanged();

	// Update track visibility
	std::vector<int> visTracks;
	int all = static_cast<int>(bt_->getSongStyle(curSong).trackAttribs.size());
	for (int i = 0; i < all; ++i) {
		if (bt_->isVisibleTrack(curSong, i)) visTracks.push_back(i);
	}
	ui->orderList->setVisibleTracks(visTracks);
	setOrderListGroupMaximumWidth();
	ui->patternEditor->setVisibleTracks(visTracks);
}

void MainWindow::assignADPCMSamples()
{
	bt_->stopPlaySong();
	lockWidgets(false);
	if (tickTimerForRealChip_) tickTimerForRealChip_->stop();
	else stream_->stop();
	bool isStoredAll = bt_->assignSampleADPCMRawSamples();	// Mutex register
	instForms_->onInstrumentADPCMSampleMemoryUpdated();

	if (!isStoredAll) {
		QMessageBox::warning(this, tr("Warning"),
							 tr("Insufficient memory size to load ADPCM samples. Please delete the unused samples."));
	}

	if (tickTimerForRealChip_) tickTimerForRealChip_->start();
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
	bool streamState = false;
	RealChipInterfaceType intf = config_.lock()->getRealChipInterface();
	if (intf == RealChipInterfaceType::NONE) {
		tickTimerForRealChip_.reset();
		bt_->connectToRealChip(RealChipInterfaceType::NONE);
		QString streamErr;
		streamState = stream_->initialize(
						  config_.lock()->getSampleRate(),
						  config_.lock()->getBufferLength(),
						  bt_->getModuleTickFrequency(),
						  gui_utils::utf8ToQString(config_.lock()->getSoundAPI()),
						  gui_utils::utf8ToQString(config_.lock()->getSoundDevice()),
						  &streamErr);
		if (!streamState) showStreamFailedDialog(streamErr);
		stream_->start();
	}
	else {
		stream_->stop();
		if (tickTimerForRealChip_) {
			tickTimerForRealChip_->stop();
		}
		else {
			tickTimerForRealChip_ = std::make_unique<PreciseTimer>();
			tickTimerForRealChip_->setInterval(1000000 / bt_->getModuleTickFrequency());
			tickEventMethod_ = metaObject()->indexOfSlot("onNewTickSignaledRealChip()");
			Q_ASSERT(tickEventMethod_ != -1);
			tickTimerForRealChip_->setFunction([&]{
				QMetaMethod method = this->metaObject()->method(this->tickEventMethod_);
				method.invoke(this, Qt::QueuedConnection);
			});
		}

		setRealChipInterface(intf);

		tickTimerForRealChip_->start();
	}

	setMidiConfiguration();
	NoteNameManager::getManager().setNotationSystem(config_.lock()->getNotationSystem());
	updateFonts();
	ui->orderList->setHorizontalScrollMode(config_.lock()->getMoveCursorByHorizontalScroll());
	ui->patternEditor->setHorizontalScrollMode(config_.lock()->getMoveCursorByHorizontalScroll());
	instForms_->updateByConfiguration();

	bt_->changeConfiguration(config_);

	if (streamState) {
		uint32_t sr = stream_->getStreamRate();
		if (config_.lock()->getSampleRate() != sr) {
			showStreamRateWarningDialog(sr);
			bt_->setStreamRate(sr);
		}
	}

	setShortcuts();
	updateInstrumentListColors();
	bmManForm_->onConfigurationChanged(config_.lock()->getShowRowNumberInHex());
	ksManForm_->onConfigurationChanged(config_.lock()->getShowRowNumberInHex());

	visualTimer_->stop();
	visualTimer_->start(static_cast<int>(std::round(1000. / config_.lock()->getWaveViewFrameRate())));

	update();
}

void MainWindow::setRealChipInterface(RealChipInterfaceType intf)
{
	if (intf == bt_->getRealChipInterfaceType()) return;

	if (isWindowModified()
			&& QMessageBox::warning(this, tr("Warning"),
									tr("The module has been changed. Do you want to save it?"),
									QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
		on_actionSave_As_triggered();
	}

	struct Pair
	{
		std::weak_ptr<QLibrary> lib;
		const char* symbol;
	};
	static std::unordered_map<RealChipInterfaceType, Pair> RCI_DIC = {
		{ RealChipInterfaceType::SCCI, { scciDll_, "getSoundInterfaceManager" } },
		{ RealChipInterfaceType::C86CTL, { c86ctlDll_, "CreateInstance" } }
	};

	auto& dll = RCI_DIC.at(intf).lib;
	dll.lock()->load();
	if (dll.lock()->isLoaded())
		bt_->connectToRealChip(intf, new RealChipInterfaceGeneratorFunc(dll.lock()->resolve(RCI_DIC[intf].symbol)));
	else
		bt_->connectToRealChip(RealChipInterfaceType::NONE);

	bt_->assignSampleADPCMRawSamples();	// Mutex register
	instForms_->onInstrumentADPCMSampleMemoryUpdated();
}

void MainWindow::setMidiConfiguration()
{
	MidiInterface &midiIntf = MidiInterface::getInstance();
	std::string midiApi = config_.lock()->getMidiAPI();
	std::string midiInPortName = config_.lock()->getMidiInputPort();

	if (config_.lock()->getMidiEnabled()) {
		std::string errDetail;
		if (midiApi.empty()) {
			config_.lock()->setMidiEnabled(false);
			midiIntf.switchApi("");	// Clear
		}
		else {
			if (!midiIntf.isAvailableApi(midiApi)) {
				showMidiFailedDialog("Invalid API name.");
				midiIntf.switchApi("");	// Clear
				return;
			}
			if (midiIntf.switchApi(midiApi, &errDetail)) {
				bool resPort = true;
				if (!midiInPortName.empty())
					resPort = midiIntf.openInputPortByName(midiInPortName, &errDetail);
				else if (midiIntf.supportsVirtualPort())
					resPort = midiIntf.openInputPort(~0u, &errDetail);
				else
					config_.lock()->setMidiEnabled(false);
				if (!resPort) {
					showMidiFailedDialog(QString::fromStdString(errDetail));
					midiIntf.switchApi("");	// Clear
				}
			}
			else {
				showMidiFailedDialog(QString::fromStdString(errDetail));
				midiIntf.switchApi("");	// Clear
			}
		}
	}
}

void MainWindow::updateFonts()
{
	ui->patternEditor->setFonts(
				gui_utils::utf8ToQString(config_.lock()->getPatternEditorHeaderFont()),
				config_.lock()->getPatternEditorHeaderFontSize(),
				gui_utils::utf8ToQString(config_.lock()->getPatternEditorRowsFont()),
				config_.lock()->getPatternEditorRowsFontSize());
	ui->orderList->setFonts(
				gui_utils::utf8ToQString(config_.lock()->getOrderListHeaderFont()),
				config_.lock()->getOrderListHeaderFontSize(),
				gui_utils::utf8ToQString(config_.lock()->getOrderListRowsFont()),
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
	QString filePath = QString::fromStdString(bt_->getModulePath());
	QString fileName = filePath.isEmpty() ? tr("Untitled") : QFileInfo(filePath).fileName();
	QString songTitle = gui_utils::utf8ToQString(bt_->getSongTitle(n));
	if (songTitle.isEmpty()) songTitle = tr("Untitled");
	QMainWindow::setWindowTitle(QString("%1[*] [#%2 %3] - BambooTracker")
								.arg(fileName, QString::number(n), songTitle));
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
		ui->instrumentList->setCurrentRow(0);
	}
}

QString MainWindow::getModuleFileBaseName() const
{
	auto filePathStd = bt_->getModulePath();
	QString filePath = QString::fromStdString(filePathStd);
	return (filePath.isEmpty() ? tr("Untitled") : QFileInfo(filePath).completeBaseName());
}

/******************************/
/********** Instrument list events **********/
void MainWindow::on_instrumentList_customContextMenuRequested(const QPoint &pos)
{
	auto& list = ui->instrumentList;
	QPoint globalPos = list->mapToGlobal(pos);
	QMenu menu;

	// Leave Before Qt5.7.0 style due to windows xp
	menu.addActions({ ui->actionNew_Instrument, ui->actionNew_Drumki_t, ui->actionRemove_Instrument });
	menu.addSeparator();
	menu.addAction(ui->actionRename_Instrument);
	menu.addSeparator();
	menu.addActions({ ui->actionClone_Instrument, ui->actionDeep_Clone_Instrument });
	menu.addSeparator();
	menu.addActions({ ui->actionLoad_From_File, ui->actionSave_To_File });
	menu.addSeparator();
	menu.addActions({ ui->actionImport_From_Bank_File, ui->actionExport_To_Bank_File });
	menu.addSeparator();
	menu.addAction(ui->actionEdit);

	menu.exec(globalPos);
}

void MainWindow::on_instrumentList_itemDoubleClicked(QListWidgetItem *)
{
	openInstrumentEditor();
}

void MainWindow::on_instrumentList_itemSelectionChanged()
{
	int num = (ui->instrumentList->currentRow() == -1)
			  ? -1
			  : ui->instrumentList->currentItem()->data(Qt::UserRole).toInt();
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
		ui->action_Insert->setEnabled(false);
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
		ui->actionFine_Decrease_Values->setEnabled(false);
		ui->actionFine_Increase_Values->setEnabled(false);
		ui->actionCoarse_D_ecrease_Values->setEnabled(false);
		ui->actionCoarse_I_ncrease_Values->setEnabled(false);
	}
	else {
		// Edit
		bool enabled = QApplication::clipboard()->text().startsWith("PATTERN_");
		ui->actionPaste->setEnabled(enabled);
		ui->actionMix->setEnabled(enabled);
		ui->actionOverwrite->setEnabled(enabled);
		ui->action_Insert->setEnabled(enabled);
		ui->actionDelete->setEnabled(true);
		// Pattern
		ui->actionInterpolate->setEnabled(isSelectedPattern_);
		ui->actionReverse->setEnabled(isSelectedPattern_);
		ui->actionReplace_Instrument->setEnabled(
					isSelectedPattern_ && ui->instrumentList->currentRow() != -1);
		ui->actionExpand->setEnabled(isSelectedPattern_);
		ui->actionShrink->setEnabled(isSelectedPattern_);
		ui->actionDecrease_Note->setEnabled(true);
		ui->actionIncrease_Note->setEnabled(true);
		ui->actionDecrease_Octave->setEnabled(true);
		ui->actionIncrease_Octave->setEnabled(true);
		ui->actionFine_Decrease_Values->setEnabled(true);
		ui->actionFine_Increase_Values->setEnabled(true);
		ui->actionCoarse_D_ecrease_Values->setEnabled(true);
		ui->actionCoarse_I_ncrease_Values->setEnabled(true);
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
	ui->action_Insert->setEnabled(false);
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
	ui->actionFine_Decrease_Values->setEnabled(false);
	ui->actionFine_Increase_Values->setEnabled(false);
	ui->actionCoarse_D_ecrease_Values->setEnabled(false);
	ui->actionCoarse_I_ncrease_Values->setEnabled(false);

	updateMenuByOrderSelection(isSelectedOrder_);
}

void MainWindow::onCurrentTrackChanged()
{
	SoundSource src = bt_->getCurrentTrackAttribute().source;
	bool space = (bt_->findFirstFreeInstrumentNumber() != -1);
	ui->actionNew_Instrument->setEnabled((src != SoundSource::RHYTHM) && space);
	ui->actionNew_Drumki_t->setEnabled((src == SoundSource::ADPCM) && space);
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
	ui->action_Insert->setEnabled(false);
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
	ui->actionFine_Decrease_Values->setEnabled(false);
	ui->actionFine_Increase_Values->setEnabled(false);
	ui->actionCoarse_D_ecrease_Values->setEnabled(false);
	ui->actionCoarse_I_ncrease_Values->setEnabled(false);
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
					enabled && ui->instrumentList->currentRow() != -1);
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
	ModulePropertiesDialog dialog(bt_, config_.lock()->getMixerVolumeFM(), config_.lock()->getMixerVolumeSSG(), this);
	if (dialog.exec() == QDialog::Accepted
			&& showUndoResetWarningDialog(tr("Do you want to change song properties?"))) {
		int instRow = ui->instrumentList->currentRow();
		bt_->stopPlaySong();
		lockWidgets(false);
		dialog.onAccepted();
		freezeViews();
		if (!tickTimerForRealChip_) stream_->stop();
		loadModule();
		setModifiedTrue();
		setWindowTitle();
		ui->instrumentList->setCurrentRow(instRow);
		if (!tickTimerForRealChip_) stream_->start();
		assignADPCMSamples();
	}
}

void MainWindow::on_actionNew_Instrument_triggered()
{
	addInstrument();
}

void MainWindow::on_actionRemove_Instrument_triggered()
{
	removeInstrument(ui->instrumentList->currentRow());
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
	ui->statusBar->showMessage(editable ? tr("Change to edit mode") : tr("Change to jam mode"), STATUS_DISPLAY_TIMEOUT);
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
	static const QString APP_NAME = "BambooTracker v" + QString::fromStdString(Version::ofApplicationInString());
	static const QString APP_DESC = tr("YM2608 Music Tracker");
	static constexpr char COPY[] = "Copyright (C) 2018-2021 Rerrah";
	static const QString WEB = tr("Web:")
			+ R"( <a href="https://bambootracker.github.io/BambooTracker/">https://bambootracker.github.io/BambooTracker/</a>)";

	static const QString LICENSE = tr("This software is licensed under the GNU General Public License v2.0 or later.");
	static const QString SOURCE = tr("Source is available at:");
	static const QString REPO = R"(<a href="https://github.com/BambooTracker/BambooTracker">https://github.com/BambooTracker/BambooTracker</a>)";

	static const QString LIB_HEAD = tr("Libraries:");
	static const QString CONTRIBUTORS = tr("Also see changelog which lists contributors.");
	static const QString ACKNOWLEGEMENT = tr("Thank you to everyone who reports bugs, makes suggestions, and contributes to this project!");

	static const QStringList LIBS = {
	#ifdef USE_REAL_CHIP
		tr("C86CTL by (C) honet (BSD 3-Clause)"),
	#endif
		tr("emu2149 by (C) Mitsutaka Okazaki (MIT License)"),
		tr("fmopn by (C) Tatsuyuki Satoh, Jarek Burczynski, ValleyBell (GPL v2+)"),
		tr("libOPNMIDI by (C) Vitaly Novichkov (MIT License part)"),
		tr("Nuked OPN-MOD by (C) Alexey Khokholov (Nuke.YKT), Jean Pierre Cimalando (LGPL v2.1+)"),
		tr("Qt (GPL v2+ or LGPL v3)"),
		tr("RtAudio by (C) Gary P. Scavone (RtAudio License)"),
		tr("RtMidi by (C) Gary P. Scavone (RtMidi License)"),
	#ifdef USE_REAL_CHIP
		tr("SCCI by (C) gasshi (SCCI License)"),
	#endif
		tr("Silk icons by (C) Mark James (CC BY 2.5 or 3.0)"),
		tr("ymdeltat by (C) Tatsuyuki Satoh, Jarek Burczynski, ValleyBell (GPL v2+)")
	};

	static const QString FORMATTED_TEXT =
			"<h2>" + APP_NAME + "</h2>"
			"<p><strong>" + APP_DESC + "</strong></p>"
			"<p><b>" + COPY + "</b></p>"
			"<p>" + WEB + "</p>"
			"<p>" + LICENSE + "<br>"
			+ SOURCE + "<br>" + REPO + "</p>"
			"<hr>"
			"<p>" + LIB_HEAD + "<ul style='margin-left: 16px; -qt-list-indent: 0;'>"
			+ (QStringList { "" } + LIBS).join("<li>") + "</li>"
			+ "</ul></p>"
			"<p>" + CONTRIBUTORS + "<br>"
			+ ACKNOWLEGEMENT +"</p>";

	QMessageBox dialog(QMessageBox::NoIcon, tr("About"), FORMATTED_TEXT, QMessageBox::Ok, this);
	dialog.setTextFormat(Qt::RichText);
	dialog.setIconPixmap(QIcon(":/icon/app_icon").pixmap(QSize(44, 44)));
	dialog.exec();
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

	GrooveSettingsDialog dialog(this);
	dialog.setGrooveSquences(seqs);
	if (dialog.exec() == QDialog::Accepted) {
		bt_->stopPlaySong();
		lockWidgets(false);
		bt_->setGrooves(dialog.getGrooveSequences());
		ui->grooveSpinBox->setMaximum(static_cast<int>(bt_->getGrooveCount()) - 1);
		setModifiedTrue();
	}
}

void MainWindow::on_actionConfiguration_triggered()
{
	ConfigurationDialog dialog(config_.lock(), palette_, stream_, this);
	QObject::connect(&dialog, &ConfigurationDialog::applyPressed, this, &MainWindow::changeConfiguration);

	if (dialog.exec() == QDialog::Accepted) {
		changeConfiguration();
		io::saveConfiguration(config_.lock());
		io::savePalette(palette_.get());
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
		switch (ModuleSaveCheckDialog(bt_->getModuleTitle(), this).exec()) {
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
	if (!tickTimerForRealChip_) stream_->stop();
	bt_->makeNewModule();
	loadModule();
	setInitialSelectedInstrument();
	isModifiedForNotCommand_ = false;
	setWindowModified(false);
	if (!tickTimerForRealChip_) stream_->start();
	assignADPCMSamples();
}

void MainWindow::on_actionComments_triggered()
{
	if (commentDialog_) {
		if (commentDialog_->isVisible()) commentDialog_->activateWindow();
		else commentDialog_->show();
	}
	else {
		commentDialog_ = std::make_unique<CommentEditDialog>(gui_utils::utf8ToQString(bt_->getModuleComment()), this);
		commentDialog_->show();
		QObject::connect(commentDialog_.get(), &CommentEditDialog::commentChanged,
						 this, [&](const QString text) {
			bt_->setModuleComment(text.toUtf8().toStdString());
			setModifiedTrue();
		});
	}
}

bool MainWindow::on_actionSave_triggered()
{
	auto path = QString::fromStdString(bt_->getModulePath());
	if (!path.isEmpty() && QFileInfo::exists(path) && QFileInfo(path).isFile()) {
		if (!backupModule(path)) return false;

		try {
			QByteArray bytes;
			{
				io::BinaryContainer container;
				bt_->saveModule(container);
				bytes.reserve(container.size());
				std::move(container.begin(), container.end(), std::back_inserter(bytes));
			}
			QFile fp(path);
			if (!fp.open(QIODevice::WriteOnly)) {
				FileIOErrorMessageBox::openError(path, false, io::FileType::Mod, this);
				return false;
			}
			fp.write(bytes);
			fp.close();

			isModifiedForNotCommand_ = false;
			isSavedModBefore_ = true;
			setWindowModified(false);
			setWindowTitle();
			return true;
		}
		catch (io::FileIOError& e) {
			FileIOErrorMessageBox(path, false, e, this).exec();
			return false;
		}
		catch (std::exception& e) {
			FileIOErrorMessageBox(path, false, io::FileType::Mod, QString(e.what()), this).exec();
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
					   tr("BambooTracker module (*.btm)") + ";;" + tr("All files (*)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
					   , QFileDialog::DontUseNativeDialog
#endif
					   );
	if (file.isNull()) return false;
	if (!file.endsWith(".btm")) file += ".btm";	// For linux

	if (QFile::exists(file)) {	// Backup if the module already exists
		if (!backupModule(file)) return false;
	}

	bt_->setModulePath(file.toStdString());
	try {
		QByteArray bytes;
		{
			io::BinaryContainer container;
			bt_->saveModule(container);
			bytes.reserve(container.size());
			std::move(container.begin(), container.end(), std::back_inserter(bytes));
		}

		QFile fp(file);
		if (!fp.open(QIODevice::WriteOnly)) {
			FileIOErrorMessageBox::openError(file, false, io::FileType::Mod, this);
			return false;
		}
		fp.write(bytes);
		fp.close();

		isModifiedForNotCommand_ = false;
		isSavedModBefore_ = true;
		setWindowModified(false);
		setWindowTitle();
		config_.lock()->setWorkingDirectory(QFileInfo(file).dir().path().toStdString());
		changeFileHistory(file);
		return true;
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(file, false, e, this).exec();
		return false;
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(file, false, io::FileType::Mod, QString(e.what()), this).exec();
		return false;
	}
}

void MainWindow::on_actionOpen_triggered()
{
	if (isWindowModified()) {
		switch (ModuleSaveCheckDialog(bt_->getModuleTitle(), this).exec()) {
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
												tr("BambooTracker module (*.btm)") + ";;" + tr("All files (*)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
												, QFileDialog::DontUseNativeDialog
#endif
												);
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

		auto list = ui->instrumentList;
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
	// Record current mute states
	const auto& style = bt_->getSongStyle(bt_->getCurrentSongNumber());
	const auto& attribs = style.trackAttribs;
	std::vector<bool> muteStates(attribs.size());
	std::vector<int> unmuteTracks;
	for (size_t i = 0; i < attribs.size(); ++i) {
		muteStates[i] = bt_->isMute(attribs[i].number);
		if (!muteStates[i]) unmuteTracks.push_back(static_cast<int>(i));
	}

	unmuteTracks = gui_utils::adaptVisibleTrackList(unmuteTracks, style.type, SongType::Standard);
	WaveExportSettingsDialog dialog(unmuteTracks, this);
	if (dialog.exec() != QDialog::Accepted) return;

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString path = QFileDialog::getSaveFileName(
					   this, tr("Export to WAV"),
					   QString("%1/%2.wav").arg(dir.isEmpty() ? "." : dir, getModuleFileBaseName()),
					   tr("WAV signed 16-bit PCM (*.wav)") + ";;" + tr("All files (*)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
					   , QFileDialog::DontUseNativeDialog
#endif
					   );
	if (path.isNull()) return;
	if (!path.endsWith(".wav")) path += ".wav";	// For linux
	QString exDir = QFileInfo(path).dir().path();

	int max = static_cast<int>(bt_->getTotalStepCount(
								   bt_->getCurrentSongNumber(), static_cast<size_t>(dialog.getLoopCount()))) + 3;
	QProgressDialog progress("", tr("Cancel"), 0, max, this);
	progress.setWindowModality(Qt::ApplicationModal);
	progress.setWindowFlags(progress.windowFlags()
							& ~Qt::WindowContextHelpButtonHint
							& ~Qt::WindowCloseButtonHint);
	auto bar = [&progress] {
		QApplication::processEvents();
		progress.setValue(progress.value() + 1);
		return progress.wasCanceled();
	};

	unmuteTracks = dialog.getSoloExportTracks();
	unmuteTracks.insert(unmuteTracks.begin(), -1);

	bt_->stopPlaySong();
	lockWidgets(false);
	stream_->stop();

	for (size_t i = 0; i < unmuteTracks.size(); ++i) {
		int curTrack = unmuteTracks[i];
		QString text, name;
		if (curTrack == -1) text = tr("Export to WAV");	// Mixed all
		else {
			if (curTrack < 6) name = gui_utils::getTrackName(SongType::Standard, SoundSource::FM, curTrack);
			else if (curTrack < 9) name = gui_utils::getTrackName(SongType::Standard, SoundSource::SSG, curTrack - 6);
			else if (curTrack < 15) name = gui_utils::getTrackName(SongType::Standard, SoundSource::RHYTHM, curTrack - 9);
			else name = gui_utils::getTrackName(SongType::Standard, SoundSource::ADPCM, 0);
			text = tr("Export %1 to WAV").arg(name);
		}
		progress.setLabelText(text);
		progress.setValue(0);
		progress.open();

		// Update mute states
		if (i == 1) {
			for (const TrackAttribute& attrib : attribs)
				bt_->setTrackMuteState(attrib.number, attrib.number != unmuteTracks[i]);
		}
		else if (i > 1) {
			int prevTrack = unmuteTracks[i - 1];
			if (style.type == SongType::FM3chExpanded) {
				if (prevTrack == 2) {
					bt_->setTrackMuteState(2, true);
					bt_->setTrackMuteState(3, true);
					bt_->setTrackMuteState(4, true);
					bt_->setTrackMuteState(5, true);
				}
				else if (prevTrack < 2) {
					bt_->setTrackMuteState(prevTrack, true);
				}
				else {
					bt_->setTrackMuteState(prevTrack + 3, true);
				}
				if (curTrack == 2) {
					bt_->setTrackMuteState(2, false);
					bt_->setTrackMuteState(3, false);
					bt_->setTrackMuteState(4, false);
					bt_->setTrackMuteState(5, false);
				}
				else if (curTrack < 2) {
					bt_->setTrackMuteState(curTrack, false);
				}
				else {
					bt_->setTrackMuteState(curTrack + 3, false);
				}
			}
			else {
				bt_->setTrackMuteState(prevTrack, true);
				bt_->setTrackMuteState(curTrack, false);
			}
		}

		try {
			QByteArray bytes;
			{
				const uint32_t rate = static_cast<uint32_t>(dialog.getSampleRate());
				const uint16_t nCh = 2;
				const int loopCnt = dialog.getLoopCount();
				io::WavContainer container(rate, nCh, 16);
				if (!bt_->exportToWav(container, loopCnt, bar))
					break;	// Jump if cancelled
				bytes.reserve(container.size());
				std::move(container.begin(), container.end(), std::back_inserter(bytes));
			}

			if (curTrack > -1) path = QString("%1/%2 - %3.wav").arg(exDir).arg(curTrack + 1, 2, 10, QChar('0')).arg(name);
			QFile fp(path);
			if (!fp.open(QIODevice::WriteOnly)) {
				FileIOErrorMessageBox::openError(path, false, io::FileType::WAV, this);
				break;	// Jump to post process
			}
			fp.write(bytes);
			fp.close();
			bar();

			config_.lock()->setWorkingDirectory(QFileInfo(path).dir().path().toStdString());
		}
		catch (io::FileIOError& e) {
			FileIOErrorMessageBox(path, false, e, this).exec();
			break;
		}
		catch (std::exception& e) {
			FileIOErrorMessageBox(path, false, io::FileType::WAV, QString(e.what()), this).exec();
			break;
		}
	}

	// Restore states
	for (size_t i = 0 ; i < attribs.size(); ++i) {
		bt_->setTrackMuteState(attribs[i].number, muteStates[i]);
	}
	stream_->start();
}

void MainWindow::on_actionVGM_triggered()
{
	VgmExportSettingsDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted) return;
	io::GD3Tag tag = dialog.getGD3Tag();

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString path = QFileDialog::getSaveFileName(
					   this, tr("Export to VGM"),
					   QString("%1/%2.vgm").arg(dir.isEmpty() ? "." : dir, getModuleFileBaseName()),
					   tr("VGM file (*.vgm)") + ";;" + tr("All files (*)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
					   , QFileDialog::DontUseNativeDialog
#endif
					   );
	if (path.isNull()) return;
	if (!path.endsWith(".vgm")) path += ".vgm";	// For linux

	int max = static_cast<int>(bt_->getTotalStepCount(bt_->getCurrentSongNumber(), 1)) + 3;
	QProgressDialog progress(tr("Export to VGM"), tr("Cancel"), 0, max, this);
	progress.setValue(0);
	progress.setWindowFlags(progress.windowFlags()
							& ~Qt::WindowContextHelpButtonHint
							& ~Qt::WindowCloseButtonHint);
	progress.setWindowModality(Qt::ApplicationModal);
	progress.open();

	bt_->stopPlaySong();
	lockWidgets(false);
	stream_->stop();

	try {
		auto bar = [&progress]() -> bool {
				   QApplication::processEvents();
				   progress.setValue(progress.value() + 1);
				   return progress.wasCanceled();
	};

		QByteArray bytes;
		{
			io::BinaryContainer container;
			if (!bt_->exportToVgm(container, dialog.getExportTarget(), dialog.enabledGD3(), tag, bar))
				goto AFTER_VGM_WRITE;	// Jump if cancelled
			bytes.reserve(container.size());
			std::move(container.begin(), container.end(), std::back_inserter(bytes));
		}
		QFile fp(path);
		if (!fp.open(QIODevice::WriteOnly)) {
			FileIOErrorMessageBox::openError(path, false, io::FileType::VGM, this);
			goto AFTER_VGM_WRITE;	// Jump if cancelled
		}
		fp.write(bytes);
		fp.close();
		bar();

		config_.lock()->setWorkingDirectory(QFileInfo(path).dir().path().toStdString());
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(path, false, e, this).exec();
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(path, false, io::FileType::VGM, QString(e.what()), this).exec();
	}

AFTER_VGM_WRITE:
	stream_->start();
}

void MainWindow::on_actionS98_triggered()
{
	S98ExportSettingsDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted) return;
	io::S98Tag tag = dialog.getS98Tag();

	QString dir = QString::fromStdString(config_.lock()->getWorkingDirectory());
	QString path = QFileDialog::getSaveFileName(
					   this, tr("Export to S98"),
					   QString("%1/%2.s98").arg(dir.isEmpty() ? "." : dir, getModuleFileBaseName()),
					   tr("S98 file (*.s98)") + ";;" + tr("All files (*)"), nullptr
#if defined(Q_OS_LINUX) || (defined(Q_OS_BSD4) && !defined(Q_OS_DARWIN))
					   , QFileDialog::DontUseNativeDialog
#endif
					   );
	if (path.isNull()) return;
	if (!path.endsWith(".s98")) path += ".s98";	// For linux

	int max = static_cast<int>(bt_->getTotalStepCount(bt_->getCurrentSongNumber(), 1)) + 3;
	QProgressDialog progress(tr("Export to S98"), tr("Cancel"), 0, max, this);
	progress.setValue(0);
	progress.setWindowFlags(progress.windowFlags()
							& ~Qt::WindowContextHelpButtonHint
							& ~Qt::WindowCloseButtonHint);
	progress.setWindowModality(Qt::ApplicationModal);
	progress.open();

	bt_->stopPlaySong();
	lockWidgets(false);
	stream_->stop();

	try {
		auto bar = [&progress]() -> bool {
				   QApplication::processEvents();
				   progress.setValue(progress.value() + 1);
				   return progress.wasCanceled();
	};

		QByteArray bytes;
		{
			io::BinaryContainer container;
			if (!bt_->exportToS98(container, dialog.getExportTarget(), dialog.enabledTag(),
								  tag, dialog.getResolution(), bar))
				goto AFTER_S98_WRITE;	// Jump if cancelled
			bytes.reserve(container.size());
			std::move(container.begin(), container.end(), std::back_inserter(bytes));
		}

		QFile fp(path);
		if (!fp.open(QIODevice::WriteOnly)) {
			FileIOErrorMessageBox::openError(path, false, io::FileType::S98, this);
			goto AFTER_S98_WRITE;	// Jump to post process
		}
		fp.write(bytes);
		fp.close();
		bar();

		config_.lock()->setWorkingDirectory(QFileInfo(path).dir().path().toStdString());
	}
	catch (io::FileIOError& e) {
		FileIOErrorMessageBox(path, false, e, this).exec();
	}
	catch (std::exception& e) {
		FileIOErrorMessageBox(path, false, io::FileType::S98, QString(e.what()), this).exec();
	}

AFTER_S98_WRITE:
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
	int16_t wave[2 * bt_defs::OUTPUT_HISTORY_SIZE];
	bt_->getOutputHistory(wave);

	ui->waveVisual->setStereoSamples(wave, bt_defs::OUTPUT_HISTORY_SIZE);
}

void MainWindow::on_action_Effect_List_triggered()
{
	if (effListDialog_) {
		if (effListDialog_->isVisible()) effListDialog_->activateWindow();
		else effListDialog_->show();
	}
	else {
		effListDialog_ = std::make_unique<EffectListDialog>(this);
		effListDialog_->show();
	}
}

void MainWindow::on_actionShortcuts_triggered()
{
	if (shortcutsDialog_) {
		if (shortcutsDialog_->isVisible()) shortcutsDialog_->activateWindow();
		else shortcutsDialog_->show();
	}
	else {
		shortcutsDialog_ = std::make_unique<KeyboardShortcutListDialog>(this);
		shortcutsDialog_->show();
	}
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

		std::unordered_map<int, int> rplMap = bt_->replaceDuplicateInstrumentsInPatterns();
		auto list = ui->instrumentList;
		for (auto& pairs : rplMap) {
			for (int j = 0; j < list->count(); ++j) {
				if (list->item(j)->data(Qt::UserRole).toInt() == pairs.first)
					removeInstrument(j);
			}
		}
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
	GoToDialog dialog(bt_, this);
	if (dialog.exec() == QDialog::Accepted) {
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
			bt_->setCurrentOrderNumber(dialog.getOrder());
			bt_->setCurrentStepNumber(dialog.getStep());
			bt_->setCurrentTrack(dialog.getTrack());
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

void MainWindow::on_actionNew_Drumki_t_triggered()
{
	addDrumkit();
}

void MainWindow::on_action_Wave_View_triggered(bool checked)
{
	config_.lock()->setVisibleWaveView(checked);
	ui->waveVisual->setVisible(checked);
	if (checked)
		visualTimer_->start(static_cast<int>(std::round(1000. / config_.lock()->getWaveViewFrameRate())));
	else
		visualTimer_->stop();
}

void MainWindow::on_action_Transpose_Song_triggered()
{
	TransposeSongDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted) {
		if (showUndoResetWarningDialog(tr("Do you want to transpose a song?"))) {
			bt_->stopPlaySong();
			lockWidgets(false);

			bt_->transposeSong(bt_->getCurrentSongNumber(),
							   dialog.getTransposeSeminotes(), dialog.getExcludeInstruments());
			ui->patternEditor->onPatternDataGlobalChanged();

			bt_->clearCommandHistory();
			comStack_->clear();
			setModifiedTrue();
		}
	}
}

void MainWindow::on_action_Swap_Tracks_triggered()
{
	SwapTracksDialog dialog(bt_->getSongStyle(bt_->getCurrentSongNumber()), this);
	if (dialog.exec() == QDialog::Accepted) {
		int track1 = dialog.getTrack1();
		int track2 = dialog.getTrack2();
		if ((track1 != track2) && showUndoResetWarningDialog(tr("Do you want to swap tracks?"))) {
			bt_->stopPlaySong();
			lockWidgets(false);

			bt_->swapTracks(bt_->getCurrentSongNumber(), track1, track2);
			ui->orderList->onOrderDataGlobalChanged();
			ui->patternEditor->onPatternDataGlobalChanged();

			bt_->clearCommandHistory();
			comStack_->clear();
			setModifiedTrue();
		}
	}
}

void MainWindow::on_action_Insert_triggered()
{
	if (isEditedPattern_) ui->patternEditor->onPasteInsertPressed();
}

void MainWindow::on_action_Hide_Tracks_triggered()
{
	HideTracksDialog dialog(bt_->getSongStyle(bt_->getCurrentSongNumber()),
						  ui->patternEditor->getVisibleTracks(), this);
	if (dialog.exec() == QDialog::Accepted) {
		std::vector<int> visTracks = dialog.getVisibleTracks();
		ui->orderList->setVisibleTracks(visTracks);
		setOrderListGroupMaximumWidth();
		ui->patternEditor->setVisibleTracks(visTracks);

		int song = bt_->getCurrentSongNumber();
		int all = static_cast<int>(bt_->getSongStyle(song).trackAttribs.size());
		for (int i = 0; i < all; ++i) {
			if (std::any_of(visTracks.begin(), visTracks.end(), [i](int n) { return i == n; })) {
				bt_->setTrackVisibility(song, i, true);
			}
			else {
				bt_->setTrackVisibility(song, i, false);
				if (config_.lock()->getMuteHiddenTracks()) {
					bt_->setTrackMuteState(i, true);
				}
			}
		}

		setModifiedTrue();
	}
}

void MainWindow::on_action_Estimate_Song_Length_triggered()
{
	double time = bt_->getApproximateSongLength(bt_->getCurrentSongNumber());
	int seconds = static_cast<int>(std::round(time));
	QMessageBox dialog;
	dialog.setIcon(QMessageBox::Information);
	dialog.setText(tr("Approximate song length: %1m%2s")
				.arg(seconds / 60).arg(seconds % 60, 2, 10, QChar('0')));
	dialog.exec();
}

void MainWindow::on_action_Key_Signature_Manager_triggered()
{
	if (ksManForm_->isVisible()) ksManForm_->activateWindow();
	else ksManForm_->show();
}

void MainWindow::on_action_Welcome_triggered()
{
	welcomeDialog_->open();
}
