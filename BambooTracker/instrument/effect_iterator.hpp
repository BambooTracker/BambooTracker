/*
 * Copyright (C) 2018-2020 Rerrah
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
#include <vector>
#include "sequence_iterator_interface.hpp"

class ArpeggioEffectIterator : public SequenceIteratorInterface
{
public:
	ArpeggioEffectIterator(int second, int third);
	int getPosition() const override;
	int getSequenceType() const override;
	int getCommandType() const override;
	int getCommandData() const override;
	int next(bool isReleaseBegin = false) override;
	int front() override;
	int end() override;

private:
	int pos_;
	bool started_;
	int second_, third_;
};

class ArpeggioEffectIterator2 : public SequenceIterator2<int>
{
public:
	ArpeggioEffectIterator2(int second, int third);
	SequenceType type() const noexcept override { return SequenceType::NO_SEQUENCE_TYPE; }

	static constexpr int NO_ARP_DATA = -1;
	int data() const noexcept override;

	int next() override;
	int front() override;
	inline int release() override { return next(); }
	int end() override;

private:
	bool started_;
	int second_, third_;
};

class WavingEffectIterator : public SequenceIteratorInterface
{
public:
	WavingEffectIterator(int period, int depth);
	int getPosition() const override;
	int getSequenceType() const override;
	int getCommandType() const override;
	int getCommandData() const override;
	int next(bool isReleaseBegin = false) override;
	int front() override;
	int end() override;

private:
	int pos_;
	bool started_;
	std::vector<int> seq_;
};

class WavingEffectIterator2 : public SequenceIterator2<int>
{
public:
	WavingEffectIterator2(int period, int depth);
	SequenceType type() const noexcept override { return SequenceType::NO_SEQUENCE_TYPE; }

	static constexpr int NO_WAVE_DATA = -1;
	int data() const override;

	int next() override;
	int front() override;
	inline int release() override { return next(); }
	int end() override;

private:
	int pos_;
	bool started_;
	std::vector<int> seq_;
};

class NoteSlideEffectIterator : public SequenceIteratorInterface
{
public:
	NoteSlideEffectIterator(int speed, int seminote);
	int getPosition() const override;
	int getSequenceType() const override;
	int getCommandType() const override;
	int getCommandData() const override;
	int next(bool isReleaseBegin = false) override;
	int front() override;
	int end() override;

private:
	int pos_;
	bool started_;
	std::vector<int> seq_;
};

class NoteSlideEffectIterator2 : public SequenceIterator2<int>
{
public:
	NoteSlideEffectIterator2(int speed, int seminote);
	SequenceType type() const noexcept override { return SequenceType::NO_SEQUENCE_TYPE; }

	static constexpr int NO_SLIDE_DATA = -1;
	int data() const override;

	int next() override;
	int front() override;
	inline int release() override { return next(); }
	int end() override;

private:
	int pos_;
	bool started_;
	std::vector<int> seq_;
};
