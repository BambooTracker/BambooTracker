/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

enum class EffectDisplayControl
{
	Unset, ReverseFMVolumeDelay, ReverseFMBrightness
};

class SetEffectValueToStepCommand final : public AbstractCommand
{
public:
	SetEffectValueToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
								int orderNum, int stepNum, int n, int value,
								EffectDisplayControl ctrl, bool secondEntry);
	bool redo() override;
	bool undo() override;
	bool mergeWith(const AbstractCommand* other) override;

private:
	std::weak_ptr<Module> mod_;
	const int song_, track_, order_, step_, n_;
	int val_, prevVal_;
	const EffectDisplayControl ctrl_;
	bool isSecondEntry_;
};
