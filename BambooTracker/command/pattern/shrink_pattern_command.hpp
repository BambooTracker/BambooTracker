#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include <string>
#include "module.hpp"

class ShrinkPatternCommand : public AbstractCommand
{
public:
	ShrinkPatternCommand(std::weak_ptr<Module> mod, int songNum,
						 int beginTrack, int beginColmn, int beginOrder, int beginStep,
						 int endTrack, int endColumn, int endStep);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, bTrack_, bCol_, order_, bStep_;
	int eStep_;
	std::vector<std::vector<std::string>> prevCells_;
};
