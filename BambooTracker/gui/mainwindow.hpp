#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <memory>
#include <QMainWindow>
#include <QKeyEvent>
#include <QListWidgetItem>
#include "bamboo_tracker.hpp"

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

	//*********** dummy
	int channel_ = 0;
	//********************

	// Voice list
	void addVoice();
	void removeVoice();
	void editVoice();

private slots:
	void on_voiceListWidget_customContextMenuRequested(const QPoint &pos);
	void on_voiceListWidget_itemDoubleClicked(QListWidgetItem *item);
};

#endif // MAINWINDOW_HPP
