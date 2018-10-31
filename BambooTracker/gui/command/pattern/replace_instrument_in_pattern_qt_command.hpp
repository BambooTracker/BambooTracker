#ifndef REPLACE_INSTRUMENT_IN_PATTERN_QT_COMMAND_HPP
#define REPLACE_INSTRUMENT_IN_PATTERN_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class ReplaceInstrumentInPatternQtCommand : public QUndoCommand
{
public:
	ReplaceInstrumentInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // REPLACE_INSTRUMENT_IN_PATTERN_QT_COMMAND_HPP
