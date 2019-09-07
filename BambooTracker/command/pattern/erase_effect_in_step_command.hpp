#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <string>
#include "module.hpp"

class EraseEffectInStepCommand : public AbstractCommand
{
public:
	EraseEffectInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int n);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, n_;
	std::string prevEffID_;
	int prevEffVal_;
};
