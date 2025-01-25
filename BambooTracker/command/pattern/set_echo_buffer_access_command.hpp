/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

class SetEchoBufferAccessCommand final : public AbstractCommand
{
public:
	SetEchoBufferAccessCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
							   int orderNum, int stepNum, int bufNum);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, buf_;
	int prevNote_;
};
