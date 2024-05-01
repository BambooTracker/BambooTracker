/***
	c86ctl
	
	Copyright (c) 2009-2012, honet. All rights reserved.
	This software is licensed under the BSD license.

	honet.kk(at)gmail.com
 */


#ifndef _C86CTL_H
#define _C86CTL_H

#include <ObjBase.h>
#include "cbus_boardtype.h"

#ifdef __cplusplus
namespace c86ctl{
#endif

/*----------------------------------------------------------------------------*/
/*  定数定義                                                                  */
/*----------------------------------------------------------------------------*/
#define C86CTL_ERR_NONE						0
#define C86CTL_ERR_UNKNOWN					-1
#define C86CTL_ERR_INVALID_PARAM			-2
#define C86CTL_ERR_UNSUPPORTED				-3
#define C86CTL_ERR_NODEVICE					-1000
#define C86CTL_ERR_NOT_IMPLEMENTED			-9999

enum ChipType {
	CHIP_UNKNOWN		= 0x0000,
	
	CHIP_OPNA			= 0x00001,
	CHIP_YM2608			= 0x00001,
	CHIP_YM2608NOADPCM	= 0x10001,
	CHIP_OPM			= 0x00002,
	CHIP_YM2151			= 0x00002,
	CHIP_OPN3L			= 0x00003,
	CHIP_YMF288			= 0x00003,
	CHIP_OPL3			= 0x00004,
	CHIP_YMF262			= 0x00004,
	CHIP_OPLL			= 0x00005,
	CHIP_YM2413			= 0x00005,
	
	CHIP_SN76489		= 0x00006,
	CHIP_SN76496		= 0x10006,
	CHIP_AY38910		= 0x00007,
	CHIP_YM2149			= 0x10007,
	CHIP_YM2203			= 0x00008,
	CHIP_YM2612			= 0x00009,
	CHIP_YM3526			= 0x0000a,
	CHIP_YM3812			= 0x0000b,
	CHIP_YMF271			= 0x0000c,
	CHIP_YMF278B		= 0x0000d,
	CHIP_YMZ280B		= 0x0000e,
	CHIP_YMF297			= 0x0000f,
	CHIP_YMF297_OPN3L	= 0x0000f,
	CHIP_YMF297_OPL3	= 0x2000f,
	CHIP_YM2610B		= 0x00010,
	CHIP_Y8950			= 0x00020,
	CHIP_Y8950ADPCM 	= 0x10020,
    CHIP_YM3438         = 0x00021,

