#ifndef PASTE_MIX_COPIED_DATA_TO_PATTERN_QT_COMMAND_HPP
#define PASTE_MIX_COPIED_DATA_TO_PATTERN_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"

class PasteMixCopiedDataToPatternQtCommand : public QUndoCommand
{
public:
	PasteMixCopiedDataToPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	PatternEditorPanel* panel_;
};

#endif // PASTE_MIX_COPIED_DATA_TO_PATTERN_QT_COMMAND_HPP
