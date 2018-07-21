#ifndef REMOVE_INSTRUMENT_QT_COMMAND_HPP
#define REMOVE_INSTRUMENT_QT_COMMAND_HPP

#include <map>
#include <memory>
#include <QUndoCommand>
#include <QWidget>
#include <QListWidget>
#include <QString>
#include "misc.hpp"

class RemoveInstrumentQtCommand : public QUndoCommand
{
public:
	RemoveInstrumentQtCommand(QListWidget *list, int num, int row,
							  std::map<int, std::unique_ptr<QWidget>> &map, QUndoCommand *parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	QString name_;
	int row_;
	SoundSource source_;
	std::map<int, std::unique_ptr<QWidget>> &map_;
};

#endif // REMOVE_INSTRUMENT_QT_COMMAND_HPP
