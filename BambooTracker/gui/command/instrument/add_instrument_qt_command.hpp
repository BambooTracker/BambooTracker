#ifndef ADD_INSTRUMENT_QT_COMMAND_HPP
#define ADD_INSTRUMENT_QT_COMMAND_HPP

#include <map>
#include <memory>
#include <QUndoCommand>
#include <QWidget>
#include <QListWidget>
#include <QString>
#include "misc.hpp"

class AddInstrumentQtCommand : public QUndoCommand
{
public:
	AddInstrumentQtCommand(QListWidget *list, int num, QString name, SoundSource source,
						   std::map<int, std::unique_ptr<QWidget>> &map, QUndoCommand *parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	QString name_;
	SoundSource source_;
	std::map<int, std::unique_ptr<QWidget>> &map_;
};

#endif // ADD_INSTRUMENT_QT_COMMAND_HPP
