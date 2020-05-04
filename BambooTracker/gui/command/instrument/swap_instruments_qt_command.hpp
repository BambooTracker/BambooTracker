#ifndef SWAP_INSTRUMENTS_QT_COMMAND_HPP
#define SWAP_INSTRUMENTS_QT_COMMAND_HPP

#include <QUndoCommand>
#include <memory>
#include <QListWidget>
#include <QString>
#include "gui/instrument_editor/instrument_form_manager.hpp"

class PatternEditor;

class SwapInstrumentsQtCommand : public QUndoCommand
{
public:
	SwapInstrumentsQtCommand(QListWidget *list, int inst1Row, int inst2Row,
							 QString inst1Name, QString inst2Name,
							 std::weak_ptr<InstrumentFormManager> formMan,
							 PatternEditor* pattern,
							 QUndoCommand* parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget* list_;
	PatternEditor* ptn_;
	int inst1Row_, inst2Row_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	QString inst1Name_, inst2Name_;

	void swap(int above, int below, QString aboveName, QString belowName);
};

#endif // SWAP_INSTRUMENTS_QT_COMMAND_HPP
