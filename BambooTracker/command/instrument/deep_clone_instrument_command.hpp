#pragma once

#include "abstruct_command.hpp"
#include "instruments_manager.hpp"

class DeepCloneInstrumentCommand : public AbstructCommand
{
public:
	DeepCloneInstrumentCommand(InstrumentsManager &manager, int num, int refNum);

	void redo() override;
	void undo() override;
	int getID() const override;

private:
	InstrumentsManager& manager_;
	int cloneInstNum_, refInstNum_;
};
