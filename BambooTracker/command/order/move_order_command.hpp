#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"

class MoveOrderCommand : public AbstractCommand
{
public:
	MoveOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum, bool isUp);
	void redo() override;
	void undo() override;
	CommandId getID() const override;

private:
	std::weak_ptr<Module> mod_;
	int song_, order_;
	bool isUp_;

	void swap();
};
