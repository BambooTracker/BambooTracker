#ifndef ADD_INSTRUMENT_QT_COMMAND_HPP
#define ADD_INSTRUMENT_QT_COMMAND_HPP

#include <QUndoCommand>
#include <QListWidget>
#include <QString>

class AddInstrumentQtCommand : public QUndoCommand
{
public:
	AddInstrumentQtCommand(QListWidget *list, int num, QString name, QUndoCommand *parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	QString name_;
};

#endif // ADD_INSTRUMENT_QT_COMMAND_HPP
