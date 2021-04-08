/*
 * Copyright (C) 2018-2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "./gui/mainwindow.hpp"
#include <memory>
#include <exception>
#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QObject>
#include <QDebug>
#include <QString>
#include "configuration.hpp"
#include "gui/q_application_wrapper.hpp"
#include "gui/configuration_handler.hpp"

namespace
{
// Localization
static void setupTranslations();
static QString findQtTranslationsDir();
static QString findAppTranslationsDir();
}

int main(int argc, char* argv[])
{
	try {
		std::shared_ptr<Configuration> config = std::make_shared<Configuration>();
		io::loadConfiguration(config);

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

		std::unique_ptr<QApplicationWrapper> a(std::make_unique<QApplicationWrapper>(argc, argv));
		if (config->getEnableTranslation()) setupTranslations();
		a->setWindowIcon(QIcon(":/icon/app_icon"));

		QString filePath = (argc > 1) ? argv[argc - 1] : "";	// Last argument file

		std::unique_ptr<MainWindow> w(std::make_unique<MainWindow>(config, filePath));
		w->show();
		int ret = a->exec();

		io::saveConfiguration(config);
		if (ret) QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("An unknown error occurred."));

		return ret;
	}
	catch (std::exception& e) {
		QMessageBox::critical(nullptr, QObject::tr("Error"),
							  QObject::tr("An unknown error occurred.\n%1").arg(e.what()));
		return 1;
	}
}

namespace
{
// Sets up the translation according to the current language
void setupTranslations()
{
	QApplication *a = qApp;
	const QString lang = QLocale::system().name();

	QTranslator *qtTr = new QTranslator(a);
	QTranslator *appTr = new QTranslator(a);

	QString qtDir = findQtTranslationsDir();
	QString appDir = findAppTranslationsDir();

	if (!qtDir.isEmpty()) {
		QString baseName = "qt_" + lang;
		if (qtTr->load(baseName, qtDir)) {
			qDebug() << "Translation" << baseName << "from" << qtDir;
		}
	}

	if (!appDir.isEmpty()) {
		QString baseName = "bamboo_tracker_" + lang;
		if (appTr->load(baseName, appDir)) {
			qDebug() << "Translation" << baseName << "from" << appDir;
		}
	}

	a->installTranslator(qtTr);
	a->installTranslator(appTr);
}

// Finds the location of Qt translation catalogs
QString findQtTranslationsDir()
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	return QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
	return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
#endif
}

// Finds the location of our translation catalogs
QString findAppTranslationsDir()
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
}
