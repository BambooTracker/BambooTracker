#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include "module.hpp"
#include "track.hpp"

class ClonePatternsCommand : public AbstractCommand
{
public:
	ClonePatternsCommand(std::weak_ptr<Module> mod, int songNum,
						 int beginOrder, int beginTrack, int endOrder, int endTrack);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, bOrder_, bTrack_, eOrder_, eTrack_;
	std::vector<std::vector<OrderData>> prevOdrs_;
};
