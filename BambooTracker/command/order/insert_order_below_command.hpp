#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"

class InsertOrderBelowCommand : public AbstractCommand
{
public:
	InsertOrderBelowCommand(std::weak_ptr<Module> mod, int songNum, int orderNum);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, order_;
};
