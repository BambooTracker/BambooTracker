#ifndef INCREASE_NOTE_KEY_IN_PATTERN_QT_COMMAND_HPP
#define INCREASE_NOTE_KEY_IN_PATTERN_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class IncreaseNoteKeyInPatternQtCommand : public QUndoCommand
{
public:
	IncreaseNoteKeyInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // INCREASE_NOTE_KEY_IN_PATTERN_QT_COMMAND_HPP
