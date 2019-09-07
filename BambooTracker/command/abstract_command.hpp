#pragma once

#include "command_id.hpp"

struct AbstractCommand
{
	virtual ~AbstractCommand() {}
	virtual void redo() = 0;
	virtual void undo() = 0;
	virtual CommandId getID() const = 0;
	virtual bool mergeWith(const AbstractCommand* other)
	{
		(void)other;
		return false;
	}
};
