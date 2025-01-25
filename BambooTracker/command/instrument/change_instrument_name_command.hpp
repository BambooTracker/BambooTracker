/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include "../abstract_command.hpp"
#include "instruments_manager.hpp"

class ChangeInstrumentNameCommand final : public AbstractCommand
{
public:
	ChangeInstrumentNameCommand(std::weak_ptr<InstrumentsManager> manager,
								int num, const std::string& name);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int instNum_;
	std::string oldName_, newName_;
};
