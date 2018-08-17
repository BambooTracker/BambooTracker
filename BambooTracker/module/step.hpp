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

	std::string getEffectID() const;
	void setEffectID(std::string str);

	int getEffectValue() const;
	void setEffectValue(int v);

private:
	/// noteNum_
	///		0<=: note number (key on)
	///		 -1: none
	///		 -2: key off
	int noteNum_;
	/// instNum_
	///		0<=: instrument number
	///		 -1: none
	int instNum_;
	/// vol_
	///		0<=: volume level
	///		 -1: none
	int vol_;
	std::string effID_;
	/// effVal_
	///		0<=: effect value
	///		 -1: none
	int effVal_;
};
