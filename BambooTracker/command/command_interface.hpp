#pragma once

struct CommandInterface
{
	virtual void invoke() = 0;
	virtual void undo() = 0;
	virtual void redo() = 0;
	virtual ~CommandInterface() {}
};
