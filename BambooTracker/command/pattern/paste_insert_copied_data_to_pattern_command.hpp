#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include <string>
#include "module.hpp"

class PasteInsertCopiedDataToPatternCommand : public AbstractCommand
{
public:
	PasteInsertCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
										  int beginOrder, int beginStep, std::vector<std::vector<std::string>> cells);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, col_, order_, step_;
	std::vector<std::vector<std::string>> cells_, prevCells_;
};
