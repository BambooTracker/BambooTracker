#ifndef DELETE_PREVIOUS_STEP_QT_COMMAND_HPP
#define DELETE_PREVIOUS_STEP_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class DeletePreviousStepQtCommand : public QUndoCommand
{
public:
	DeletePreviousStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // DELETE_PREVIOUS_STEP_QT_COMMAND_HPP
