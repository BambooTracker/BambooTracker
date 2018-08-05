#include "./gui/mainwindow.hpp"
#include <QApplication>
#include <memory>

int main(int argc, char *argv[])
{
	std::unique_ptr<QApplication> a(std::make_unique<QApplication>(argc, argv));
	std::unique_ptr<MainWindow> w(std::make_unique<MainWindow>());
	w->show();

	int ret = a->exec();

	return ret;
}
