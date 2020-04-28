#ifndef DEEP_CLONE_INSTRUMENT_QT_COMMAND_HPP
#define DEEP_CLONE_INSTRUMENT_QT_COMMAND_HPP

#include <QUndoCommand>
#include <QListWidget>
#include <memory>
#include "gui/mainwindow.hpp"
#include "gui/instrument_editor/instrument_form_manager.hpp"

enum class InstrumentType;

class DeepCloneInstrumentQtCommand : public QUndoCommand
{
public:
	DeepCloneInstrumentQtCommand(QListWidget *list, int num, InstrumentType type, QString name,
								 std::weak_ptr<InstrumentFormManager> formMan, MainWindow* mainwin, bool onlyUsed,
								 QUndoCommand* parent = nullptr);

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget* list_;
	int cloneNum_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	InstrumentType type_;
	QString name_;
	MainWindow* mainwin_;
	bool onlyUsed_;
};

#endif // DEEP_CLONE_INSTRUMENT_QT_COMMAND_HPP
