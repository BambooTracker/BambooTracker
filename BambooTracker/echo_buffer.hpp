/*
 * Copyright (C) 2021 Rerrah
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

#include <deque>
#include "note.hpp"

class EchoBuffer
{
	using DequeType = std::deque<Note>;
public:
	using reference = DequeType::reference;
	using const_reference = DequeType::const_reference;
	using size_type = DequeType::size_type;

	reference at(size_type n) { return deque_.at(n); }
	const_reference at(size_type n) const { return deque_.at(n); }

	reference operator[](size_type n) { return deque_[n]; }
	const_reference operator[](size_type n) const { return deque_[n]; }

	reference latest() { return deque_.front(); }
	const_reference latest() const { return deque_.front(); }

	size_type size() const noexcept { return deque_.size(); }

	void clear() noexcept { deque_.clear(); }

	void push(const Note& y) {
		deque_.push_front(y);
		if (MAX_ <= deque_.size()) deque_.pop_back();
	}

private:
	std::deque<Note> deque_;
	static constexpr size_type MAX_ = 4;
};
