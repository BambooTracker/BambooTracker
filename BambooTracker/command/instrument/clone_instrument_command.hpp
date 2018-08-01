#pragma once

#include "abstruct_command.hpp"
#include "instruments_manager.hpp"

class CloneInstrumentCommand : public AbstructCommand
{
public:
	CloneInstrumentCommand(InstrumentsManager &manager, int num, int refNum);

	void redo() override;
	void undo() override;
	int getID() const override;

private:
	InstrumentsManager& manager_;
	int cloneInstNum_, refInstNum_;
};
