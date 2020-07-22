#include "q_application_wrapper.hpp"
#include <exception>
#include <QMessageBox>

QApplicationWrapper::QApplicationWrapper(int& argc, char** argv) : QApplication (argc, argv) {}

bool QApplicationWrapper::notify(QObject* receiver, QEvent* event)
{
	try {
		return QApplication::notify(receiver, event);
	}
	catch (std::exception& e) {
		QMessageBox::critical(nullptr, QObject::tr("Error"),
							  QObject::tr("An unknown error occurred.\n%1").arg(e.what()));
		return false;
	}
}
