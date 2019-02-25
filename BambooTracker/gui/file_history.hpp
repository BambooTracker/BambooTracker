#ifndef FILE_HISTORY_HPP
#define FILE_HISTORY_HPP

#include <deque>
#include <QString>

class FileHistory
{
public:
	FileHistory();

	void addFile(QString path);
	void clearHistory();
	QString at(size_t i);
	size_t size() const;
	bool empty() const;

private:
	static const int HISTORY_SIZE_;
	std::deque<QString> list_;
};

#endif // FILE_HISTORY_HPP
