#pragma once

#include <string>
#include "abstract_command.hpp"
#include "instruments_manager.hpp"

class ChangeInstrumentNameCommand : public AbstractCommand
{
public:
	ChangeInstrumentNameCommand(InstrumentsManager &manager, int num, std::string name);

	void redo() override;
	void undo() override;
	int getID() const override;

private:
	InstrumentsManager& manager_;
	int instNum_;
	std::string oldName_, newName_;
};