	CHIP_TMS3631RI104   = 0x00023,
};


/*----------------------------------------------------------------------------*/
/*  構造体定義                                                                */
/*----------------------------------------------------------------------------*/
struct Devinfo{
	char Devname[16];
	char Rev;
	char Serial[15];
};

/*----------------------------------------------------------------------------*/
/*  Interface定義                                                             */
/*----------------------------------------------------------------------------*/
// IRealChipBase {5C457918-E66D-4AC1-8CB5-B91C4704DF79}
static const GUID IID_IRealChipBase = 
{ 0x5c457918, 0xe66d, 0x4ac1, { 0x8c, 0xb5, 0xb9, 0x1c, 0x47, 0x4, 0xdf, 0x79 } };

interface IRealChipBase : public IUnknown
{
	virtual int __stdcall initialize(void) = 0;
	virtual int __stdcall deinitialize(void) = 0;
	virtual int __stdcall getNumberOfChip(void) = 0;
	virtual HRESULT __stdcall getChipInterface( int id, REFIID riid, void** ppi ) = 0;
};

// ---------------------------------------
// DEPRECATED. use IRealChip3 instead.
// IRealChip {F959C007-6B4D-46F3-BB60-9B0897C7E642}
static const GUID IID_IRealChip = 
{ 0xf959c007, 0x6b4d, 0x46f3, { 0xbb, 0x60, 0x9b, 0x8, 0x97, 0xc7, 0xe6, 0x42 } };
interface IRealChip : public IUnknown
{
public:
	virtual int __stdcall reset(void) = 0;
	virtual void __stdcall out(UINT addr, UCHAR data) = 0;
	virtual UCHAR __stdcall in(UINT addr) = 0;
};

// DEPRECATED. use IRealChip3 instead.
// IRealChip2 {BEFA830A-0DF3-46E4-A79E-FABB78E80357}
static const GUID IID_IRealChip2 = 
{ 0xbefa830a, 0xdf3, 0x46e4, { 0xa7, 0x9e, 0xfa, 0xbb, 0x78, 0xe8, 0x3, 0x57 } };

interface IRealChip2 : public IRealChip
{
	//IRealChipより継承
	//public:
	//	virtual int __stdcall reset(void) = 0;
	//	virtual void __stdcall out(UINT addr, UCHAR data) = 0;
	//	virtual UCHAR __stdcall in(UINT addr) = 0;
public:
	virtual int __stdcall getChipStatus(UINT addr, UCHAR* status) = 0;
	virtual void __stdcall directOut(UINT addr, UCHAR data) = 0;
};

// IRealChip3 {761DB10B-2432-4747-AC75-0EA6D9336797}
static const GUID IID_IRealChip3 =
{ 0x761db10b, 0x2432, 0x4747, { 0xac, 0x75, 0xe, 0xa6, 0xd9, 0x33, 0x67, 0x97 } };

interface IRealChip3 : public IRealChip2
{
	//IRealChip2より継承
	//public:
	//	virtual int __stdcall reset(void) = 0;
	//	virtual void __stdcall out(UINT addr, UCHAR data) = 0;
	//	virtual UCHAR __stdcall in(UINT addr) = 0;
	//public:
	//	virtual int __stdcall getChipStatus(UINT addr, UCHAR *status) = 0;
	//	virtual void __stdcall directOut(UINT addr, UCHAR data) = 0;
public:
	virtual int __stdcall getChipType(enum ChipType* type) = 0;
};


// ---------------------------------------
// DEPRECATED. use IGimic2 instead.
// IGimic {175C7DA0-8AA5-4173-96DA-BB43B8EB8F17}
static const GUID IID_IGimic = 
{ 0x175c7da0, 0x8aa5, 0x4173, { 0x96, 0xda, 0xbb, 0x43, 0xb8, 0xeb, 0x8f, 0x17 } };
interface IGimic : public IUnknown
{
	virtual int __stdcall getFWVer(UINT* major, UINT* minor, UINT* revision, UINT* build) = 0;
	virtual int __stdcall getMBInfo(struct Devinfo* info) = 0;
	virtual int __stdcall getModuleInfo(struct Devinfo* info) = 0;
	virtual int __stdcall setSSGVolume(UCHAR vol) = 0;
	virtual int __stdcall getSSGVolume(UCHAR* vol) = 0;
	virtual int __stdcall setPLLClock(UINT clock) = 0;
	virtual int __stdcall getPLLClock(UINT* clock) = 0;
};

// IGimic2 {47141A01-15F5-4BF5-9554-CA7AACD54BB8}
static const GUID IID_IGimic2 = 
{ 0x47141a01, 0x15f5, 0x4bf5, { 0x95, 0x54, 0xca, 0x7a, 0xac, 0xd5, 0x4b, 0xb8 } };
interface IGimic2 : public IGimic
{
	virtual int __stdcall getModuleType( enum ChipType *type ) = 0;
};


// ---------------------------------------
// 仮定義です。まだ使えません・・・ごめん。
// IC86Usb {312481E2-A93C-4A2F-87CA-CE3AC1096ED5}
static const GUID IID_IC86BOX = 
{ 0x312481e2, 0xa93c, 0x4a2f, { 0x87, 0xca, 0xce, 0x3a, 0xc1, 0x9, 0x6e, 0xd5 } };
interface IC86Box : public IUnknown
{
	virtual int __stdcall getFWVer(UINT *major, UINT *minor, UINT *revision, UINT *build) = 0;
	virtual int __stdcall getBoardType(CBUS_BOARD_TYPE *type) = 0;
	virtual int __stdcall getSlotIndex() = 0;
	virtual int __stdcall writeBoardControl(UINT index, UINT val) = 0;
};


/*----------------------------------------------------------------------------*/
/*  公開関数定義                                                              */
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI CreateInstance( REFIID riid, void** ppi );


int WINAPI c86ctl_initialize(void);					// DEPRECATED
int WINAPI c86ctl_deinitialize(void);				// DEPRECATED
int WINAPI c86ctl_reset(void);						// DEPRECATED
void WINAPI c86ctl_out( UINT addr, UCHAR data );	// DEPRECATED
UCHAR WINAPI c86ctl_in( UINT addr );				// DEPRECATED



#ifdef __cplusplus
}
}
#endif

#endif
