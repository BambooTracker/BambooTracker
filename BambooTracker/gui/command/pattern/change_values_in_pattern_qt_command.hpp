#ifndef ADDVALUEINPATTERNQTCOMMAND_HPP
#define ADDVALUEINPATTERNQTCOMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class ChangeValuesInPatternQtCommand : public QUndoCommand
{
public:
	ChangeValuesInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // ADDVALUEINPATTERNQTCOMMAND_HPP
