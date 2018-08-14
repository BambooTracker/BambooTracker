#pragma once

#include <string>
#include "abstruct_command.hpp"
#include "instrument.hpp"
#include "instruments_manager.hpp"
#include "misc.hpp"

class AddInstrumentCommand : public AbstructCommand
{
public:
	AddInstrumentCommand(InstrumentsManager &manager, int num, SoundSource source, std::string name);

	void redo() override;
	void undo() override;
	int getID() const override;

private:
	InstrumentsManager &manager_;
	int num_;
	SoundSource source_;
	std::string name_;
};
