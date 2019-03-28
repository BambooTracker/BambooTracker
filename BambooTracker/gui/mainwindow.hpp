#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

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
#include <QMessageBox>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include <QLibrary>
#include "configuration.hpp"
#include "bamboo_tracker.hpp"
#include "audio_stream.hpp"
#include "gui/instrument_editor/instrument_form_manager.hpp"
#include "gui/color_palette.hpp"
/*#include "timer.hpp"*/
#include "gui/file_history.hpp"

class AbstractBank;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(std::weak_ptr<Configuration> config, QString filePath, QWidget *parent = nullptr);
	~MainWindow() override;

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;
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

private:
	std::unique_ptr<Ui::MainWindow> ui;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;
	std::shared_ptr<BambooTracker> bt_;
	std::shared_ptr<AudioStream> stream_;
	std::unique_ptr<QTimer> timer_;
	/*std::unique_ptr<Timer> timer_;*/
	std::shared_ptr<QUndoStack> comStack_;
	std::shared_ptr<FileHistory> fileHistory_;

	std::unique_ptr<QLibrary> scciDll_;

	// Instrument list
	std::shared_ptr<InstrumentFormManager> instForms_;
	void addInstrument();
	void removeInstrument(int row);
	void editInstrument();
	int findRowFromInstrumentList(int instNum);
	void editInstrumentName();
	void cloneInstrument();
    void deepCloneInstrument();
	void loadInstrument();
	void saveInstrument();
	void importInstrumentsFromBank();

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
	void stopPlaySong();
	void lockControls(bool isLock);

	// Octave change
	void changeOctave(bool upFlag);

	// Configuration change
	void changeConfiguration();
	void setMidiConfiguration();

	// History change
	void changeFileHistory(QString file);

	// Layout decypherer
	JamKey getJamKeyFromLayoutMapping(Qt::Key key);

	void setWindowTitle();
	void setModifiedTrue();
	void setInitialSelectedInstrument();

	bool isEditedPattern_, isEditedOrder_, isEditedInstList_;
	bool isSelectedPO_;

	bool isSavedModBefore_;

	// Sub tool bar
	QSpinBox* octave_;
	QSpinBox *highlight1_, *highlight2_;

	// Status bar
	QLabel* statusDetail_;
	QLabel* statusStyle_;
	QLabel* statusInst_;
	QLabel* statusOctave_;
	QLabel* statusIntr_;
	QLabel* statusBpm_;
	QLabel* statusPlayPos_;

	// Meta methods
	int midiKeyEventMethod_;

private slots:
	void on_instrumentListWidget_customContextMenuRequested(const QPoint &pos);
	void on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item);
	void onInstrumentListWidgetItemAdded(const QModelIndex& parent, int start, int end);
	void on_instrumentListWidget_itemSelectionChanged();
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
	void updateMenuByInstrumentList();
	void updateMenuByPatternAndOrderSelection(bool isSelected);
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
	void onNewTickSignaled();
	void on_actionClear_triggered();
	void on_keyRepeatCheckBox_stateChanged(int arg1);

	inline bool showUndoResetWarningDialog(QString text)
	{
		return (QMessageBox::warning(this, "Warning",
									text + " If you execute this command, the command history is reset.",
									QMessageBox::Yes | QMessageBox::No,
									QMessageBox::No) == QMessageBox::Yes);
	}
};

#endif // MAINWINDOW_HPP
