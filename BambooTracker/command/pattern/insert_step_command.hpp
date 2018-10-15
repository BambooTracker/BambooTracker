#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"

class InsertStepCommand : public AbstractCommand
{
public:
	InsertStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum);
	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_;
};
