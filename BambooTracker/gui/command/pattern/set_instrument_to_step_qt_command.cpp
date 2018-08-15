#include "set_instrument_to_step_qt_command.hpp"

SetInstrumentToStepQtCommand::SetInstrumentToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isComplete_(false)
{
}

void SetInstrumentToStepQtCommand::redo()
{
	panel_->update();
}

void SetInstrumentToStepQtCommand::undo()
{
	panel_->update();
}

int SetInstrumentToStepQtCommand::id() const
{
	return 0x24;
}

bool SetInstrumentToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isComplete_) {
		auto com = dynamic_cast<const SetInstrumentToStepQtCommand*>(other);
		if (com->getPos() == pos_) {
			isComplete_ = true;
			redo();
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

PatternPosition SetInstrumentToStepQtCommand::getPos() const
{
	return pos_;
}
