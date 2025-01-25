/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include "../abstract_command.hpp"
#include "module.hpp"

class SetEffectIDToStepCommand final : public AbstractCommand
{
public:
	SetEffectIDToStepCommand(
			std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum,
			int n, std::string id, bool fillValue00, bool secondEntry);
	bool redo() override;
	bool undo() override;
	bool mergeWith(const AbstractCommand* other) override;

private:
	std::weak_ptr<Module> mod_;
	const int song_, track_, order_, step_, n_;
	std::string effID_, prevEffID_;
	bool filledValue00_;
	bool isSecondEntry_;
};
