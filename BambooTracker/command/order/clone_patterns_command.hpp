/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <vector>
#include "../abstract_command.hpp"
#include "module.hpp"

class ClonePatternsCommand final : public AbstractCommand
{
public:
	ClonePatternsCommand(std::weak_ptr<Module> mod, int songNum,
						 int beginOrder, int beginTrack, int endOrder, int endTrack);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, bOrder_, bTrack_, eOrder_, eTrack_;
	std::vector<std::vector<OrderInfo>> prevOdrs_;
};
