#include "q_application_wrapper.hpp"
#include <QMessageBox>

QApplicationWrapper::QApplicationWrapper(int& argc, char** argv) : QApplication (argc, argv) {}

bool QApplicationWrapper::notify(QObject* receiver, QEvent* event)
{
	try {
		return QApplication::notify(receiver, event);
	}
	catch (...) {
		QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("An unknown error occured."));
		return false;
	}
}
