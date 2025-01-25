/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

class DuplicateOrderCommand : public AbstractCommand
{
public:
	DuplicateOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, order_;
};
