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

#include <set>

class AbstractInstrumentProperty
{
public:
	virtual ~AbstractInstrumentProperty() = default;

	inline void setNumber(int num) noexcept { num_ = num; }
	inline int getNumber() const noexcept { return num_; }

	void registerUserInstrument(int instNum);
	void deregisterUserInstrument(int instNum);
	bool isUserInstrument() const;
	inline std::multiset<int> getUserInstruments() const noexcept { return users_; }
	void clearUserInstruments();

	virtual bool isEdited() const = 0;
	virtual void clearParameters() = 0;

protected:
	explicit AbstractInstrumentProperty(int num);

private:
	int num_;
	std::multiset<int> users_;
};
