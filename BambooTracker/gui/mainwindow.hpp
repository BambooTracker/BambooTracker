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
	void startPlayPattern();
	void startPlayFromCurrentStep();
	void stopPlaySong();
	void lockControls(bool isLock);

	// Kill sound
	void killSound();

	// Octave change
	void changeOctave(bool upFlag);

	// Toggle Jam mode
	void toggleJamMode();

	void setWindowTitle();
	void setModifiedTrue();

private slots:
	void on_instrumentListWidget_customContextMenuRequested(const QPoint &pos);
	void on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item);
	void onInstrumentListWidgetItemAdded(const QModelIndex& parent, int start, int end);
	void on_instrumentListWidget_itemSelectionChanged();
	void on_modSetDialogOpenToolButton_clicked();
	void on_grooveCheckBox_stateChanged(int arg1);
};

#endif // MAINWINDOW_HPP
