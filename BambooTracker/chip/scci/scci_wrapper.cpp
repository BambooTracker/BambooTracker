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

#ifdef _WIN32
#include "scci.hpp"
#include "SCCIDefines.hpp"
#endif

Scci::Scci()
{
#ifdef _WIN32
	scciManager_ = nullptr;
	scciChip_ = nullptr;
#endif
}

Scci::~Scci()
{
#ifdef _WIN32
	if (scciChip_) scciManager_->releaseSoundChip(scciChip_);
	if (scciManager_) scciManager_->releaseInstance();
#endif
}

bool Scci::createInstance(ScciGeneratorFunc* f)
{
#ifdef _WIN32
	auto getManager = f ? reinterpret_cast<scci::SCCIFUNC>((*f)()) : nullptr;
	if (f) delete f;
	auto man = getManager ? getManager() : nullptr;
	if (man) {
		scciManager_ = man;
		scciManager_->initializeInstance();
		scciManager_->reset();
		scciChip_ = scciManager_->getSoundChip(scci::SC_TYPE_YM2608, scci::SC_CLOCK_7987200);
		if (!scciChip_) {
			scciManager_->releaseInstance();
			scciManager_ = nullptr;
			return false;
		}
		return true;
	}
	else {
		if (!scciChip_) return false;
		scciManager_->releaseSoundChip(scciChip_);
		scciChip_ = nullptr;

		scciManager_->releaseInstance();
		scciManager_ = nullptr;
		return false;
	}
#else
	if (f) delete f;
	return false;
#endif
}

bool Scci::isUsed() const
{
#ifdef _WIN32
	return (scciManager_ != nullptr);
#else
	return false;
#endif
}

void Scci::initialize()
{
#ifdef _WIN32
	if (scciChip_) scciChip_->init();
#endif
}

void Scci::setRegister(uint32_t dAddr, uint32_t dData)
{
#ifdef _WIN32
	if (scciChip_) scciChip_->setRegister(dAddr, dData);
#else
	(void)dAddr;
	(void)dData;
#endif
}
