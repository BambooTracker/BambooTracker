#ifndef CHANGE_INSTRUMENT_NAME_QT_COMMAND_HPP
#define CHANGE_INSTRUMENT_NAME_QT_COMMAND_HPP

#include <memory>
#include <map>
#include <QUndoCommand>
#include <QString>
#include <QListWidget>
#include "misc.hpp"

class ChangeInstrumentNameQtCommand : public QUndoCommand
{
public:
	ChangeInstrumentNameQtCommand(QListWidget *list, int num, int row,
								  std::map<int, std::unique_ptr<QWidget>>& map,
								  QString oldName, QString newName, QUndoCommand* parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	int row_;
	std::map<int, std::unique_ptr<QWidget>> &map_;
	QString oldName_, newName_;
	SoundSource source_;
};

#endif // CHANGE_INSTRUMENT_NAME_QT_COMMAND_HPP
