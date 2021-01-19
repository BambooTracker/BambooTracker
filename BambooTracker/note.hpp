/*
 * Copyright (C) 2018-2021 Rerrah
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

#include <cstdint>
#include <utility>

class Note;

namespace note_utils
{
uint16_t calculateFNumber(int absPitch, int finePitch);
uint16_t calculateSSGSquareTP(int absPitch, int finePitch);
uint16_t calculateSSGTriangleEP(int absPitch, int finePitch);
uint16_t calculateSSGSawEP(int absPitch, int finePitch);
}

class Note
{
public:
	enum NoteName { C = 0, CS, D, DS, E, F, FS, G, GS, A, AS, B };

	static constexpr int OCTAVE_RANGE = 8;
	static constexpr int DEFAULT_OCTAVE = 4;
	static constexpr int NOTE_NUMBER_RANGE = OCTAVE_RANGE * 12;		// 96
	static constexpr int DEFAULT_NOTE_NUM = DEFAULT_OCTAVE * 12;	// C4
	static constexpr int SEMINOTE_PITCH = 32;
	static constexpr int ABS_PITCH_RANGE = NOTE_NUMBER_RANGE * SEMINOTE_PITCH;	// 3072

	Note(int octave, NoteName name, int pitch = 0);
	explicit Note(int noteNum = DEFAULT_NOTE_NUM);

	void setOctave(int octave) noexcept;
	int getOctave();

	void setNoteName(NoteName name) noexcept;
	NoteName getNotename();

	void setPitch(int pitch) noexcept;
	int getPitch();

	int getAbsolutePicth();
	int getNoteNumber();

	void add(const Note& note);
	void add(int octave, NoteName name, int pitch = 0);
	void addPitch(int pitch);
	void addNoteNumber(int nn);

	friend bool operator==(Note& a, Note& b);
	friend bool operator!=(Note& a, Note& b) { return !(a == b); }

	friend Note operator+(const Note& a, const Note& b);
	friend Note operator+(const Note& a, int b);
	friend Note operator+(int a, const Note& b);

	Note& operator+=(const Note& b);
	Note& operator+=(int b);

private:
	int octave_;
	int name_;
	int pitch_;
	bool request_eval_;

	Note(int octave, int name, int pitch);
	void evaluateState();
	void evaluateState(int octave, int note, int pitch);
};

inline Note::Note(int octave, NoteName name, int pitch)
	: octave_(octave), name_(name), pitch_(pitch), request_eval_(true)
{
}

inline Note::Note(int octave, int name, int pitch)
	: octave_(octave), name_(name), pitch_(pitch), request_eval_(true)
{
}

inline void Note::setOctave(int octave) noexcept
{
	octave_ = octave;
	request_eval_ = true;
}

inline int Note::getOctave()
{
	if (request_eval_) evaluateState();
	return octave_;
}

inline void Note::setNoteName(NoteName name) noexcept
{
	name_ = name;
	request_eval_ = true;
}

inline Note::NoteName Note::getNotename()
{
	if (request_eval_) evaluateState();
	return static_cast<NoteName>(name_);
}

inline void Note::setPitch(int pitch) noexcept
{
	pitch_ = pitch;
	request_eval_ = true;
}

inline int Note::getPitch()
{
	if (request_eval_) evaluateState();
	return pitch_;
}

inline int Note::getAbsolutePicth()
{
	if (request_eval_) evaluateState();
	return pitch_ + SEMINOTE_PITCH * (name_ + 12 * octave_);
}

inline int Note::getNoteNumber()
{
	if (request_eval_) evaluateState();
	return 12 * octave_ + name_;
}

inline void Note::add(const Note& note)
{
	add(note.octave_, static_cast<NoteName>(note.name_), note.pitch_);
}

inline void Note::add(int octave, NoteName name, int pitch)
{
	octave_ += octave;
	name_ += name;
	pitch_ += pitch;
	request_eval_ = true;
}

inline void Note::addPitch(int pitch)
{
	pitch_ += pitch;
	request_eval_ = true;
}

inline void Note::addNoteNumber(int nn)
{
	name_ += nn;
	request_eval_ = true;
}

inline void Note::evaluateState()
{
	evaluateState(octave_, name_, pitch_);
}

inline bool operator==(Note& a, Note& b)
{
	if (a.request_eval_) a.evaluateState();
	if (b.request_eval_) b.evaluateState();
	return (a.octave_ == b.octave_ && a.name_ == b.name_ && a.pitch_ == b.pitch_);
}

inline Note operator+(const Note& a, const Note& b)
{
	return Note(a) += b;
}

inline Note operator+(const Note& a, int b)
{
	return Note(a) += b;
}

inline Note operator+(int a, const Note& b)
{
	return Note(b) += a;
}

inline Note& Note::operator+=(const Note& b)
{
	add(b);
	return *this;
}

inline Note& Note::operator+=(int b)
{
	addPitch(b);
	return *this;
}

