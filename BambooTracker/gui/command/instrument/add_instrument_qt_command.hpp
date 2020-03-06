#ifndef ADD_INSTRUMENT_QT_COMMAND_HPP
#define ADD_INSTRUMENT_QT_COMMAND_HPP

#include <memory>
#include <QUndoCommand>
#include <QWidget>
#include <QListWidget>
#include <QString>
#include "gui/mainwindow.hpp"
#include "gui/instrument_editor/instrument_form_manager.hpp"
#include "misc.hpp"

class AddInstrumentQtCommand : public QUndoCommand
{
public:
	AddInstrumentQtCommand(QListWidget *list, int num, QString name, SoundSource source,
						   std::weak_ptr<InstrumentFormManager> formMan,
						   MainWindow* mainwin, bool onlyUsed, bool preventFirstStore = false,
						   QUndoCommand *parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	QString name_;
	SoundSource source_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	MainWindow* mainwin_;
	bool onlyUsed_, hasDone_;
};

#endif // ADD_INSTRUMENT_QT_COMMAND_HPP
