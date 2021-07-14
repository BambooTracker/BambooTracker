/*
 * Copyright (C) 2020-2021 Rerrah
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

namespace
{
const std::unordered_map<io::FileType, QString> FILE_NAMES = {
	{ io::FileType::Mod, QT_TRANSLATE_NOOP("FileType", "module") },
	{ io::FileType::S98, QT_TRANSLATE_NOOP("FileType", "s98") },
	{ io::FileType::VGM, QT_TRANSLATE_NOOP("FileType", "vgm") },
	{ io::FileType::WAV, QT_TRANSLATE_NOOP("FileType", "wav") },
	{ io::FileType::Bank, QT_TRANSLATE_NOOP("FileType", "bank") },
	{ io::FileType::Inst, QT_TRANSLATE_NOOP("FileType", "instrument") }
};
}

FileIOErrorMessageBox::FileIOErrorMessageBox(const QString& file, bool isInput, io::FileType ftype, const QString desc, QWidget* parent)
	: parent_(parent), desc_(desc)
{
	setText(file, isInput, ftype);
}

FileIOErrorMessageBox::FileIOErrorMessageBox(const QString& file, bool isInput, const io::FileIOError& e, QWidget* parent)
	: parent_(parent)
{
	const io::FileIOError *err = &e;
	QString type = FILE_NAMES.at(err->fileType());

	if (dynamic_cast<const io::FileNotExistError*>(err)) {
		desc_ = tr("Path does not exist.");
	}
	else if (dynamic_cast<const io::FileUnsupportedError*>(err)) {
		desc_ = tr("Unsupported file format.");
	}
	else if (dynamic_cast<const io::FileVersionError*>(err)) {
		desc_ = tr("Could not load the %1 properly. "
				   "Please make sure that you have the latest version of BambooTracker.").arg(file);
	}
	else if (auto ce = dynamic_cast<const io::FileCorruptionError*>(err)) {
		desc_ = tr("Could not load the %1. It may be corrupted. Stopped at %2.").arg(type).arg(ce->position());
	}

	setText(file, isInput, e.fileType());
}

void FileIOErrorMessageBox::setText(const QString& file, bool isInput, io::FileType ftype)
{
	if (isInput) {
		text_ = tr("Failed to load %1.").arg(file);
	}
	else {
		switch (ftype) {
		case io::FileType::S98:
		case io::FileType::VGM:
		case io::FileType::WAV:
			text_ = tr("Failed to export to %1.");
			break;
		default:
			text_ = tr("Failed to save the %1.");
			break;
		}
		text_ = text_.arg(file);
	}
}

void FileIOErrorMessageBox::exec()
{
	QMessageBox::critical(parent_, tr("Error"), text_ + "\n" + desc_);
}
