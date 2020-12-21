/*
 * Copyright (C) 2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PATTERN_EDITOR_COMMON_QT_COMMAND_HPP
#define PATTERN_EDITOR_COMMON_QT_COMMAND_HPP

#include <QUndoCommand>
#include "command/command_id.hpp"
#include "gui/pattern_editor/pattern_position.hpp"

class PatternEditorPanel;

namespace gui_command_impl
{
class PatternEditorCommonQtCommand : public QUndoCommand
{
public:
	virtual void redo() override;
	virtual void undo() override;
	int id() const override final;

protected:
	PatternEditorPanel* panel_;
	PatternEditorCommonQtCommand(CommandId id, PatternEditorPanel* panel,
								 bool redrawAll, QUndoCommand* parent);

private:
	CommandId id_;
	bool redrawAll_;
};

template<CommandId comId, bool redrawAll>
class PatternEditorCommonQtCommandRedraw final : public PatternEditorCommonQtCommand
{
public:
	PatternEditorCommonQtCommandRedraw(PatternEditorPanel* panel, QUndoCommand* parent = nullptr)
		: PatternEditorCommonQtCommand(comId, panel, redrawAll, parent) {}
};

template<CommandId id>
using PatternEditorCommonQtCommandRedrawAll = PatternEditorCommonQtCommandRedraw<id, true>;

template<CommandId id>
using PatternEditorCommonQtCommandRedrawText = PatternEditorCommonQtCommandRedraw<id, false>;

class PatternEditorEntryQtCommand : public PatternEditorCommonQtCommand
{
public:
	void undo() override;
	bool mergeWith(const QUndoCommand* other) override;

protected:
	PatternEditorEntryQtCommand(CommandId id, PatternEditorPanel* panel, bool redrawAll, PatternPosition pos, bool secondEntry, QUndoCommand* parent = nullptr);

private:
	const PatternPosition pos_;
	bool isSecondEntry_;
};

template<CommandId comId, bool redrawAll>
class PatternEditorEntryQtCommandRedraw final : public PatternEditorEntryQtCommand
{
public:
	PatternEditorEntryQtCommandRedraw(PatternEditorPanel* panel, PatternPosition pos, bool secondEntry, QUndoCommand* parent = nullptr)
		: PatternEditorEntryQtCommand(comId, panel, redrawAll, pos, secondEntry, parent) {}
};

template<CommandId id>
using PatternEditorEntryQtCommandRedrawAll = PatternEditorEntryQtCommandRedraw<id, true>;

template<CommandId id>
using PatternEditorEntryQtCommandRedrawText = PatternEditorEntryQtCommandRedraw<id, false>;
}

#endif // PATTERN_EDITOR_COMMON_QT_COMMAND_HPP
