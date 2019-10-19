#include "set_instrument_to_step_qt_command.hpp"
#include "command_id.hpp"

SetInstrumentToStepQtCommand::SetInstrumentToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, bool secondEntry, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isSecond_(secondEntry)
{
}

void SetInstrumentToStepQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void SetInstrumentToStepQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int SetInstrumentToStepQtCommand::id() const
{
	return CommandId::SetInstrumentInStep;
}

bool SetInstrumentToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isSecond_) {
		auto com = dynamic_cast<const SetInstrumentToStepQtCommand*>(other);
		if (com->getPos() == pos_ && com->isSecondEntry()) {
			isSecond_ = true;
			redo();
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

PatternPosition SetInstrumentToStepQtCommand::getPos() const
{
	return pos_;
}

bool SetInstrumentToStepQtCommand::isSecondEntry() const
{
	return isSecond_;
}
