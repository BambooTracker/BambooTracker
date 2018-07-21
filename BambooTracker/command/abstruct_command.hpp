#pragma once

struct AbstructCommand
{
	virtual ~AbstructCommand() {}
	virtual void redo() = 0;
	virtual void undo() = 0;
	virtual int getID() const = 0;
	virtual bool mergeWith(const AbstructCommand* other)
	{
		return false;
	}
};
