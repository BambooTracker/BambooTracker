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

	// Instrument list
	void addInstrument();
	void removeInstrument();
	void editInstrument();

private slots:
	void on_instrumentListWidget_customContextMenuRequested(const QPoint &pos);
	void on_instrumentListWidget_itemDoubleClicked(QListWidgetItem *item);
};

#endif // MAINWINDOW_HPP
