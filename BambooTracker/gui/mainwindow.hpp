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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <atomic>
#include <memory>
#include <cstdint>
#include <QMainWindow>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QWidget>
#include <QUndoStack>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QShowEvent>
#include <QMessageBox>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include <QLibrary>
#include <QShortcut>
#include <QAction>
#include <QActionGroup>
#include <QLineEdit>
#include "enum_hash.hpp"
#include "configuration.hpp"
#include "bamboo_tracker.hpp"
#include "precise_timer.hpp"
#include "audio/audio_stream.hpp"
#include "gui/instrument_editor/instrument_form_manager.hpp"
#include "gui/color_palette.hpp"
#include "gui/file_history.hpp"
#include "gui/effect_list_dialog.hpp"
#include "gui/keyboard_shortcut_list_dialog.hpp"
#include "gui/bookmark_manager_form.hpp"
#include "gui/instrument_selection_dialog.hpp"
#include "gui/comment_edit_dialog.hpp"
#include "gui/key_signature_manager_form.hpp"

namespace Ui {
	class ModuleSaveCheckDialog;
	class MainWindow;
}

class ModuleSaveCheckDialog : public QMessageBox
{
  Q_OBJECT

public:
  explicit ModuleSaveCheckDialog(const std::string& name, QWidget* parent = nullptr);
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(std::weak_ptr<Configuration> config, QString filePath, bool isFirstLaunch, QWidget *parent = nullptr);
	~MainWindow() override;

