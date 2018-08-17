#ifndef ERASE_EFFECT_VALUE_IN_STEP_QT_COMMAND_HPP
#define ERASE_EFFECT_VALUE_IN_STEP_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class EraseEffectValueInStepQtCommand : public QUndoCommand
{
public:
	EraseEffectValueInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // ERASE_EFFECT_VALUE_IN_STEP_QT_COMMAND_HPP
