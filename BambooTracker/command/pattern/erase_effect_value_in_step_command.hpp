#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include "module.hpp"

class EraseEffectValueInStepCommand : public AbstructCommand
{
public:
	EraseEffectValueInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int n);
	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, n_;
	int prevVal_;
};
