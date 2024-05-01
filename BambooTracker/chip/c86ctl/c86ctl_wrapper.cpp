/*
 * Copyright (C) 2020-2021 Rerrah
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

#include "c86ctl_wrapper.hpp"
#include <cmath>

#include "c86ctl.h"

C86ctl::C86ctl() : base_(nullptr), rc_(nullptr), gm_(nullptr) {}

C86ctl::~C86ctl()
{
	if (gm_) gm_->Release();
	if (rc_) rc_->Release();
	if (base_) {
		base_->deinitialize();
		base_->Release();
	}
}

bool C86ctl::createInstance(RealChipInterfaceGeneratorFunc* f)
{
	// Create instance
	auto tmpFunc = f ? reinterpret_cast<void*>((*f)()) : nullptr;	// Avoid MSVC C4191
	if (f) delete f;

	if (auto createInstance = reinterpret_cast<HRESULT(WINAPI*)(REFIID, void**)>(tmpFunc)) {
		c86ctl::IRealChipBase* base = nullptr;
		createInstance(c86ctl::IID_IRealChipBase, reinterpret_cast<void**>(&base));
		if (base_) {
			base_->Release();
			base_ = nullptr;
			if (!base) return false;
		}
		base_ = base;
		base_->initialize();
		int nChip = base_->getNumberOfChip();
		for (int i = 0; i < nChip; ++i) {
			c86ctl::IRealChip2* rc = nullptr;
			base_->getChipInterface(i, c86ctl::IID_IRealChip, reinterpret_cast<void**>(&rc));
			if (rc) {
				if (rc_) rc_->Release();
				rc_ = rc;
				rc_->reset();

				c86ctl::IGimic2* gm = nullptr;
				if (rc_->QueryInterface(c86ctl::IID_IGimic2, reinterpret_cast<void**>(&gm)) == S_OK) {
					c86ctl::ChipType type;
					gm->getModuleType(&type);
					switch (type) {
					case c86ctl::CHIP_YM2608:
					case c86ctl::CHIP_YM2608NOADPCM:
						if (gm_) gm_->Release();
						gm_ = gm;
						return true;

					default:
						break;
					}
					gm->Release();
				}
				rc_->Release();
				rc_ = nullptr;
			}
		}
	}
	else {	// Clear interfaces
		if (!base_) return false;
		rc_->reset();
		gm_->Release();
		gm_ = nullptr;
		rc_->Release();
		rc_ = nullptr;
	}

	base_->deinitialize();
	base_->Release();
	base_ = nullptr;
	return false;
}

bool C86ctl::hasConnected() const
{
	return (base_ != nullptr);
}

void C86ctl::reset()
{
	if (rc_) rc_->reset();
}

void C86ctl::setRegister(uint32_t addr, uint8_t data)
{
	if (rc_) rc_->out(addr, data);
}

void C86ctl::setSSGVolume(double dB)
{
	if (gm_) {
		// NOTE: estimate SSG volume roughly
		uint8_t vol = static_cast<uint8_t>(std::round((dB < -3.0) ? (2.5 * dB + 45.5)
																  : (7. * dB + 59.)));
		gm_->setSSGVolume(vol);
	}
}
