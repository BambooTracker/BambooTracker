#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <memory>
#include <map>
#include <QMainWindow>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QWidget>
#include <QUndoStack>
#include "bamboo_tracker.hpp"
#include "audio_stream.hpp"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::MainWindow *ui;

	std::unique_ptr<BambooTracker> bt_;
	std::unique_ptr<AudioStream> stream_;
	std::unique_ptr<QUndoStack> comStack_;

	//*********** dummy
	int channel_ = 0;
	//********************

	// Instrument list
	std::map<int, std::unique_ptr<QWidget>> instFormMap_;
	void addInstrument();
	void removeInstrument();
	void editInstrument();

	// Undo-Redo
	void undo();
	void redo();

	// Play song
	bool isPlaySong_;
	void startPlaySong();
	void stopPlaySong();

private slots:
	void on_instrumentListWidget_customContextMenuRequested(const QPoint &pos);
	void on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item);
};

#endif // MAINWINDOW_HPP
