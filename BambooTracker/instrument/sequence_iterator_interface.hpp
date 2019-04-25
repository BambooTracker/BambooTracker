#pragma once

class SequenceIteratorInterface
{
public:
	virtual ~SequenceIteratorInterface() = default;
	/// -1: sequence end
	/// else: position in the sequence
	virtual int getPosition() const = 0;
	/// 0: absolute
	/// 1: fix
	/// 1: relative
	virtual int getSequenceType() const = 0;
	virtual int getCommandType() const = 0;
	virtual int getCommandData() const = 0;
	virtual int next(bool isReleaseBegin = false) = 0;
	virtual int front() = 0;
	virtual int end() = 0;
};
