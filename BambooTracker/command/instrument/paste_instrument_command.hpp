#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include "instruments_manager.hpp"

class PasteInstrumentCommand : public AbstructCommand
{
public:
	PasteInstrumentCommand(InstrumentsManager &manager, int num, int refNum);

	void redo() override;
	void undo() override;
	int getID() const override;

private:
	InstrumentsManager& manager_;
	std::unique_ptr<AbstructInstrument> oldInst_, refInst_;
};
