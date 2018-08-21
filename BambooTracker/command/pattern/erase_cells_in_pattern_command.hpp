#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include <vector>
#include <string>
#include "module.hpp"

class EraseCellsInPatternCommand : public AbstructCommand
{
public:
	EraseCellsInPatternCommand(std::weak_ptr<Module> mod, int songNum,
							   int beginTrack, int beginColmn, int beginOrder, int beginStep,
							   int endTrack, int endColumn, int endStep);
	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, bTrack_, bCol_, order_, bStep_;
	int eTrack_, eCol_, eStep_;
	std::vector<std::vector<std::string>> prevCells_;
};
