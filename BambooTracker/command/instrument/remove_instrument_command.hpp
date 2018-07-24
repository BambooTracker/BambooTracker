#pragma once

#include <memory>
#include "abstruct_command.hpp"
#include "instruments_manager.hpp"
#include "instrument.hpp"

class RemoveInstrumentCommand : public AbstructCommand
{
public:
	RemoveInstrumentCommand(InstrumentsManager &manager, int number);

	void redo() override;
	void undo() override;
	int getID() const override;

private:
	InstrumentsManager &manager_;
	int number_;
	std::unique_ptr<AbstructInstrument> inst_;
};
