/*
 * Copyright (C) 2018-2019 Rerrah
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

#include "set_effect_value_to_step_qt_command.hpp"
#include "command_id.hpp"

SetEffectValueToStepQtCommand::SetEffectValueToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, bool secondEntry, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isSecond_(secondEntry)
{
}

void SetEffectValueToStepQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void SetEffectValueToStepQtCommand::undo()
{
	panel_->redrawByPatternChanged();
	panel_->resetEntryCount();
}

int SetEffectValueToStepQtCommand::id() const
{
	return CommandId::SetEffectValueToStep;
}

bool SetEffectValueToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isSecond_) {
		auto com = dynamic_cast<const SetEffectValueToStepQtCommand*>(other);
		if (com->getPos() == pos_ && com->isSecondEntry()) {
			isSecond_ = true;
			redo();
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

PatternPosition SetEffectValueToStepQtCommand::getPos() const
{
	return pos_;
}

bool SetEffectValueToStepQtCommand::isSecondEntry() const
{
	return isSecond_;
}
