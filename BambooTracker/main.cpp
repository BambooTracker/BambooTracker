#include "./gui/mainwindow.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include <memory>

int main(int argc, char *argv[])
{
	try {
		std::unique_ptr<QApplication> a(std::make_unique<QApplication>(argc, argv));
		std::unique_ptr<MainWindow> w(std::make_unique<MainWindow>());
		w->show();
		return a->exec();
	} catch (...) {
		QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("An unknown error occured."));
		return -1;
	}
}
