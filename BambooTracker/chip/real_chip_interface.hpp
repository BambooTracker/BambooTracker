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

#include <cstdint>

struct RealChipInterfaceGeneratorFunc
{
	using FuncPtr = void (*)();
	RealChipInterfaceGeneratorFunc(FuncPtr fp) : fp_(fp) {}
	FuncPtr operator()() const { return fp_; }

private:
	FuncPtr fp_;
};

enum class RealChipInterfaceType : int
{
	NONE = 0,
	SCCI = 1,
	C86CTL = 2
};

class SimpleRealChipInterface
{
public:
	virtual ~SimpleRealChipInterface() = default;

	virtual bool createInstance(RealChipInterfaceGeneratorFunc* f)
	{
		if (f) delete f;
		return true;
	}

	virtual RealChipInterfaceType getType() const { return RealChipInterfaceType::NONE; }
	virtual bool hasConnected() const { return false; }
	virtual void reset() {}

	virtual void setRegister(uint32_t addr, uint8_t data)
	{
		(void)addr;
		(void)data;
	}

	virtual void setSSGVolume(double dB)
	{
		(void)dB;
	}
};
