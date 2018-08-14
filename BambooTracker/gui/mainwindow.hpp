#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <memory>
#include <map>
#include <QMainWindow>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QWidget>
#include <QUndoStack>
#include <QCloseEvent>
#include "bamboo_tracker.hpp"
#include "audio_stream.hpp"

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
	std::map<int, std::unique_ptr<QWidget>> instFormMap_;
	void addInstrument();
	void removeInstrument();
	void editInstrument();
	int findRowFromInstrumentList(int instNum);
	void editInstrumentName();
    void copyInstrument();
    void pasteInstrument();
    void cloneInstrument();

	// Undo-Redo
	void undo();
	void redo();

	// Play song
	void startPlaySong();
	void startPlayPattern();
	void startPlayFromCurrentStep();
	void stopPlaySong();

	// Octave change
	void changeOctave(bool upFlag);

	// Toggle Jam mode
	void toggleJamMode();

private slots:
	void on_instrumentListWidget_customContextMenuRequested(const QPoint &pos);
	void on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item);
	void onInstrumentListWidgetItemAdded(const QModelIndex& parent, int start, int end);
	void on_instrumentListWidget_itemSelectionChanged();
	void onInstrumentFMEnvelopeChanged(int envNum, int fromInstNum);
};

#endif // MAINWINDOW_HPP
