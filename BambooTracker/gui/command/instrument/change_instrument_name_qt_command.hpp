#ifndef CHANGE_INSTRUMENT_NAME_QT_COMMAND_HPP
#define CHANGE_INSTRUMENT_NAME_QT_COMMAND_HPP

#include <memory>
#include <QUndoCommand>
#include <QString>
#include <QListWidget>
#include "gui/instrument_editor/instrument_form_manager.hpp"

class ChangeInstrumentNameQtCommand : public QUndoCommand
{
public:
	ChangeInstrumentNameQtCommand(QListWidget *list, int num, int row,
								  std::weak_ptr<InstrumentFormManager> formMan,
								  QString oldName, QString newName, QUndoCommand* parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	int row_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	QString oldName_, newName_;
};

#endif // CHANGE_INSTRUMENT_NAME_QT_COMMAND_HPP
