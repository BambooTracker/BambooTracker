/*
 * Copyright (C) 2018 Rerrah
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

#include "step.hpp"

Step::Step()
	: noteNum_(-1),
	  instNum_(-1),
	  vol_(-1)
{
	for (size_t i = 0; i < 4; ++i) {
		effID_[i] = "--";
		effVal_[i] = -1;
	}
}

int Step::getNoteNumber() const
{
	return noteNum_;
}

void Step::setNoteNumber(int num)
{
	noteNum_ = num;
}

int Step::getInstrumentNumber() const
{
	return instNum_;
}

void Step::setInstrumentNumber(int num)
{
	instNum_ = num;
}

int Step::getVolume() const
{
	return vol_;
}

void Step::setVolume(int volume)
{
	vol_ = volume;
}

std::string Step::getEffectID(int n) const
{
	return effID_[n];
}

void Step::setEffectID(int n, std::string str)
{
	effID_[n] = str;
}

int Step::getEffectValue(int n) const
{
	return effVal_[n];
}

void Step::setEffectValue(int n, int v)
{
	effVal_[n] = v;
}

int Step::checkEffectID(std::string str) const
{
	for (int i = 0; i < 4; ++i) {
		if (effID_[i] == str && effVal_[i] != -1) return i;
	}
	return -1;
}

bool Step::existCommand() const
{
	if (noteNum_ != -1) return true;
	if (instNum_ != -1) return true;
	if (vol_ != -1) return true;
	for (int i = 0; i < 4; ++i) {
		if (effID_[i] != "--") return true;
		if (effVal_[i] != -1) return true;
	}
	return false;
}
