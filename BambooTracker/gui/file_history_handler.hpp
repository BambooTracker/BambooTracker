#ifndef FILE_HISTORY_HANDLER_HPP
#define FILE_HISTORY_HANDLER_HPP

#include <memory>
#include <QString>
#include "file_history.hpp"

class FileHistoryHandler
{
public:
	static bool saveFileHistory(std::weak_ptr<FileHistory> history);
	static bool loadFileHistory(std::weak_ptr<FileHistory> history);

private:
	const static QString ORGANIZATION_;
	const static QString FILE_;

	FileHistoryHandler();
};

#endif // FILE_HISTORY_HANDLER_HPP
