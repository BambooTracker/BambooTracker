#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include <string>
#include "module.hpp"

class PasteCopiedDataToPatternCommand : public AbstractCommand
{
public:
	PasteCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
									std::vector<std::vector<std::string>> cells);
	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, col_, order_, step_;
	std::vector<std::vector<std::string>> cells_, prevCells_;

	void setCells(std::vector<std::vector<std::string>>& cells);
};
