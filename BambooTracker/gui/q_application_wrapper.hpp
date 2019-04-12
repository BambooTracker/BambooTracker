#ifndef Q_APPLICATION_WRAPPER_HPP
#define Q_APPLICATION_WRAPPER_HPP

#include <QApplication>

class QApplicationWrapper : public QApplication
{
public:
	QApplicationWrapper(int& argc, char** argv);
	bool notify(QObject* receiver, QEvent* event) override;
};

#endif // Q_APPLICATION_WRAPPER_HPP