	void assignADPCMSamples();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void showEvent(QShowEvent*) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void moveEvent(QMoveEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

	// Midi
private:
	static void midiThreadReceivedEvent(double delay, const uint8_t *msg, size_t len, void *userData);
private slots:
	void midiKeyEvent(uchar status, uchar key, uchar velocity);
	void midiProgramEvent(uchar status, uchar program);

private:
	std::unique_ptr<Ui::MainWindow> ui;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;
	std::shared_ptr<BambooTracker> bt_;
	std::shared_ptr<AudioStream> stream_;
	std::unique_ptr<PreciseTimer> tickTimerForRealChip_;
	std::unique_ptr<QTimer> visualTimer_;
	std::shared_ptr<QUndoStack> comStack_;
	std::shared_ptr<FileHistory> fileHistory_;

	std::shared_ptr<QLibrary> scciDll_, c86ctlDll_;

	// Instrument list
	std::shared_ptr<InstrumentFormManager> instForms_;
	QListWidgetItem* renamingInstItem_;
	QLineEdit* renamingInstEdit_;
	void addInstrument();
	void addDrumkit();
	void removeInstrument(int row);
	void openInstrumentEditor();
	int findRowFromInstrumentList(int instNum);
	void renameInstrument();
	void finishRenamingInstrument();
	void cloneInstrument();
	void deepCloneInstrument();
	void loadInstrument();
	void funcLoadInstrument(QString file);
	void saveInstrument();
	void importInstrumentsFromBank();
	void funcImportInstrumentsFromBank(QString file);
	void exportInstrumentsToBank();
	void swapInstruments(int row1, int row2);

	// Undo-Redo
	void undo();
	void redo();
	bool isModifiedForNotCommand_;

	// Load data
	void loadModule();
	void openModule(QString file);
	void loadSong();

	// Play song
	void startPlaySong();
	void startPlayFromStart();
	void startPlayPattern();
	void startPlayFromCurrentStep();
	void startPlayFromMarker();
	void playStep();
	void stopPlaySong();
	bool hasLockedWigets_;
	void lockWidgets(bool isLock);

	// Octave change
	void changeOctave(bool upFlag);

	// Configuration change
	void changeConfiguration();
	void setRealChipInterface(RealChipInterfaceType intf);
	void setMidiConfiguration();
	void updateFonts();

	// History change
	void changeFileHistory(QString file);

	// Backup
	bool backupModule(QString srcFile);

	void setWindowTitle();
	void setModifiedTrue();
	void setInitialSelectedInstrument();
	QString getModuleFileBaseName() const;

	bool isEditedPattern_, isEditedOrder_, isEditedInstList_;
	bool isSelectedPattern_, isSelectedOrder_;

	bool hasShownOnce_;

	bool isSavedModBefore_;

	bool firstViewUpdateRequest_;

	// Menus
	std::unique_ptr<QActionGroup> pasteModeGroup_;

	// Toolbar
	QSpinBox *octave_, *highlight1_, *highlight2_, *volume_;

	// Status bars
	QLabel *statusStyle_, *statusInst_, *statusOctave_;
	QLabel *statusIntr_, *statusMixer_, *statusBpm_, *statusPlayPos_;

	// Shortcuts
	QAction octUpSc_, octDownSc_;
	QShortcut focusPtnSc_, focusOdrSc_, focusInstSc_;
	std::unique_ptr<QShortcut> instAddSc_;
	QAction playAndStopSc_, playStepSc_, goPrevOdrSc_, goNextOdrSc_, prevInstSc_, nextInstSc_;
	QAction incPtnSizeSc_, decPtnSizeSc_, incEditStepSc_, decEditStepSc_, prevSongSc_, nextSongSc_;
	QAction jamVolUpSc_, jamVolDownSc_;
	void setShortcuts();

	// Dialogs
	std::unique_ptr<EffectListDialog> effListDialog_;
	std::unique_ptr<KeyboardShortcutListDialog> shortcutsDialog_;
	std::unique_ptr<BookmarkManagerForm> bmManForm_;
	std::unique_ptr<CommentEditDialog> commentDialog_;
	std::unique_ptr<KeySignatureManagerForm> ksManForm_;
	std::unique_ptr<QMessageBox> welcomeDialog_;

	// Bank import
	std::atomic_bool bankJamMidiCtrl_;
	std::unique_ptr<InstrumentSelectionDialog> importBankDialog_;

	// Meta methods
	int tickEventMethod_;
	int midiKeyEventMethod_;
	int midiProgramEventMethod_;

	void updateInstrumentListColors();
	void setOrderListGroupMaximumWidth();
	void freezeViews();
	void unfreezeViews();

	inline bool showUndoResetWarningDialog(QString text)
	{
		return (QMessageBox::warning(this, tr("Warning"),
									 tr("%1 If you execute this command, the command history is reset.").arg(text),
									 QMessageBox::Yes | QMessageBox::No,
									 QMessageBox::No) == QMessageBox::Yes);
	}

	inline void showStreamFailedDialog(const QString& errDetail)
	{
		QMessageBox::critical(this, tr("Error"),
							  tr("Could not open the audio stream. Please change the sound settings in Configuration.")
							  + "\n\n" + errDetail,
							  QMessageBox::Ok, QMessageBox::Ok);
	}

	inline void showStreamRateWarningDialog(uint32_t curRate)
	{
		QMessageBox::warning(this, tr("Warning"),
							 tr("Could not set the sample rate of the audio stream to %1Hz. Currently the stream runs on %2Hz instead.")
							 .arg(config_.lock()->getSampleRate()).arg(curRate),
							 QMessageBox::Ok, QMessageBox::Ok);
	}

	inline void showMidiFailedDialog(const QString& errDetail)
	{
		QMessageBox::critical(this, tr("Error"),
							  tr("Could not initialize MIDI input.")
							  + "\n\n" + errDetail,
							  QMessageBox::Ok, QMessageBox::Ok);
	}

private slots:
	void on_instrumentList_customContextMenuRequested(const QPoint& pos);
	void on_instrumentList_itemDoubleClicked(QListWidgetItem*);
	void on_instrumentList_itemSelectionChanged();
	void on_grooveCheckBox_stateChanged(int arg1);
	void on_actionExit_triggered();
	void on_actionUndo_triggered();
	void on_actionRedo_triggered();
	void on_actionCut_triggered();
	void on_actionCopy_triggered();
	void on_actionPaste_triggered();
	void on_actionDelete_triggered();
	void updateMenuByPattern();
	void updateMenuByOrder();
	void onCurrentTrackChanged();
	void updateMenuByInstrumentList();
	void updateMenuByPatternSelection(bool isSelected);
	void updateMenuByOrderSelection(bool isSelected);
	void on_actionAll_triggered();
	void on_actionNone_triggered();
	void on_actionDecrease_Note_triggered();
	void on_actionIncrease_Note_triggered();
	void on_actionDecrease_Octave_triggered();
	void on_actionIncrease_Octave_triggered();
	void on_actionInsert_Order_triggered();
	void on_actionRemove_Order_triggered();
	void on_actionModule_Properties_triggered();
	void on_actionNew_Instrument_triggered();
	void on_actionRemove_Instrument_triggered();
	void on_actionClone_Instrument_triggered();
	void on_actionDeep_Clone_Instrument_triggered();
	void on_actionEdit_triggered();
	void on_actionPlay_triggered();
	void on_actionPlay_Pattern_triggered();
	void on_actionPlay_From_Start_triggered();
	void on_actionPlay_From_Cursor_triggered();
	void on_actionStop_triggered();
	void on_actionEdit_Mode_triggered();
	void on_actionToggle_Track_triggered();
	void on_actionSolo_Track_triggered();
	void on_actionKill_Sound_triggered();
	void on_actionAbout_triggered();
	void on_actionFollow_Mode_triggered();
	void on_actionGroove_Settings_triggered();
	void on_actionConfiguration_triggered();
	void on_actionExpand_triggered();
	void on_actionShrink_triggered();
	void on_actionDuplicate_Order_triggered();
	void on_actionMove_Order_Up_triggered();
	void on_actionMove_Order_Down_triggered();
	void on_actionClone_Patterns_triggered();
	void on_actionClone_Order_triggered();
	void on_actionNew_triggered();
	void on_actionComments_triggered();
	bool on_actionSave_triggered();
	bool on_actionSave_As_triggered();
	void on_actionOpen_triggered();
	void on_actionLoad_From_File_triggered();
	void on_actionSave_To_File_triggered();
	void on_actionImport_From_Bank_File_triggered();
	void on_actionInterpolate_triggered();
	void on_actionReverse_triggered();
	void on_actionReplace_Instrument_triggered();
	void on_actionRow_triggered();
	void on_actionColumn_triggered();
	void on_actionPattern_triggered();
	void on_actionOrder_triggered();
	void on_actionRemove_Unused_Instruments_triggered();
	void on_actionRemove_Unused_Patterns_triggered();
	void on_actionWAV_triggered();
	void on_actionVGM_triggered();
	void on_actionS98_triggered();
	void on_actionMix_triggered();
	void on_actionOverwrite_triggered();
	void onNewTickSignaledRealChip();
	void onNewTickSignaled(int state);
	void on_actionClear_triggered();
	void on_keyRepeatCheckBox_stateChanged(int arg1);
	void updateVisuals();
	void on_action_Effect_List_triggered();
	void on_actionShortcuts_triggered();
	void on_actionExport_To_Bank_File_triggered();
	void on_actionRemove_Duplicate_Instruments_triggered();
	void on_actionRename_Instrument_triggered();
	void on_action_Bookmark_Manager_triggered();
	void on_actionFine_Decrease_Values_triggered();
	void on_actionFine_Increase_Values_triggered();
	void on_actionCoarse_D_ecrease_Values_triggered();
	void on_actionCoarse_I_ncrease_Values_triggered();
	void on_action_Toggle_Bookmark_triggered();
	void on_action_Next_Bookmark_triggered();
	void on_action_Previous_Bookmark_triggered();
	void on_action_Instrument_Mask_triggered();
	void on_action_Volume_Mask_triggered();
	void on_actionSet_Ro_w_Marker_triggered();
	void on_actionPlay_From_Marker_triggered();
	void on_action_Go_To_triggered();
	void on_actionRemove_Unused_ADPCM_Samples_triggered();
	void on_action_Status_Bar_triggered();
	void on_action_Toolbar_triggered();
	void on_actionNew_Drumki_t_triggered();
	void on_action_Wave_View_triggered(bool checked);
	void on_action_Transpose_Song_triggered();
	void on_action_Swap_Tracks_triggered();
	void on_action_Insert_triggered();
	void on_action_Hide_Tracks_triggered();
	void on_action_Estimate_Song_Length_triggered();
	void on_action_Key_Signature_Manager_triggered();
	void on_action_Welcome_triggered();
};

#endif // MAINWINDOW_HPP
