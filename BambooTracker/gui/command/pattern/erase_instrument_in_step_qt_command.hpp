#ifndef ERASE_INSTRUMENT_IN_STEP_QT_COMMAND_HPP
#define ERASE_INSTRUMENT_IN_STEP_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class EraseInstrumentInStepQtCommand : public QUndoCommand
{
public:
	EraseInstrumentInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // ERASE_INSTRUMENT_IN_STEP_QT_COMMAND_HPP
