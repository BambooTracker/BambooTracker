#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include "module.hpp"

class ChangeValuesInPatternCommand : public AbstractCommand
{
public:
	ChangeValuesInPatternCommand(std::weak_ptr<Module> mod, int songNum,
								 int beginTrack, int beginColumn, int beginOrder, int beginStep,
								 int endTrack, int endColumn, int endStep, int value);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_;
	int bTrack_, bCol_, order_, bStep_;
	int eTrack_, eCol_, eStep_;
	int diff_;
	std::vector<std::vector<int>> prevVals_;
};
