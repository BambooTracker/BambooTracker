/*
 * Copyright (C) 2018-2022 Rerrah
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
#include <stdexcept>

class Step
{
public:
	Step();

	enum NoteValue : int
	{
		NOTE_NONE		= -1,
		// Special notes
		NOTE_KEY_OFF	= -2,
		NOTE_ECHO0		= -3,
		NOTE_ECHO1		= -4,
		NOTE_ECHO2		= -5,
		NOTE_ECHO3		= -6,
		NOTE_KEY_CUT	= -7,
	};
	int getNoteNumber() const noexcept { return note_; }
	void setNoteNumber(int num) { note_ = num; }
	void setKeyOff() { note_ = NOTE_KEY_OFF; }
	void setKeyCut() { note_ = NOTE_KEY_CUT; }
	void setEchoBuffer(int n) { note_ = NOTE_ECHO0 - n; }
	void clearNoteNumber() noexcept { note_ = NOTE_NONE; }
	bool hasGeneralNote() const noexcept { return note_ > NOTE_NONE; }
	bool hasKeyOff() const noexcept { return note_ == NOTE_KEY_OFF; }
	bool hasKeyCut() const noexcept { return note_  == NOTE_KEY_CUT; }
	bool hasNoteEchoBuffer(int n) const { return note_ == (NOTE_ECHO0 - n); }
	bool isEmptyNote() const noexcept { return note_ == NOTE_NONE; }

	static bool testEmptyNote(int note) { return note == NOTE_NONE; }

	static constexpr int INST_NONE = -1;
	int getInstrumentNumber() const noexcept { return inst_; }
	void setInstrumentNumber(int num) { inst_ = num; }
	void clearInstrumentNumber() noexcept { inst_ = INST_NONE; }
	bool hasInstrument() const noexcept { return inst_ != INST_NONE; }

	static bool testEmptyInstrument(int inst) { return inst == INST_NONE; }

	static constexpr int VOLUME_NONE = -1;
	int getVolume() const noexcept { return vol_; }
	void setVolume(int volume) { vol_ = volume; }
	void clearVolume() noexcept { vol_ = VOLUME_NONE; }
	bool hasVolume() const noexcept { return vol_ != VOLUME_NONE; }

	static bool testEmptyVolume(int vol) { return vol == VOLUME_NONE; }

	static const std::string EFF_ID_NONE;	// "--"
	std::string getEffectId(int n) const { return eff_[n].id; }
	void setEffectId(int n, const std::string& str) { eff_[n].id = str; }
	void clearEffectId(int n) { eff_[n].id = EFF_ID_NONE; }
	bool hasEffectId(int n) const { return eff_[n].id != EFF_ID_NONE; }

	static bool testEmptyEffectId(const std::string& id) { return id == EFF_ID_NONE; }

	static constexpr int EFF_VAL_NONE = -1;
	int getEffectValue(int n) const { return eff_[n].value; }
	void setEffectValue(int n, int v) { eff_[n].value = v; }
	void clearEffectValue(int n) { eff_[n].value = EFF_VAL_NONE; }
	bool hasEffectValue(int n) const { return eff_[n].value != EFF_VAL_NONE; }

	static bool testEmptyEffectValue(int v) { return v == EFF_VAL_NONE; }

	struct PlainEffect
	{
		std::string id;
		int value;
	};

	static constexpr int N_EFFECT = 4;

	PlainEffect getEffect(int n) const { return eff_[n]; }
	void setEffect(int n, const PlainEffect& effect) { eff_[n] = effect; }
	void setEffect(int n, const std::string& id, int value)
	{
		setEffectId(n, id);
		setEffectValue(n, value);
	}
	void clearEffect(int n)
	{
		clearEffectId(n);
		clearEffectValue(n);
	}

	static constexpr int N_COLUMN = 3 + N_EFFECT * 2;

	void clear();

	bool hasEvent() const;

private:
	/// noteNum_
	///		0<=: note number (key on)
	///		 -1: none
	///		 -2: key off
	///		 -3: echo previous note
	///		 -4: echo 2 notes before
	///		 -5: echo 3 notes before
	///		 -6: echo 4 notes before
	///		 -7: key cut
	int note_;
	/// instNum_
	///		0<=: instrument number
	///		 -1: none
	int inst_;
	/// vol_
	///		0<=: volume level
	///		 -1: none
	int vol_;
	/// eff
	///	[id]
	///		 "--": none
	///		other: effect ID
	/// [value]
	///		0<=: effect value
	///		 -1: none
	PlainEffect eff_[N_EFFECT];
};
