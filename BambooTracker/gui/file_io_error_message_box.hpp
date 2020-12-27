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

#ifndef FILE_IO_ERROR_MESSAGE_BOX_HPP
#define FILE_IO_ERROR_MESSAGE_BOX_HPP

#include <unordered_map>
#include <QString>
#include <QWidget>
#include "io/io_file_type.hpp"
#include "io/file_io_error.hpp"
#include "enum_hash.hpp"

class FileIOErrorMessageBox : public QObject
{
	Q_OBJECT

public:
	FileIOErrorMessageBox(const QString& file, bool isInput, io::FileType ftype, const QString desc, QWidget* parent = nullptr);
	FileIOErrorMessageBox(const QString& file, bool isInput, const io::FileIOError& e, QWidget* parent = nullptr);
	void exec();

	inline static void openError(const QString& file, bool isInput, io::FileType ftype, QWidget* parent = nullptr)
	{
		FileIOErrorMessageBox(file, isInput, ftype, tr("Could not open the file."), parent).exec();
	}

private:
	QWidget* parent_;
	QString text_, desc_;

	static const std::unordered_map<io::FileType, QString> FILE_NAMES_;

	void setText(const QString& file, bool isInput, io::FileType ftype);
};

#endif // FILE_IO_ERROR_MESSAGE_BOX_HPP
