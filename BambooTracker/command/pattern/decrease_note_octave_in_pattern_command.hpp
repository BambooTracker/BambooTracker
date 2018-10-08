#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include <vector>
#include "module.hpp"

class DecreaseNoteOctaveInPatternCommand : public AbstructCommand
{
public:
	DecreaseNoteOctaveInPatternCommand(std::weak_ptr<Module> mod, int songNum,
									int beginTrack, int beginOrder, int beginStep,
									int endTrack, int endStep);
	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_;
	int bTrack_, order_, bStep_;
	int eTrack_, eStep_;
	std::vector<int> prevKeys_;
};
