#pragma once

#include <string>
#include "command_interface.hpp"
#include "abstruct_instrument.hpp"
#include "instruments_manager.hpp"
#include "misc.hpp"

class AddInstrumentCommand : public CommandInterface
{
public:
	AddInstrumentCommand(InstrumentsManager &manager, int num, SoundSource source, std::string name);

	void invoke() override;
	void undo() override;
	void redo() override;

private:
	InstrumentsManager &manager_;
	int num_;
	SoundSource source_;
	std::string name_;

	void add();
};
