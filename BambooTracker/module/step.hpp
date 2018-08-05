#pragma once

#include <string>

class Step
{
public:
	Step();

private:
	/// noteNum_
	///		0<=: note number
	///		 -1: none
	///		 -2: key on
	///		 -3: key off
	///		 -4: key release
	int noteNum_;
	/// instNum_
	///		0<=: instrument number
	///		 -1: none
	int instNum_;
	/// vol_
	///		0<=: volume level
	///		 -1: none
	int vol_;
	std::string effStr_;
};
