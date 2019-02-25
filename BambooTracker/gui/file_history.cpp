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
