/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

class SetPatternToOrderCommand final : public AbstractCommand
{
public:
	SetPatternToOrderCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
							 int orderNum, int patternNum, bool secondEntry);
	bool redo() override;
	bool undo() override;
	bool mergeWith(const AbstractCommand* other) override;

private:
	std::weak_ptr<Module> mod_;
	const int song_, track_, order_;
	int pattern_;
	const int prevPattern_;
	bool isSecondEntry_;
};
