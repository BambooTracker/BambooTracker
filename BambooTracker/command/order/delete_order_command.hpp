#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include "module.hpp"
#include "track.hpp"

class DeleteOrderCommand : public AbstractCommand
{
public:
	DeleteOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum);
	void redo() override;
	void undo() override;
	int getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, order_;
	std::vector<OrderData> prevOdr_;
};
