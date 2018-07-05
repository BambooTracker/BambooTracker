#ifndef REMOVE_INSTRUMENT_QT_COMMAND_HPP
#define REMOVE_INSTRUMENT_QT_COMMAND_HPP

#include <QUndoCommand>
#include <QListWidget>
#include <QString>

class RemoveInstrumentQtCommand : public QUndoCommand
{
public:
	RemoveInstrumentQtCommand(QListWidget *list, int num, int row, QUndoCommand *parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	QString name_;
	int row_;
};

#endif // REMOVE_INSTRUMENT_QT_COMMAND_HPP
