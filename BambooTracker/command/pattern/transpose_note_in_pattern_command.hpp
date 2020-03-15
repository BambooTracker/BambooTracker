#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include "module.hpp"

class TransposeNoteInPatternCommand : public AbstractCommand
{
public:
	TransposeNoteInPatternCommand(std::weak_ptr<Module> mod, int songNum,
									int beginTrack, int beginOrder, int beginStep,
									int endTrack, int endStep, int seminote);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_;
	int bTrack_, order_, bStep_;
	int eTrack_, eStep_;
	int seminote_;
	std::vector<int> prevKeys_;
};
