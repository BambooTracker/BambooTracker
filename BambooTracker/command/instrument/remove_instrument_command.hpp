#pragma once

#include <memory>
#include "abstract_command.hpp"
#include "instruments_manager.hpp"
#include "instrument.hpp"

class RemoveInstrumentCommand : public AbstractCommand
{
public:
	RemoveInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int number);

	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int number_;
	std::unique_ptr<AbstractInstrument> inst_;
};
