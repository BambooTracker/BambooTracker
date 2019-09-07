#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <string>
#include "module.hpp"

class DeletePreviousStepCommand : public AbstractCommand
{
public:
	DeletePreviousStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_;
	int prevNote_, prevInst_, prevVol_, prevEffVal_[4];
	std::string prevEffID_[4];
};
