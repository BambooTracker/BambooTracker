#pragma once

#include <memory>
#include "abstract_command.hpp"
#include "instruments_manager.hpp"

class cloneInstrumentCommand : public AbstractCommand
{
public:
	cloneInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int num, int refNum);

	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int cloneInstNum_, refInstNum_;
};
