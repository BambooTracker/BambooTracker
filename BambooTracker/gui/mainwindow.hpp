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
#include "bamboo_tracker.hpp"
#include "audio_stream.hpp"
#include "gui/instrument_editor/instrument_form_manager.hpp"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void closeEvent(QCloseEvent* ce) override;

private:
	Ui::MainWindow *ui;

	std::shared_ptr<BambooTracker> bt_;
	std::unique_ptr<AudioStream> stream_;
	std::shared_ptr<QUndoStack> comStack_;

	// Instrument list
	std::shared_ptr<InstrumentFormManager> instForms_;
	void addInstrument();
	void removeInstrument();
	void editInstrument();
	int findRowFromInstrumentList(int instNum);
	void editInstrumentName();
	void cloneInstrument();
    void deepCloneInstrument();

	// Undo-Redo
	void undo();
	void redo();
	bool isModifiedForNotCommand_;

	// Load data
	void loadModule();
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

	void setWindowTitle();
	void setModifiedTrue();

	bool isEditedPattern_, isEditedOrder_;
	bool isSelectedPO_;

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
	void on_actionPaste_Mix_triggered();
	void on_actionDelete_triggered();
	void updateMenuByPattern();
	void updateMenuByOrder();
	void onPatternAndOrderFocusLost();
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
	void on_actionMute_Track_triggered();
	void on_actionSolo_Track_triggered();
	void on_actionKill_Sound_triggered();
	void on_actionAbout_triggered();
	void on_actionFollow_Mode_triggered();
};

#endif // MAINWINDOW_HPP
