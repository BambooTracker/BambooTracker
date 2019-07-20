#pragma once

#include <string>

class Step
{
public:
	Step();

	int getNoteNumber() const;
	void setNoteNumber(int num);

	int getInstrumentNumber() const;
	void setInstrumentNumber(int num);

	int getVolume() const;
	void setVolume(int volume);

	std::string getEffectID(int n) const;
	void setEffectID(int n, std::string str);

	int getEffectValue(int n) const;
	void setEffectValue(int n, int v);

	/// NOTE: Deprecated
	int checkEffectID(std::string str) const;

	bool existCommand() const;

private:
	/// noteNum_
	///		0<=: note number (key on)
	///		 -1: none
	///		 -2: key off
	///		 -3: echo previous note
	///		 -4: echo 2 notes before
	///		 -5: echo 3 notes before
	///		 -6: echo 4 notes before
	int noteNum_;
	/// instNum_
	///		0<=: instrument number
	///		 -1: none
	int instNum_;
	/// vol_
	///		0<=: volume level
	///		 -1: none
	int vol_;
	std::string effID_[4];
	/// effVal_
	///		0<=: effect value
	///		 -1: none
	int effVal_[4];
};
