#include "c86ctl_wrapper.hpp"
#include "c86ctl.h"

C86ctlBase::C86ctlBase(void (*func)())
	: base_(nullptr)
{
#ifdef _C86CTL_H
	if (auto createInstance = reinterpret_cast<HRESULT(WINAPI*)(REFIID, void**)>(func))
		createInstance(c86ctl::IID_IRealChipBase, reinterpret_cast<void**>(&base_));
#else
	(void)func;
#endif
}

bool C86ctlBase::isEmpty() const
{
	return (base_ == nullptr);
}

void C86ctlBase::initialize()
{
	if (base_) base_->initialize();
}

void C86ctlBase::deinitialize()
{
	if (base_) base_->deinitialize();
}

int C86ctlBase::getNumberOfChip()
{
	if (base_) return base_->getNumberOfChip();
	else return 0;
}

C86ctlRealChip* C86ctlBase::getChipInterface(int id)
{
	c86ctl::IRealChip2* rc = nullptr;
	if (base_) base_->getChipInterface(id, c86ctl::IID_IRealChip2, reinterpret_cast<void**>(&rc));
	return (rc ? new C86ctlRealChip(rc) : nullptr);
}

C86ctlRealChip::C86ctlRealChip(c86ctl::IRealChip2* rc)
	: rc_(rc)
{
#ifndef _C86CTL_H
	rc_ = nullptr;
#endif
}

C86ctlRealChip::~C86ctlRealChip()
{
	if (rc_) rc_->Release();
}

void C86ctlRealChip::resetChip()
{
	if (rc_) rc_->reset();
}

void C86ctlRealChip::out(uint32_t addr, uint8_t data)
{
	if (rc_) rc_->out(addr, data);
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

C86ctlGimic::C86ctlGimic(c86ctl::IGimic2* gm)
	: gm_(gm)
{
#ifndef _C86CTL_H
	gm_ = nullptr;
#endif
}

C86ctlGimic::~C86ctlGimic()
{
	if (gm_) gm_->Release();
}

void C86ctlGimic::setSSGVolume(uint8_t vol)
{
	if (gm_) gm_->setSSGVolume(vol);
}
