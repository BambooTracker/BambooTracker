#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"

class SetKeyOnToStepCommand : public AbstractCommand
{
public:
	SetKeyOnToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum,
						  int noteNum, bool instMask, int instNum, bool volMask, int vol, bool isFMReversed);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, note_, inst_, vol_;
	int prevNote_, prevInst_, prevVol_;
	bool instMask_, volMask_;
	bool isFMReserved_;
};
