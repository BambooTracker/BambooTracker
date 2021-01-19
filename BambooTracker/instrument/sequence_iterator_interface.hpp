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

enum class SequenceType
{
	PlainSequence,
	AbsoluteSequence,
	FixedSequence,
	RelativeSequence
};

template<class T>
class SequenceIteratorInterface
{
public:
	virtual ~SequenceIteratorInterface() = default;

	static constexpr int END_SEQ_POS = -1;
	inline int pos() const noexcept { return pos_; }
	inline bool hasEnded() const noexcept { return pos_ == END_SEQ_POS; }

	virtual SequenceType type() const = 0;
	virtual T data() const = 0;

	virtual int next() = 0;
	virtual int front() = 0;
	virtual int release() = 0;
	virtual int end() = 0;

protected:
	explicit SequenceIteratorInterface(int initPos = 0) : pos_(initPos) {}
	int pos_;
};
