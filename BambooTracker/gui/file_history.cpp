/*
 * Copyright (C) 2019 Rerrah
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

#include "file_history.hpp"
#include <algorithm>

const int FileHistory::HISTORY_SIZE_ = 8;

FileHistory::FileHistory() {}

void FileHistory::addFile(QString path)
{
	auto it = std::find_if(list_.begin(), list_.end(), [&path](QString& p) { return (path == p); });
	if (it != list_.end()) list_.erase(it);
	list_.push_front(path);
	if (list_.size() > HISTORY_SIZE_) list_.pop_back();
}

void FileHistory::clearHistory()
{
	list_.clear();
}

QString FileHistory::at(size_t i)
{
	return list_.at(i);
}

size_t FileHistory::size() const
{
	return list_.size();
}

bool FileHistory::empty() const
{
	return list_.empty();
}
