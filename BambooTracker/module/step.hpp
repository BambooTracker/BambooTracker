/*
 * Copyright (C) 2018-2019 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
