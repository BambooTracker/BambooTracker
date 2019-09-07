#pragma once

#include <memory>
#include <string>
#include "abstract_command.hpp"
#include "instrument.hpp"
#include "instruments_manager.hpp"
#include "misc.hpp"

class AddInstrumentCommand : public AbstractCommand
{
public:
	AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int num, SoundSource source, std::string name);
	AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, std::unique_ptr<AbstractInstrument> inst);

	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int num_;
	SoundSource source_;
	std::string name_;
	std::unique_ptr<AbstractInstrument> inst_;
};
