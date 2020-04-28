#ifndef REMOVE_INSTRUMENT_QT_COMMAND_HPP
#define REMOVE_INSTRUMENT_QT_COMMAND_HPP

#include <memory>
#include <QUndoCommand>
#include <QWidget>
#include <QListWidget>
#include <QString>
#include "gui/mainwindow.hpp"
#include "gui/instrument_editor/instrument_form_manager.hpp"

enum class InstrumentType;

class RemoveInstrumentQtCommand : public QUndoCommand
{
public:
	RemoveInstrumentQtCommand(QListWidget *list, int num, int row, QString name, InstrumentType type,
							  std::weak_ptr<InstrumentFormManager> formMan,
							  MainWindow* mainwin, bool updateRequested, QUndoCommand *parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	int num_;
	QString name_;
	int row_;
	InstrumentType type_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	MainWindow* mainwin_;
	bool updateRequested_;
};

#endif // REMOVE_INSTRUMENT_QT_COMMAND_HPP
