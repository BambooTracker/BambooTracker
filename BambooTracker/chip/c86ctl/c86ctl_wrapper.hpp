#pragma once

#include <cstdint>

namespace c86ctl
{
struct IRealChipBase;
struct IRealChip2;
struct IGimic2;
}

class C86ctlRealChip;
class C86ctlGimic;

class C86ctlBase
{
public:
	explicit C86ctlBase(void (*func)());
	bool isEmpty() const noexcept;

	void initialize();
	void deinitialize();
	int getNumberOfChip();
	C86ctlRealChip* getChipInterface(int id);

private:
	c86ctl::IRealChipBase* base_;
};

class C86ctlRealChip
{
public:
	explicit C86ctlRealChip(c86ctl::IRealChip2* rc);
	~C86ctlRealChip();

	void resetChip();
	void out(uint32_t addr, uint8_t data);

	C86ctlGimic* queryInterface();

#ifdef _WIN32
private:
	c86ctl::IRealChip2* rc_;
#endif
};

class C86ctlGimic
{
public:
	explicit C86ctlGimic(c86ctl::IGimic2* gm);
	~C86ctlGimic();

	void setSSGVolume(uint8_t vol);

#ifdef _WIN32
private:
	c86ctl::IGimic2* gm_;
#endif
};
