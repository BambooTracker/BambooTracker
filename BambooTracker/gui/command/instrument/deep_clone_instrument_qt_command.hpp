#ifndef DEEP_CLONE_INSTRUMENT_QT_COMMAND_HPP
#define DEEP_CLONE_INSTRUMENT_QT_COMMAND_HPP

#include <QUndoCommand>
#include <QListWidget>
#include <map>
#include <memory>
#include "misc.hpp"

class DeepCloneInstrumentQtCommand : public QUndoCommand
{
public:
	DeepCloneInstrumentQtCommand(QListWidget *list, int num, int refNum,
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

#endif // DEEP_CLONE_INSTRUMENT_QT_COMMAND_HPP
