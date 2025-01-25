/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "instruments_manager.hpp"
#include "module.hpp"

class SwapInstrumentsCommand final : public AbstractCommand
{
public:
	SwapInstrumentsCommand(std::weak_ptr<InstrumentsManager> manager, std::weak_ptr<Module> mod,
						   int inst1, int inst2, int song, bool patternChange);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	std::weak_ptr<Module> mod_;
	int inst1Num_, inst2Num_;
	int songNum_;
	bool patternChange_;

	void swapInstrumentsInPatterns();
};
