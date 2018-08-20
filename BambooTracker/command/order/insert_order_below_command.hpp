#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include "module.hpp"

class InsertOrderBelowCommand : public AbstructCommand
{
public:
	InsertOrderBelowCommand(std::weak_ptr<Module> mod, int songNum, int orderNum);
	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, order_;
};
