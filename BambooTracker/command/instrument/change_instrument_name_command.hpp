#pragma once

#include <memory>
#include <string>
#include "abstract_command.hpp"
#include "instruments_manager.hpp"

class ChangeInstrumentNameCommand : public AbstractCommand
{
public:
	ChangeInstrumentNameCommand(std::weak_ptr<InstrumentsManager> manager, int num, std::string name);

	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int instNum_;
	std::string oldName_, newName_;
};
