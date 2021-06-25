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

#include "scci_wrapper.hpp"
#include "scci.hpp"
#include "SCCIDefines.hpp"

Scci::Scci() : man_(nullptr), chip_(nullptr) {}

Scci::~Scci()
{
	if (chip_) man_->releaseSoundChip(chip_);
	if (man_) man_->releaseInstance();
}

bool Scci::createInstance(RealChipInterfaceGeneratorFunc* f)
{
	auto getManager = f ? reinterpret_cast<scci::SCCIFUNC>((*f)()) : nullptr;
	if (f) delete f;
	auto man = getManager ? getManager() : nullptr;
	if (man) {
		man_ = man;
		man_->initializeInstance();
		man_->reset();
		chip_ = man_->getSoundChip(scci::SC_TYPE_YM2608, scci::SC_CLOCK_7987200);
		if (!chip_) {
			man_->releaseInstance();
			man_ = nullptr;
			return false;
		}
		return true;
	}
	else {
		if (!chip_) return false;
		man_->releaseSoundChip(chip_);
		chip_ = nullptr;

		man_->releaseInstance();
		man_ = nullptr;
		return false;
	}
}

bool Scci::hasConnected() const
{
	return (man_ != nullptr);
}

void Scci::reset()
{
	if (chip_) chip_->init();
}

void Scci::setRegister(uint32_t addr, uint8_t data)
{
	if (chip_) chip_->setRegister(addr, data);
}
