#include "c86ctl_wrapper.hpp"
#include "c86ctl.h"

#ifndef _C86CTL_H
namespace c86ctl {
	using IRealChipBase = void;
	using IRealChip2 = void;
	using IGimic2 = void;
}
#endif

C86ctlBase::C86ctlBase(void (*func)())
	: base_(nullptr)
{
#ifdef _C86CTL_H
	if (auto createInstance = reinterpret_cast<HRESULT(WINAPI*)(REFIID, void**)>(func))
		createInstance(c86ctl::IID_IRealChipBase, reinterpret_cast<void**>(&base_));
#endif
}

bool C86ctlBase::isEmpty() const
{
	return (base_ == nullptr);
}

void C86ctlBase::initialize()
{
#ifdef _C86CTL_H
	if (base_) base_->initialize();
#endif
}

void C86ctlBase::deinitialize()
{
#ifdef _C86CTL_H
	if (base_) base_->deinitialize();
#endif
}

int C86ctlBase::getNumberOfChip()
{
#ifdef _C86CTL_H
	if (base_) return base_->getNumberOfChip();
#endif
	return 0;
}

C86ctlRealChip* C86ctlBase::getChipInterface(int id)
{
	c86ctl::IRealChip2* rc = nullptr;
#ifdef _C86CTL_H
	if (base_) base_->getChipInterface(id, c86ctl::IID_IRealChip2, reinterpret_cast<void**>(&rc));
#endif
	return (rc ? new C86ctlRealChip(rc) : nullptr);
}

C86ctlRealChip::C86ctlRealChip(c86ctl::IRealChip2* rc) : rc_(rc) {}

C86ctlRealChip::~C86ctlRealChip()
{
#ifdef _C86CTL_H
	rc_->Release();
#endif
}

void C86ctlRealChip::resetChip()
{
#ifdef _C86CTL_H
	rc_->reset();
#endif
}

void C86ctlRealChip::out(uint32_t addr, uint8_t data)
{
#ifdef _C86CTL_H
	rc_->out(addr, data);
#endif
}

C86ctlGimic* C86ctlRealChip::queryInterface()
{
#ifdef _C86CTL_H
	c86ctl::IGimic2* gm = nullptr;
	if (rc_->QueryInterface(c86ctl::IID_IGimic2, reinterpret_cast<void**>(&gm)) == S_OK) {
		c86ctl::ChipType type;
		gm->getModuleType(&type);
		if (type == c86ctl::CHIP_OPNA) {
			return new C86ctlGimic(gm);
		}
		gm->Release();
	}
#endif
	return nullptr;
}

C86ctlGimic::C86ctlGimic(c86ctl::IGimic2* gm) : gm_(gm) {}

C86ctlGimic::~C86ctlGimic()
{
#ifdef _C86CTL_H
	gm_->Release();
#endif
}

void C86ctlGimic::setSSGVolume(uint8_t vol)
{
#ifdef _C86CTL_H
	gm_->setSSGVolume(vol);
#endif
}
