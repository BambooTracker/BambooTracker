#ifndef CLONE_INSTRUMENT_QT_COMMAND_H
#define CLONE_INSTRUMENT_QT_COMMAND_H

#include <QUndoCommand>
#include <QListWidget>
#include <memory>
#include <map>
#include "misc.hpp"

class CloneInstrumentQtCommand : public QUndoCommand
{
public:
	CloneInstrumentQtCommand(QListWidget *list, int num, int refNum,
							 std::map<int, std::unique_ptr<QWidget>>& map,
							 QUndoCommand* parent = nullptr);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE;

private:
	QListWidget* list_;
	int cloneNum_, refNum_;
	std::map<int, std::unique_ptr<QWidget>>& map_;
	SoundSource source_;
};

#endif // CLONE_INSTRUMENT_QT_COMMAND_H
