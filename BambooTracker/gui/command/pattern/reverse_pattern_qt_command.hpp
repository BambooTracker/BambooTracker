#ifndef REVERSE_PATTERN_QT_COMMAND_HPP
#define REVERSE_PATTERN_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class ReversePatternQtCommand : public QUndoCommand
{
public:
	ReversePatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // REVERSE_PATTERN_QT_COMMAND_HPP
