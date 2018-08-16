#ifndef ERASE_VOLUME_IN_STEP_QT_COMMAND_HPP
#define ERASE_VOLUME_IN_STEP_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class EraseVolumeInStepQtCommand : public QUndoCommand
{
public:
	EraseVolumeInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // ERASE_VOLUME_IN_STEP_QT_COMMAND_HPP
