/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include "../abstract_command.hpp"
#include "module.hpp"
#include "vector_2d.hpp"

class PasteCopiedDataToOrderCommand final : public AbstractCommand
{
public:
	PasteCopiedDataToOrderCommand(
			std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginOrder,
			const Vector2d<int>& cells);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_;
	Vector2d<int> cells_, prevCells_;

	void setCells(const Vector2d<int>& cells);
};
