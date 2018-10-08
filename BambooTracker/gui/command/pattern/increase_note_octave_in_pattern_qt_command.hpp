#ifndef INCREASE_NOTE_OCTAVE_IN_PATTERN_QT_COMMAND_HPP
#define INCREASE_NOTE_OCTAVE_IN_PATTERN_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class IncreaseNoteOctaveInPatternQtCommand : public QUndoCommand
{
public:
	IncreaseNoteOctaveInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // INCREASE_NOTE_OCTAVE_IN_PATTERN_QT_COMMAND_HPP
