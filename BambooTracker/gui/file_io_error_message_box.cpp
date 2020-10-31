/*
 * Copyright (C) 2020 Rerrah
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

#include "file_io_error_message_box.hpp"
#include <QMessageBox>

const std::unordered_map<FileIO::FileType, QString> FileIOErrorMessageBox::FILE_NAMES_ = {
	{ FileIO::FileType::Mod, QT_TR_NOOP("module") },
	{ FileIO::FileType::S98, QT_TR_NOOP("s98") },
	{ FileIO::FileType::VGM, QT_TR_NOOP("vgm") },
	{ FileIO::FileType::WAV, QT_TR_NOOP("wav") },
	{ FileIO::FileType::Bank, QT_TR_NOOP("bank") },
	{ FileIO::FileType::Inst, QT_TR_NOOP("instrument") }
};

FileIOErrorMessageBox::FileIOErrorMessageBox(const QString& file, bool isInput, FileIO::FileType ftype, const QString desc, QWidget* parent)
	: parent_(parent), desc_(desc)
{
	setText(file, isInput, ftype);
}

FileIOErrorMessageBox::FileIOErrorMessageBox(const QString& file, bool isInput, const FileIOError& e, QWidget* parent)
	: parent_(parent)
{
	const FileIOError *err = &e;
	QString type = FILE_NAMES_.at(err->fileType());

	if (dynamic_cast<const FileNotExistError*>(err)) {
		desc_ = QObject::tr("Path does not exist.");
	}
	else if (dynamic_cast<const FileUnsupportedError*>(err)) {
		desc_ = QObject::tr("Unsupported file format.");
	}
	else if (dynamic_cast<const FileVersionError*>(err)) {
		desc_ = QObject::tr("Could not load the %1 properly. "
							"Please make sure that you have the latest version of BambooTracker.").arg(file);
	}
	else if (auto ce = dynamic_cast<const FileCorruptionError*>(err)) {
		desc_ = QObject::tr("Could not load the %1. It may be corrupted. Stopped at %2.").arg(type).arg(ce->position());
	}

	setText(file, isInput, e.fileType());
}

void FileIOErrorMessageBox::setText(const QString& file, bool isInput, FileIO::FileType ftype)
{
	if (isInput) {
		text_ = QObject::tr("Failed to load %1.").arg(file);
	}
	else {
		switch (ftype) {
		case FileIO::FileType::S98:
		case FileIO::FileType::VGM:
		case FileIO::FileType::WAV:
			text_ = QObject::tr("Failed to export to %1.");
			break;
		default:
			text_ = QObject::tr("Failed to save the %1.");
			break;
		}
		text_ = text_.arg(file);
	}
}

void FileIOErrorMessageBox::exec()
{
	QMessageBox::critical(parent_, QObject::tr("Error"), text_ + "\n" + desc_);
}
