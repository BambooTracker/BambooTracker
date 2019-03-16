#include "./gui/mainwindow.hpp"
#include <memory>
#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QObject>
#include <QDebug>
#include <QString>
#include "configuration.hpp"
#include "gui/configuration_handler.hpp"

// Localization
static void setupTranslations();
static QString findQtTranslationsDir();
static QString findAppTranslationsDir();

int main(int argc, char *argv[])
{
	try {
		QString filePath = (argc > 1) ? argv[argc - 1] : "";	// Last argument file
		std::shared_ptr<Configuration> config = std::make_shared<Configuration>();
		ConfigurationHandler::loadConfiguration(config);
		std::unique_ptr<QApplication> a(std::make_unique<QApplication>(argc, argv));
		if (config->getEnableTranslation()) setupTranslations();
		std::unique_ptr<MainWindow> w(std::make_unique<MainWindow>(config, filePath));
		w->show();
		int ret = a->exec();
		ConfigurationHandler::saveConfiguration(config);
		return ret;
	} catch (...) {
		QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("An unknown error occured."));
		return -1;
	}
}

// Sets up the translation according to the current language
static void setupTranslations()
{
	QApplication *a = qApp;
	const QString lang = QLocale::system().name();

	QTranslator *qtTr = new QTranslator(a);
	QTranslator *appTr = new QTranslator(a);

	QString qtDir = findQtTranslationsDir();
	QString appDir = findAppTranslationsDir();

	if (!qtDir.isEmpty()) {
		QString baseName = "qt_" + lang;
		qtTr->load(baseName, qtDir);
		qDebug() << "Translation" << baseName << "from" << qtDir;
	}

	if (!appDir.isEmpty()) {
		QString baseName = "bamboo_tracker_" + lang;
		appTr->load(baseName, appDir);
		qDebug() << "Translation" << baseName << "from" << appDir;
	}

	a->installTranslator(qtTr);
	a->installTranslator(appTr);
}

// Finds the location of Qt translation catalogs
static QString findQtTranslationsDir()
{
#if defined(Q_OS_DARWIN)
	// if this is macOS, attempt to load from inside an app bundle
	QString pathInAppBundle = QApplication::applicationDirPath() + "/../Resources/lang";
	if (QDir(pathInAppBundle).exists())
		return pathInAppBundle;
#endif

#if defined(Q_OS_WIN)
	// if this is Windows, translations should be distributed with the program
	return QApplication::applicationDirPath() + "/lang";
#else
	// the files are located in the installation of Qt
	return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
}

// Finds the location of our translation catalogs
static QString findAppTranslationsDir()
{
#ifndef QT_NO_DEBUG
	// if this is a debug build, attempt to load from the source directory
	QString pathInSources = QApplication::applicationDirPath() + "/.qm";
	if (QDir(pathInSources).exists())
		return pathInSources;
#endif

#if defined(Q_OS_DARWIN)
	// if this is macOS, attempt to load from inside an app bundle
	QString pathInAppBundle = QApplication::applicationDirPath() + "/../Resources/lang";
	if (QDir(pathInAppBundle).exists())
		return pathInAppBundle;
#endif

#if defined(Q_OS_WIN)
	// if this is Windows, translations should be distributed with the program
	return QApplication::applicationDirPath() + "/lang";
#else
	return QApplication::applicationDirPath() + "/../share/BambooTracker/lang";
#endif
}
