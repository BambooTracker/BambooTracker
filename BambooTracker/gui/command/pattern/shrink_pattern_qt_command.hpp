#ifndef SHRINK_PATTERN_QT_COMMAND_HPP
#define SHRINK_PATTERN_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class ShrinkPatternQtCommand : public QUndoCommand
{
public:
	ShrinkPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // SHRINK_PATTERN_QT_COMMAND_HPP
