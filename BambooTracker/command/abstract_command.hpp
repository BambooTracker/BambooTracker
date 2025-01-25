/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "command_id.hpp"

class AbstractCommand
{
public:
	AbstractCommand(CommandId id) : id_(id) {}
	virtual ~AbstractCommand() = default;
	virtual bool redo() = 0;
	virtual bool undo() = 0;
	inline CommandId getID() const noexcept { return id_; }
	virtual bool mergeWith(const AbstractCommand* other)
	{
		(void)other;
		return false;
	}

private:
	const CommandId id_;
};
