#ifndef CLONE_INSTRUMENT_QT_COMMAND_H
#define CLONE_INSTRUMENT_QT_COMMAND_H

#include <QUndoCommand>
#include <QListWidget>
#include <memory>
#include "gui/instrument_editor/instrument_form_manager.hpp"
#include "misc.hpp"

class CloneInstrumentQtCommand : public QUndoCommand
{
public:
	CloneInstrumentQtCommand(QListWidget *list, int num, SoundSource src, QString name,
							 std::weak_ptr<InstrumentFormManager> formMan,
							 QUndoCommand* parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget* list_;
	int cloneNum_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	SoundSource source_;
	QString name_;
};

#endif // CLONE_INSTRUMENT_QT_COMMAND_H
