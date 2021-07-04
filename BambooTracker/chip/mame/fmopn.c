// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh
#define YM2610B_WARNING

/*
**
** File: fm.c -- software implementation of Yamaha FM sound generator
**
** Copyright Jarek Burczynski (bujar at mame dot net)
** Copyright Tatsuyuki Satoh , MultiArcadeMachineEmulator development
**
** Version 1.5.1 (Genesis Plus GX ym2612.c rev. 368)
**
*/

/*
** History:
**
** 2006~2012  Eke-Eke (Genesis Plus GX):
** Huge thanks to Nemesis, lot of those fixes came from his tests on Sega Genesis hardware
** More informations at http://gendev.spritesmind.net/forum/viewtopic.php?t=386
**
**  TODO:
**
**  - core documentation
**  - BUSY flag support
**
**  CHANGELOG:
**
** 07-30-2014 dink (FB Alpha project):
**  - fixed missing dac channel on savestate load
**
** xx-xx-xxxx
**  - fixed LFO implementation:
**      .added support for CH3 special mode: fixes various sound effects (birds in Warlock, bug sound in Aladdin...)
**      .inverted LFO AM waveform: fixes Spider-Man & Venom : Separation Anxiety (intro), California Games (surfing event)
**      .improved LFO timing accuracy: now updated AFTER sample output, like EG/PG updates, and without any precision loss anymore.
**  - improved internal timers emulation
**  - adjusted lowest EG rates increment values
**  - fixed Attack Rate not being updated in some specific cases (Batman & Robin intro)
**  - fixed EG behavior when Attack Rate is maximal
**  - fixed EG behavior when SL=0 (Mega Turrican tracks 03,09...) or/and Key ON occurs at minimal attenuation
**  - implemented EG output immediate changes on register writes
**  - fixed YM2612 initial values (after the reset): fixes missing intro in B.O.B
**  - implemented Detune overflow (Ariel, Comix Zone, Shaq Fu, Spiderman & many other games using GEMS sound engine)
**  - implemented accurate CSM mode emulation
**  - implemented accurate SSG-EG emulation (Asterix, Beavis&Butthead, Bubba'n Stix & many other games)
**  - implemented accurate address/data ports behavior
**
** 06-23-2007 Zsolt Vasvari:
**  - changed the timing not to require the use of floating point calculations
**
** 03-08-2003 Jarek Burczynski:
**  - fixed YM2608 initial values (after the reset)
**  - fixed flag and irqmask handling (YM2608)
**  - fixed BUFRDY flag handling (YM2608)
**
** 14-06-2003 Jarek Burczynski:
**  - implemented all of the YM2608 status register flags
**  - implemented support for external memory read/write via YM2608
**  - implemented support for deltat memory limit register in YM2608 emulation
**
** 22-05-2003 Jarek Burczynski:
**  - fixed LFO PM calculations (copy&paste bugfix)
**
** 08-05-2003 Jarek Burczynski:
**  - fixed SSG support
**
** 22-04-2003 Jarek Burczynski:
**  - implemented 100% correct LFO generator (verified on real YM2610 and YM2608)
**
** 15-04-2003 Jarek Burczynski:
**  - added support for YM2608's register 0x110 - status mask
**
** 01-12-2002 Jarek Burczynski:
**  - fixed register addressing in YM2608, YM2610, YM2610B chips. (verified on real YM2608)
**    The addressing patch used for early Neo-Geo games can be removed now.
**
** 26-11-2002 Jarek Burczynski, Nicola Salmoria:
**  - recreated YM2608 ADPCM ROM using data from real YM2608's output which leads to:
**  - added emulation of YM2608 drums.
**  - output of YM2608 is two times lower now - same as YM2610 (verified on real YM2608)
**
** 16-08-2002 Jarek Burczynski:
**  - binary exact Envelope Generator (verified on real YM2203);
**    identical to YM2151
**  - corrected 'off by one' error in feedback calculations (when feedback is off)
**  - corrected connection (algorithm) calculation (verified on real YM2203 and YM2610)
**
** 18-12-2001 Jarek Burczynski:
**  - added SSG-EG support (verified on real YM2203)
**
** 12-08-2001 Jarek Burczynski:
**  - corrected sin_tab and tl_tab data (verified on real chip)
**  - corrected feedback calculations (verified on real chip)
**  - corrected phase generator calculations (verified on real chip)
**  - corrected envelope generator calculations (verified on real chip)
**  - corrected FM volume level (YM2610 and YM2610B).
**  - changed YMxxxUpdateOne() functions (YM2203, YM2608, YM2610, YM2610B, YM2612) :
**    this was needed to calculate YM2610 FM channels output correctly.
**    (Each FM channel is calculated as in other chips, but the output of the channel
**    gets shifted right by one *before* sending to accumulator. That was impossible to do
**    with previous implementation).
**
** 23-07-2001 Jarek Burczynski, Nicola Salmoria:
**  - corrected YM2610 ADPCM type A algorithm and tables (verified on real chip)
**
** 11-06-2001 Jarek Burczynski:
**  - corrected end of sample bug in ADPCMA_calc_cha().
**    Real YM2610 checks for equality between current and end addresses (only 20 LSB bits).
**
** 08-12-98 hiro-shi:
** rename ADPCMA -> ADPCMB, ADPCMB -> ADPCMA
** move ROM limit check.(CALC_CH? -> 2610Write1/2)
** test program (ADPCMB_TEST)
** move ADPCM A/B end check.
** ADPCMB repeat flag(no check)
** change ADPCM volume rate (8->16) (32->48).
**
** 09-12-98 hiro-shi:
** change ADPCM volume. (8->16, 48->64)
** replace ym2610 ch0/3 (YM-2610B)
** change ADPCM_SHIFT (10->8) missing bank change 0x4000-0xffff.
** add ADPCM_SHIFT_MASK
** change ADPCMA_DECODE_MIN/MAX.
*/




/************************************************************************/
/*    comment of hiro-shi(Hiromitsu Shioya)                             */
/*    YM2610(B) = OPN-B                                                 */
/*    YM2610  : PSG:3ch FM:4ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/*    YM2610B : PSG:3ch FM:6ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/************************************************************************/

#include <stdlib.h>
#include <string.h>	// for memset
#include <stddef.h>	// for NULL
#define _USE_MATH_DEFINES
#include <math.h>

#include "fmopn.h"


/* include external DELTA-T unit (when needed) */
#if (BUILD_YM2608||BUILD_YM2610||BUILD_YM2610B)
	#include "ymdeltat.h"
#endif

/* shared function building option */
#define BUILD_OPN (BUILD_YM2203||BUILD_YM2608||BUILD_YM2610||BUILD_YM2610B||BUILD_YM2612)
#define BUILD_OPN_PRESCALER (BUILD_YM2203||BUILD_YM2608)


/* globals */
#define TYPE_SSG    0x01    /* SSG support          */
#define TYPE_LFOPAN 0x02    /* OPN type LFO and PAN */
#define TYPE_6CH    0x04    /* FM 6CH / 3CH         */
#define TYPE_DAC    0x08    /* YM2612's DAC device  */
#define TYPE_ADPCM  0x10    /* two ADPCM units      */
#define TYPE_2610   0x20    /* bogus flag to differentiate 2608 from 2610 */


#define TYPE_YM2203 (TYPE_SSG)
#define TYPE_YM2608 (TYPE_SSG |TYPE_LFOPAN |TYPE_6CH |TYPE_ADPCM)
#define TYPE_YM2610 (TYPE_SSG |TYPE_LFOPAN |TYPE_6CH |TYPE_ADPCM |TYPE_2610)
#define TYPE_YM2612 (TYPE_DAC |TYPE_LFOPAN |TYPE_6CH)


/* globals */
#define FREQ_SH         16  /* 16.16 fixed point (frequency calculations) */
#define EG_SH           16  /* 16.16 fixed point (envelope generator timing) */
#define LFO_SH          24  /*  8.24 fixed point (LFO calculations)       */
#define TIMER_SH        16  /* 16.16 fixed point (timers calculations)    */

#define FREQ_MASK       ((1<<FREQ_SH)-1)

/* envelope generator */
#define ENV_BITS        10
#define ENV_LEN         (1<<ENV_BITS)
#define ENV_STEP        (128.0/ENV_LEN)

#define MAX_ATT_INDEX   (ENV_LEN-1) /* 1023 */
#define MIN_ATT_INDEX   (0)         /* 0 */

#define EG_ATT          4
#define EG_DEC          3
#define EG_SUS          2
#define EG_REL          1
#define EG_OFF          0

/* operator unit */
#define SIN_BITS        10
#define SIN_LEN         (1<<SIN_BITS)
#define SIN_MASK        (SIN_LEN-1)

#define TL_RES_LEN      (256) /* 8 bits addressing (real chip) */

/*  TL_TAB_LEN is calculated as:
*   13 - sinus amplitude bits     (Y axis)
*   2  - sinus sign bit           (Y axis)
*   TL_RES_LEN - sinus resolution (X axis)
*/
#define TL_TAB_LEN (13*2*TL_RES_LEN)
static signed int tl_tab[TL_TAB_LEN];

#define ENV_QUIET       (TL_TAB_LEN>>3)

/* sin waveform table in 'decibel' scale */
static unsigned int sin_tab[SIN_LEN];

/* sustain level table (3dB per step) */
/* bit0, bit1, bit2, bit3, bit4, bit5, bit6 */
/* 1,    2,    4,    8,    16,   32,   64   (value)*/
/* 0.75, 1.5,  3,    6,    12,   24,   48   (dB)*/

/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/
/* attenuation value (10 bits) = (SL << 2) << 3 */
#define SC(db) (UINT32) ( db * (4.0/ENV_STEP) )
static const UINT32 sl_table[16]={
	SC( 0),SC( 1),SC( 2),SC(3 ),SC(4 ),SC(5 ),SC(6 ),SC( 7),
	SC( 8),SC( 9),SC(10),SC(11),SC(12),SC(13),SC(14),SC(31)
};
#undef SC


#define RATE_STEPS (8)
static const UINT8 eg_inc[19*RATE_STEPS]={
/*cycle:0 1  2 3  4 5  6 7*/

/* 0 */ 0,1, 0,1, 0,1, 0,1, /* rates 00..11 0 (increment by 0 or 1) */
/* 1 */ 0,1, 0,1, 1,1, 0,1, /* rates 00..11 1 */
/* 2 */ 0,1, 1,1, 0,1, 1,1, /* rates 00..11 2 */
/* 3 */ 0,1, 1,1, 1,1, 1,1, /* rates 00..11 3 */

/* 4 */ 1,1, 1,1, 1,1, 1,1, /* rate 12 0 (increment by 1) */
/* 5 */ 1,1, 1,2, 1,1, 1,2, /* rate 12 1 */
/* 6 */ 1,2, 1,2, 1,2, 1,2, /* rate 12 2 */
/* 7 */ 1,2, 2,2, 1,2, 2,2, /* rate 12 3 */

/* 8 */ 2,2, 2,2, 2,2, 2,2, /* rate 13 0 (increment by 2) */
/* 9 */ 2,2, 2,4, 2,2, 2,4, /* rate 13 1 */
/*10 */ 2,4, 2,4, 2,4, 2,4, /* rate 13 2 */
/*11 */ 2,4, 4,4, 2,4, 4,4, /* rate 13 3 */

/*12 */ 4,4, 4,4, 4,4, 4,4, /* rate 14 0 (increment by 4) */
/*13 */ 4,4, 4,8, 4,4, 4,8, /* rate 14 1 */
/*14 */ 4,8, 4,8, 4,8, 4,8, /* rate 14 2 */
/*15 */ 4,8, 8,8, 4,8, 8,8, /* rate 14 3 */

/*16 */ 8,8, 8,8, 8,8, 8,8, /* rates 15 0, 15 1, 15 2, 15 3 (increment by 8) */
/*17 */ 16,16,16,16,16,16,16,16, /* rates 15 2, 15 3 for attack */
/*18 */ 0,0, 0,0, 0,0, 0,0, /* infinity rates for attack and decay(s) */
};


#define O(a) (a*RATE_STEPS)

/*note that there is no O(17) in this table - it's directly in the code */
static const UINT8 eg_rate_select[32+64+32]={  /* Envelope Generator rates (32 + 64 rates + 32 RKS) */
/* 32 infinite time rates (same as Rate 0) */
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),

/* rates 00-11 */
/*
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
*/
O(18),O(18),O( 0),O( 0),
O( 0),O( 0),O( 2),O( 2),   // Nemesis's tests

O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),

/* rate 12 */
O( 4),O( 5),O( 6),O( 7),

/* rate 13 */
O( 8),O( 9),O(10),O(11),

/* rate 14 */
O(12),O(13),O(14),O(15),

/* rate 15 */
O(16),O(16),O(16),O(16),

/* 32 dummy rates (same as 15 3) */
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16)

};
#undef O

/*rate  0,    1,    2,   3,   4,   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15*/
/*shift 11,   10,   9,   8,   7,   6,  5,  4,  3,  2, 1,  0,  0,  0,  0,  0 */
/*mask  2047, 1023, 511, 255, 127, 63, 31, 15, 7,  3, 1,  0,  0,  0,  0,  0 */

#define O(a) (a*1)
static const UINT8 eg_rate_shift[32+64+32]={  /* Envelope Generator counter shifts (32 + 64 rates + 32 RKS) */
/* 32 infinite time rates */
/* O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0), */

/* fixed (should be the same as rate 0, even if it makes no difference since increment value is 0 for these rates) */
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),

/* rates 00-11 */
O(11),O(11),O(11),O(11),
O(10),O(10),O(10),O(10),
O( 9),O( 9),O( 9),O( 9),
O( 8),O( 8),O( 8),O( 8),
O( 7),O( 7),O( 7),O( 7),
O( 6),O( 6),O( 6),O( 6),
O( 5),O( 5),O( 5),O( 5),
O( 4),O( 4),O( 4),O( 4),
O( 3),O( 3),O( 3),O( 3),
O( 2),O( 2),O( 2),O( 2),
O( 1),O( 1),O( 1),O( 1),
O( 0),O( 0),O( 0),O( 0),

/* rate 12 */
O( 0),O( 0),O( 0),O( 0),

/* rate 13 */
O( 0),O( 0),O( 0),O( 0),

/* rate 14 */
O( 0),O( 0),O( 0),O( 0),

/* rate 15 */
O( 0),O( 0),O( 0),O( 0),

/* 32 dummy rates (same as 15 3) */
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0)

};
#undef O

static const UINT8 dt_tab[4 * 32]={
/* this is YM2151 and YM2612 phase increment data (in 10.10 fixed point format)*/
/* FD=0 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* FD=1 */
	0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
	2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 8, 8,
/* FD=2 */
	1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
	5, 6, 6, 7, 8, 8, 9,10,11,12,13,14,16,16,16,16,
/* FD=3 */
	2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7,
	8 , 8, 9,10,11,12,13,14,16,17,19,20,22,22,22,22
};


/* OPN key frequency number -> key code follow table */
/* fnum higher 4bit -> keycode lower 2bit */
static const UINT8 opn_fktable[16] = {0,0,0,0,0,0,0,1,2,3,3,3,3,3,3,3};


/* 8 LFO speed parameters */
/* each value represents number of samples that one LFO level will last for */
static const UINT32 lfo_samples_per_step[8] = {108, 77, 71, 67, 62, 44, 8, 5};



/*There are 4 different LFO AM depths available, they are:
  0 dB, 1.4 dB, 5.9 dB, 11.8 dB
  Here is how it is generated (in EG steps):

  11.8 dB = 0, 2, 4, 6, 8, 10,12,14,16...126,126,124,122,120,118,....4,2,0
   5.9 dB = 0, 1, 2, 3, 4, 5, 6, 7, 8....63, 63, 62, 61, 60, 59,.....2,1,0
   1.4 dB = 0, 0, 0, 0, 1, 1, 1, 1, 2,...15, 15, 15, 15, 14, 14,.....0,0,0

  (1.4 dB is losing precision as you can see)

  It's implemented as generator from 0..126 with step 2 then a shift
  right N times, where N is:
    8 for 0 dB
    3 for 1.4 dB
    1 for 5.9 dB
    0 for 11.8 dB
*/
static const UINT8 lfo_ams_depth_shift[4] = {8, 3, 1, 0};



/*There are 8 different LFO PM depths available, they are:
  0, 3.4, 6.7, 10, 14, 20, 40, 80 (cents)

  Modulation level at each depth depends on F-NUMBER bits: 4,5,6,7,8,9,10
  (bits 8,9,10 = FNUM MSB from OCT/FNUM register)

  Here we store only first quarter (positive one) of full waveform.
  Full table (lfo_pm_table) containing all 128 waveforms is build
  at run (init) time.

  One value in table below represents 4 (four) basic LFO steps
  (1 PM step = 4 AM steps).

  For example:
   at LFO SPEED=0 (which is 108 samples per basic LFO step)
   one value from "lfo_pm_output" table lasts for 432 consecutive
   samples (4*108=432) and one full LFO waveform cycle lasts for 13824
   samples (32*432=13824; 32 because we store only a quarter of whole
            waveform in the table below)
*/
static const UINT8 lfo_pm_output[7*8][8]={
/* 7 bits meaningful (of F-NUMBER), 8 LFO output levels per one depth (out of 32), 8 LFO depths */
/* FNUM BIT 4: 000 0001xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 6 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 7 */ {0,   0,   0,   0,   1,   1,   1,   1},

/* FNUM BIT 5: 000 0010xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 6 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 7 */ {0,   0,   1,   1,   2,   2,   2,   3},

/* FNUM BIT 6: 000 0100xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   1},
/* DEPTH 5 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 6 */ {0,   0,   1,   1,   2,   2,   2,   3},
/* DEPTH 7 */ {0,   0,   2,   3,   4,   4,   5,   6},

/* FNUM BIT 7: 000 1000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   1,   1},
/* DEPTH 3 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 4 */ {0,   0,   0,   1,   1,   1,   1,   2},
/* DEPTH 5 */ {0,   0,   1,   1,   2,   2,   2,   3},
/* DEPTH 6 */ {0,   0,   2,   3,   4,   4,   5,   6},
/* DEPTH 7 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},

/* FNUM BIT 8: 001 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 2 */ {0,   0,   0,   1,   1,   1,   2,   2},
/* DEPTH 3 */ {0,   0,   1,   1,   2,   2,   3,   3},
/* DEPTH 4 */ {0,   0,   1,   2,   2,   2,   3,   4},
/* DEPTH 5 */ {0,   0,   2,   3,   4,   4,   5,   6},
/* DEPTH 6 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
/* DEPTH 7 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},

/* FNUM BIT 9: 010 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   2,   2,   2,   2},
/* DEPTH 2 */ {0,   0,   0,   2,   2,   2,   4,   4},
/* DEPTH 3 */ {0,   0,   2,   2,   4,   4,   6,   6},
/* DEPTH 4 */ {0,   0,   2,   4,   4,   4,   6,   8},
/* DEPTH 5 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
/* DEPTH 6 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
/* DEPTH 7 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},

/* FNUM BIT10: 100 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   4,   4,   4,   4},
/* DEPTH 2 */ {0,   0,   0,   4,   4,   4,   8,   8},
/* DEPTH 3 */ {0,   0,   4,   4,   8,   8, 0xc, 0xc},
/* DEPTH 4 */ {0,   0,   4,   8,   8,   8, 0xc,0x10},
/* DEPTH 5 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
/* DEPTH 6 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},
/* DEPTH 7 */ {0,   0,0x20,0x30,0x40,0x40,0x50,0x60},

};

/* all 128 LFO PM waveforms */
static INT32 lfo_pm_table[128*8*32]; /* 128 combinations of 7 bits meaningful (of F-NUMBER), 8 LFO depths, 32 LFO output levels per one depth */

/* register number to channel number , slot offset */
#define OPN_CHAN(N) (N&3)
#define OPN_SLOT(N) ((N>>2)&3)

/* slot number */
#define SLOT1 0
#define SLOT2 2
#define SLOT3 1
#define SLOT4 3

/* bit0 = Right enable , bit1 = Left enable */
#define OUTD_RIGHT  1
#define OUTD_LEFT   2
#define OUTD_CENTER 3


/* struct describing a single operator (SLOT) */
typedef struct
{
	INT32   *DT;        /* detune          :dt_tab[DT] */
	UINT8   KSR;        /* key scale rate  :3-KSR */
	UINT32  ar;         /* attack rate  */
	UINT32  d1r;        /* decay rate   */
	UINT32  d2r;        /* sustain rate */
	UINT32  rr;         /* release rate */
	UINT8   ksr;        /* key scale rate  :kcode>>(3-KSR) */
	UINT32  mul;        /* multiple        :ML_TABLE[ML] */

	/* Phase Generator */
	UINT32  phase;      /* phase counter */
	INT32   Incr;       /* phase step */

	/* Envelope Generator */
	UINT8   state;      /* phase type */
	UINT32  tl;         /* total level: TL << 3 */
	INT32   volume;     /* envelope counter */
	UINT32  sl;         /* sustain level:sl_table[SL] */
	UINT32  vol_out;    /* current output from EG circuit (without AM from LFO) */

	UINT8   eg_sh_ar;   /*  (attack state) */
	UINT8   eg_sel_ar;  /*  (attack state) */
	UINT8   eg_sh_d1r;  /*  (decay state) */
	UINT8   eg_sel_d1r; /*  (decay state) */
	UINT8   eg_sh_d2r;  /*  (sustain state) */
	UINT8   eg_sel_d2r; /*  (sustain state) */
	UINT8   eg_sh_rr;   /*  (release state) */
	UINT8   eg_sel_rr;  /*  (release state) */

	UINT8   ssg;        /* SSG-EG waveform */
	UINT8   ssgn;       /* SSG-EG negated output */

	UINT8   key;        /* 0=last key was KEY OFF, 1=KEY ON */

	/* LFO */
	UINT32  AMmask;     /* AM enable flag */

} FM_SLOT;

typedef struct
{
	FM_SLOT SLOT[4];    /* four SLOTs (operators) */

	UINT8   ALGO;       /* algorithm */
	UINT8   FB;         /* feedback shift */
	INT32   op1_out[2]; /* op1 output for feedback */

	INT32   *connect1;  /* SLOT1 output pointer */
	INT32   *connect3;  /* SLOT3 output pointer */
	INT32   *connect2;  /* SLOT2 output pointer */
	INT32   *connect4;  /* SLOT4 output pointer */

	INT32   *mem_connect;/* where to put the delayed sample (MEM) */
	INT32   mem_value;  /* delayed sample (MEM) value */

	INT32   pms;        /* channel PMS */
	UINT8   ams;        /* channel AMS */

	UINT32  fc;         /* fnum,blk:adjusted to sample rate */
	UINT8   kcode;      /* key code:                        */
	UINT32  block_fnum; /* current blk/fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
	UINT8   Muted;
} FM_CH;


typedef struct
{
	void *      param;              /* this chip parameter  */
	UINT32      clock;              /* master clock  (Hz)   */
	UINT32      rate;               /* sampling rate (Hz)   */
	double      freqbase;           /* frequency base       */
	int         timer_prescaler;    /* timer prescaler      */
#if FM_BUSY_FLAG_SUPPORT
	TIME_TYPE   busy_expiry_time;   /* expiry time of the busy status */
#endif
	UINT8       address;            /* address register     */
	UINT8       irq;                /* interrupt level      */
	UINT8       irqmask;            /* irq mask             */
	UINT8       status;             /* status flag          */
	UINT32      mode;               /* mode  CSM / 3SLOT    */
	UINT8       fn_h;               /* freq latch           */
	UINT8       prescaler_sel;      /* prescaler selector   */
	INT32       TA;                 /* timer a              */
	INT32       TAC;                /* timer a counter      */
	UINT8       TB;                 /* timer b              */
	INT32       TBC;                /* timer b counter      */
	/* local time tables */
	INT32       dt_tab[8][32];      /* DeTune table         */
	/* Extention Timer and IRQ handler */
	FM_TIMERHANDLER timer_handler;
	FM_IRQHANDLER   IRQ_Handler;
	ssg_callbacks SSG_funcs;
	void *      SSG_param;
} FM_ST;



/***********************************************************/
/* OPN unit                                                */
/***********************************************************/

/* OPN 3slot struct */
typedef struct
{
	UINT32  fc[3];          /* fnum3,blk3: calculated */
	UINT8   fn_h;           /* freq3 latch */
	UINT8   kcode[3];       /* key code */
	UINT32  block_fnum[3];  /* current fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
	UINT8   key_csm;        /* CSM mode Key-ON flag */
} FM_3SLOT;

/* OPN/A/B common state */
typedef struct
{
	UINT8   type;           /* chip type */
	UINT8   smpRateNative;  /* emulating at native sample rate (enable sample rate change callback) */
	UINT8   LegacyMode;     /* behave like old emulation regarding FNum writes and Key Off */
	FM_ST   ST;             /* general state */
	FM_3SLOT SL3;           /* 3 slot mode state */
	FM_CH   *P_CH;          /* pointer of CH */
	UINT32  pan[6*2];       /* fm channels output masks (0xffffffff = enable) */

	UINT32  eg_cnt;         /* global envelope generator counter */
	UINT32  eg_timer;       /* global envelope generator counter works at frequency = chipclock/144/3 */
	UINT32  eg_timer_add;   /* step of eg_timer */
	UINT32  eg_timer_overflow;/* envelope generator timer overflows every 3 samples (on real chip) */


	/* there are 2048 FNUMs that can be generated using FNUM/BLK registers
	   but LFO works with one more bit of a precision so we really need 4096 elements */
	UINT32  fn_table[4096]; /* fnumber->increment counter */
	UINT32  fn_max;    /* maximal phase increment (used for phase overflow) */

	/* LFO */
	UINT8   lfo_cnt;            /* current LFO phase (out of 128) */
	UINT32  lfo_timer;          /* current LFO phase runs at LFO frequency */
	UINT32  lfo_timer_add;      /* step of lfo_timer */
	UINT32  lfo_timer_overflow; /* LFO timer overflows every N samples (depends on LFO frequency) */
	UINT32  LFO_AM;             /* current LFO AM step */
	UINT32  LFO_PM;             /* current LFO PM step */

	INT32   m2,c1,c2;       /* Phase Modulation input for operators 2,3,4 */
	INT32   mem;            /* one sample delay memory */

	INT32   out_fm[6];      /* outputs of working channels */

#if (BUILD_YM2608||BUILD_YM2610||BUILD_YM2610B)
	INT32   out_adpcm[4];   /* channel output NONE,LEFT,RIGHT or CENTER for YM2608/YM2610 ADPCM */
	INT32   out_delta[4];   /* channel output NONE,LEFT,RIGHT or CENTER for YM2608/YM2610 DELTAT*/
#endif

	DEVCB_SRATE_CHG smpRateFunc;
	void* smpRateData;
} FM_OPN;



/* log output level */
#define LOG_ERR  3      /* ERROR       */
#define LOG_WAR  2      /* WARNING     */
#define LOG_INF  1      /* INFORMATION */
#define LOG_LEVEL LOG_INF

#ifndef __RAINE__
#define LOG(n,x) do { if( (n)>=LOG_LEVEL ) logerror x; } while (0)
#endif

/* limitter */
#define Limit(val, max,min) { \
	if ( val > max )      val = max; \
	else if ( val < min ) val = min; \
}


static UINT8 tablesInit = 0;

/* status set and IRQ handling */
INLINE void FM_STATUS_SET(FM_ST *ST,int flag)
{
	/* set status flag */
	ST->status |= flag;
	if ( !(ST->irq) && (ST->status & ST->irqmask) )
	{
		ST->irq = 1;
		/* callback user interrupt handler (IRQ is OFF to ON) */
		if(ST->IRQ_Handler) (ST->IRQ_Handler)(ST->param,1);
	}
}

/* status reset and IRQ handling */
INLINE void FM_STATUS_RESET(FM_ST *ST,int flag)
{
	/* reset status flag */
	ST->status &=~flag;
	if ( (ST->irq) && !(ST->status & ST->irqmask) )
	{
		ST->irq = 0;
		/* callback user interrupt handler (IRQ is ON to OFF) */
		if(ST->IRQ_Handler) (ST->IRQ_Handler)(ST->param,0);
	}
}

/* IRQ mask set */
INLINE void FM_IRQMASK_SET(FM_ST *ST,int flag)
{
	ST->irqmask = flag;
	/* IRQ handling check */
	FM_STATUS_SET(ST,0);
	FM_STATUS_RESET(ST,0);
}

INLINE void FM_KEYON(FM_OPN *OPN, FM_CH *CH , int s )
{
	FM_SLOT *SLOT = &CH->SLOT[s];

	// Note by Valley Bell:
	//  I assume that the CSM mode shouldn't affect channels
	//  other than FM3, so I added a check for it here.
	if( !SLOT->key && (!OPN->SL3.key_csm || CH == &OPN->P_CH[2]))
	{
		/* restart Phase Generator */
		SLOT->phase = 0;

		/* reset SSG-EG inversion flag */
		SLOT->ssgn = 0;

		if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
		{
			SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
		}
		else
		{
			/* force attenuation level to 0 */
			SLOT->volume = MIN_ATT_INDEX;

			/* directly switch to Decay (or Sustain) */
			SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
		}

		/* recalculate EG output */
		if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
			SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
		else
			SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
	}

	SLOT->key = 1;
}

INLINE void FM_KEYOFF(FM_OPN *OPN, FM_CH *CH , int s )
{
	FM_SLOT *SLOT = &CH->SLOT[s];

	if (SLOT->key && (!OPN->SL3.key_csm || CH == &OPN->P_CH[2]))
	{
		if (SLOT->state>EG_REL)
		{
			SLOT->state = EG_REL; /* phase -> Release */

			/* SSG-EG specific update */
			if (SLOT->ssg&0x08)
			{
				/* convert EG attenuation level */
				if (SLOT->ssgn ^ (SLOT->ssg&0x04))
					SLOT->volume = (0x200 - SLOT->volume);

				/* force EG attenuation level */
				if (SLOT->volume >= 0x200)
				{
					SLOT->volume = MAX_ATT_INDEX;
					SLOT->state  = EG_OFF;
				}

				/* recalculate EG output */
				SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
			}
		}
		if (OPN->LegacyMode)	// workaround for VGMs trimmed with VGMTool
		{
			// When at maximum release rate AND there was a Key On just this sample, enforce an instant Key Off.
			if (SLOT->rr >= 94 && SLOT->phase == 0)
			{
				SLOT->volume = MAX_ATT_INDEX;
				SLOT->state = EG_OFF;
			}
		}
	}

	SLOT->key = 0;
}

INLINE void FM_KEYON_CSM(FM_OPN *OPN, FM_CH *CH , int s )
{
	FM_SLOT *SLOT = &CH->SLOT[s];

	if (!SLOT->key && !OPN->SL3.key_csm)
	{
		/* restart Phase Generator */
		SLOT->phase = 0;

		/* reset SSG-EG inversion flag */
		SLOT->ssgn = 0;

		if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
		{
			SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
		}
		else
		{
			/* force attenuation level to 0 */
			SLOT->volume = MIN_ATT_INDEX;

			/* directly switch to Decay (or Sustain) */
			SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
		}

		/* recalculate EG output */
		if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
			SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
		else
			SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
	}
}

INLINE void FM_KEYOFF_CSM(FM_CH *CH , int s )
{
	FM_SLOT *SLOT = &CH->SLOT[s];
	if (!SLOT->key)
	{
		if (SLOT->state>EG_REL)
		{
			SLOT->state = EG_REL; /* phase -> Release */

			/* SSG-EG specific update */
			if (SLOT->ssg&0x08)
			{
				/* convert EG attenuation level */
				if (SLOT->ssgn ^ (SLOT->ssg&0x04))
					SLOT->volume = (0x200 - SLOT->volume);

				/* force EG attenuation level */
				if (SLOT->volume >= 0x200)
				{
					SLOT->volume = MAX_ATT_INDEX;
					SLOT->state  = EG_OFF;
				}

				/* recalculate EG output */
				SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
			}
		}
	}
}

/* OPN Mode Register Write */
INLINE void set_timers( FM_OPN *OPN, FM_ST *ST, void *n, int v )
{
	/* b7 = CSM MODE */
	/* b6 = 3 slot mode */
	/* b5 = reset b */
	/* b4 = reset a */
	/* b3 = timer enable b */
	/* b2 = timer enable a */
	/* b1 = load b */
	/* b0 = load a */

	if ((OPN->ST.mode ^ v) & 0xC0)
	{
		/* phase increment need to be recalculated */
		OPN->P_CH[2].SLOT[SLOT1].Incr=-1;

		/* CSM mode disabled and CSM key ON active*/
		if (((v & 0xC0) != 0x80) && OPN->SL3.key_csm)
		{
			/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT1);
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT2);
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT3);
			FM_KEYOFF_CSM(&OPN->P_CH[2],SLOT4);
			OPN->SL3.key_csm = 0;
		}
	}

	/* reload Timers */
	if ((v&1) && !(ST->mode&1))
	{
		ST->TAC = (1024-ST->TA);
		/* External timer handler */
		if (ST->timer_handler) (ST->timer_handler)(n,0,ST->TAC * ST->timer_prescaler,ST->clock);
		ST->TAC *= 4096;
	}
	else if (!(v & 1))
	{
		if( ST->TAC != 0 )
		{
			ST->TAC = 0;
			if (ST->timer_handler) (ST->timer_handler)(n,0,0,ST->clock);
		}
	}

	if ((v&2) && !(ST->mode&2))
	{
		ST->TBC = ( 256-ST->TB)<<4;
		/* External timer handler */
		if (ST->timer_handler) (ST->timer_handler)(n,1,ST->TBC * ST->timer_prescaler,ST->clock);
		ST->TBC *= 4096;
	}
	else if (!(v & 2))
	{
		if( ST->TBC != 0 )
		{
			ST->TBC = 0;
			if (ST->timer_handler) (ST->timer_handler)(n,1,0,ST->clock);
		}
	}

	/* reset Timers flags */
	ST->status &= (~v >> 4);

	/* if IRQ should be lowered now, do so */
	if ( (ST->irq) && !(ST->status & ST->irqmask) )
	{
		ST->irq = 0;
		/* callback user interrupt handler (IRQ is ON to OFF) */
		if(ST->IRQ_Handler) (ST->IRQ_Handler)(ST->param, 0);
	}
	ST->mode = v;
}


/* Timer A Overflow */
INLINE void TimerAOver(FM_ST *ST)
{
	/* set status (if enabled) */
	if(ST->mode & 0x04) FM_STATUS_SET(ST,0x01);
	/* clear or reload the counter */
	ST->TAC = (1024-ST->TA);
	if (ST->timer_handler) (ST->timer_handler)(ST->param,0,ST->TAC * ST->timer_prescaler,ST->clock);
	ST->TAC *= 4096;
}
/* Timer B Overflow */
INLINE void TimerBOver(FM_ST *ST)
{
	/* set status (if enabled) */
	if(ST->mode & 0x08) FM_STATUS_SET(ST,0x02);
	/* clear or reload the counter */
	ST->TBC = ( 256-ST->TB)<<4;
	if (ST->timer_handler) (ST->timer_handler)(ST->param,1,ST->TBC * ST->timer_prescaler,ST->clock);
	ST->TBC *= 4096;
}


#if FM_INTERNAL_TIMER
/* ----- internal timer mode , update timer */

/* ---------- calculate timer A ---------- */
	#define INTERNAL_TIMER_A(ST,CSM_CH)                 \
	{                                                   \
		if( (ST)->TAC && ((ST)->timer_handler==0) )     \
			if( ((ST)->TAC -= (int)((ST)->freqbase*4096)) <= 0 )    \
			{                                           \
				TimerAOver( ST );                       \
				/* CSM mode total level latch and auto key on */    \
				if( (ST)->mode & 0x80 )                 \
					CSMKeyControll( OPN, CSM_CH );      \
			}                                           \
	}
/* ---------- calculate timer B ---------- */
	#define INTERNAL_TIMER_B(ST,step)                       \
	{                                                       \
		if( (ST)->TBC && ((ST)->timer_handler==0) )         \
			if( ((ST)->TBC -= (int)((ST)->freqbase*4096*step)) <= 0 )   \
				TimerBOver( ST );                           \
	}
#else /* FM_INTERNAL_TIMER */
/* external timer mode */
#define INTERNAL_TIMER_A(ST,CSM_CH)
#define INTERNAL_TIMER_B(ST,step)
#endif /* FM_INTERNAL_TIMER */



#if FM_BUSY_FLAG_SUPPORT
#define FM_BUSY_CLEAR(ST) ((ST)->busy_expiry_time = UNDEFINED_TIME)
INLINE UINT8 FM_STATUS_FLAG(FM_ST *ST)
{
	if( COMPARE_TIMES(ST->busy_expiry_time, UNDEFINED_TIME) != 0 )
	{
		if (COMPARE_TIMES(ST->busy_expiry_time, FM_GET_TIME_NOW(&ST->device->machine())) > 0)
			return ST->status | 0x80;   /* with busy */
		/* expire */
		FM_BUSY_CLEAR(ST);
	}
	return ST->status;
}
INLINE void FM_BUSY_SET(FM_ST *ST,int busyclock )
{
	TIME_TYPE expiry_period = MULTIPLY_TIME_BY_INT(attotime::from_hz(ST->clock), busyclock * ST->timer_prescaler);
	ST->busy_expiry_time = ADD_TIMES(FM_GET_TIME_NOW(&ST->device->machine()), expiry_period);
}
#else
#define FM_STATUS_FLAG(ST) ((ST)->status)
#define FM_BUSY_SET(ST,bclock) {}
#define FM_BUSY_CLEAR(ST) {}
#endif


/* set algorithm connection */
INLINE void setup_connection( FM_OPN *OPN, FM_CH *CH, int ch )
{
	INT32 *carrier = &OPN->out_fm[ch];

	INT32 **om1 = &CH->connect1;
	INT32 **om2 = &CH->connect3;
	INT32 **oc1 = &CH->connect2;

	INT32 **memc = &CH->mem_connect;

	switch( CH->ALGO )
	{
	case 0:
		/* M1---C1---MEM---M2---C2---OUT */
		*om1 = &OPN->c1;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->m2;
		break;
	case 1:
		/* M1------+-MEM---M2---C2---OUT */
		/*      C1-+                     */
		*om1 = &OPN->mem;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->m2;
		break;
	case 2:
		/* M1-----------------+-C2---OUT */
		/*      C1---MEM---M2-+          */
		*om1 = &OPN->c2;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->m2;
		break;
	case 3:
		/* M1---C1---MEM------+-C2---OUT */
		/*                 M2-+          */
		*om1 = &OPN->c1;
		*oc1 = &OPN->mem;
		*om2 = &OPN->c2;
		*memc= &OPN->c2;
		break;
	case 4:
		/* M1---C1-+-OUT */
		/* M2---C2-+     */
		/* MEM: not used */
		*om1 = &OPN->c1;
		*oc1 = carrier;
		*om2 = &OPN->c2;
		*memc= &OPN->mem;   /* store it anywhere where it will not be used */
		break;
	case 5:
		/*    +----C1----+     */
		/* M1-+-MEM---M2-+-OUT */
		/*    +----C2----+     */
		*om1 = NULL;    /* special mark */
		*oc1 = carrier;
		*om2 = carrier;
		*memc= &OPN->m2;
		break;
	case 6:
		/* M1---C1-+     */
		/*      M2-+-OUT */
		/*      C2-+     */
		/* MEM: not used */
		*om1 = &OPN->c1;
		*oc1 = carrier;
		*om2 = carrier;
		*memc= &OPN->mem;   /* store it anywhere where it will not be used */
		break;
	case 7:
		/* M1-+     */
		/* C1-+-OUT */
		/* M2-+     */
		/* C2-+     */
		/* MEM: not used*/
		*om1 = carrier;
		*oc1 = carrier;
		*om2 = carrier;
		*memc= &OPN->mem;   /* store it anywhere where it will not be used */
		break;
	}

	CH->connect4 = carrier;
}

/* set detune & multiple */
INLINE void set_det_mul(FM_ST *ST,FM_CH *CH,FM_SLOT *SLOT,int v)
{
	SLOT->mul = (v&0x0f)? (v&0x0f)*2 : 1;
	SLOT->DT  = ST->dt_tab[(v>>4)&7];
	CH->SLOT[SLOT1].Incr=-1;
}

/* set total level */
INLINE void set_tl(FM_CH *CH,FM_SLOT *SLOT , int v)
{
	(void)CH;
	SLOT->tl = (v&0x7f)<<(ENV_BITS-7); /* 7bit TL */

	/* recalculate EG output */
	if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)) && (SLOT->state > EG_REL))
		SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
	else
		SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
}

/* set attack rate & key scale  */
INLINE void set_ar_ksr(UINT8 type, FM_CH *CH,FM_SLOT *SLOT,int v)
{
	(void)type;
	UINT8 old_KSR = SLOT->KSR;

	SLOT->ar = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

	SLOT->KSR = 3-(v>>6);
	if (SLOT->KSR != old_KSR)
	{
		CH->SLOT[SLOT1].Incr=-1;
	}

	/* Even if it seems unnecessary to do it here, it could happen that KSR and KC  */
	/* are modified but the resulted SLOT->ksr value (kc >> SLOT->KSR) remains unchanged. */
	/* In such case, Attack Rate would not be recalculated by "refresh_fc_eg_slot". */
	/* This actually fixes the intro of "The Adventures of Batman & Robin" (Eke-Eke)         */
	if ((SLOT->ar + SLOT->ksr) < (32+62))
	{
		SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar  + SLOT->ksr ];
		SLOT->eg_sel_ar = eg_rate_select[SLOT->ar  + SLOT->ksr ];
	}
	else
	{
		/* verified by Nemesis on real hardware (Attack phase is blocked) */
		SLOT->eg_sh_ar  = 0;
		SLOT->eg_sel_ar = 18*RATE_STEPS;
	}
}

/* set decay rate */
INLINE void set_dr(UINT8 type, FM_SLOT *SLOT,int v)
{
	(void)type;
	SLOT->d1r = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

	SLOT->eg_sh_d1r = eg_rate_shift [SLOT->d1r + SLOT->ksr];
	SLOT->eg_sel_d1r= eg_rate_select[SLOT->d1r + SLOT->ksr];
}

/* set sustain rate */
INLINE void set_sr(UINT8 type, FM_SLOT *SLOT,int v)
{
	(void)type;
	SLOT->d2r = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

	SLOT->eg_sh_d2r = eg_rate_shift [SLOT->d2r + SLOT->ksr];
	SLOT->eg_sel_d2r= eg_rate_select[SLOT->d2r + SLOT->ksr];
}

/* set release rate */
INLINE void set_sl_rr(UINT8 type, FM_SLOT *SLOT,int v)
{
	(void)type;
	SLOT->sl = sl_table[ v>>4 ];

	/* check EG state changes */
	if ((SLOT->state == EG_DEC) && (SLOT->volume >= (INT32)(SLOT->sl)))
		SLOT->state = EG_SUS;

	SLOT->rr  = 34 + ((v&0x0f)<<2);

	SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr  + SLOT->ksr];
	SLOT->eg_sel_rr = eg_rate_select[SLOT->rr  + SLOT->ksr];
}

/* advance LFO to next sample */
INLINE void advance_lfo(FM_OPN *OPN)
{
	if (OPN->lfo_timer_overflow)   /* LFO enabled ? */
	{
		/* increment LFO timer */
		OPN->lfo_timer +=  OPN->lfo_timer_add;

		/* when LFO is enabled, one level will last for 108, 77, 71, 67, 62, 44, 8 or 5 samples */
		while (OPN->lfo_timer >= OPN->lfo_timer_overflow)
		{
			OPN->lfo_timer -= OPN->lfo_timer_overflow;

			/* There are 128 LFO steps */
			OPN->lfo_cnt = ( OPN->lfo_cnt + 1 ) & 127;

			/* triangle (inverted) */
			/* AM: from 126 to 0 step -2, 0 to 126 step +2 */
			if (OPN->lfo_cnt<64)
				OPN->LFO_AM = (OPN->lfo_cnt ^ 63) << 1;
			else
				OPN->LFO_AM = (OPN->lfo_cnt & 63) << 1;

			/* PM works with 4 times slower clock */
			OPN->LFO_PM = OPN->lfo_cnt >> 2;
		}
	}
}

INLINE void advance_eg_channel(FM_OPN *OPN, FM_SLOT *SLOT)
{
	//unsigned int out;
	unsigned int i = 4; /* four operators per channel */

	do
	{
		switch(SLOT->state)
		{
		case EG_ATT:    /* attack phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_ar)-1)))
			{
					/* update attenuation level */
					SLOT->volume += (~SLOT->volume * (eg_inc[SLOT->eg_sel_ar + ((OPN->eg_cnt>>SLOT->eg_sh_ar)&7)]))>>4;

					/* check phase transition*/
					if (SLOT->volume <= MIN_ATT_INDEX)
					{
						SLOT->volume = MIN_ATT_INDEX;
						SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC; /* special case where SL=0 */
					}

					/* recalculate EG output */
					if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))  /* SSG-EG Output Inversion */
					SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
					else
						SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
			}
			break;

		case EG_DEC:    /* decay phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_d1r)-1)))
			{
					/* SSG EG type */
					if (SLOT->ssg&0x08)
					{
						/* update attenuation level */
						if (SLOT->volume < 0x200)
					{
						SLOT->volume += 4 * eg_inc[SLOT->eg_sel_d1r + ((OPN->eg_cnt>>SLOT->eg_sh_d1r)&7)];

						/* recalculate EG output */
						if (SLOT->ssgn ^ (SLOT->ssg&0x04))   /* SSG-EG Output Inversion */
							SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
						else
							SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}

					}
					else
					{
					/* update attenuation level */
					SLOT->volume += eg_inc[SLOT->eg_sel_d1r + ((OPN->eg_cnt>>SLOT->eg_sh_d1r)&7)];

					/* recalculate EG output */
					SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}

					/* check phase transition*/
					if (SLOT->volume >= (INT32)(SLOT->sl))
						SLOT->state = EG_SUS;
			}
			break;

		case EG_SUS:    /* sustain phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_d2r)-1)))
			{
					/* SSG EG type */
					if (SLOT->ssg&0x08)
					{
					/* update attenuation level */
					if (SLOT->volume < 0x200)
					{
						SLOT->volume += 4 * eg_inc[SLOT->eg_sel_d2r + ((OPN->eg_cnt>>SLOT->eg_sh_d2r)&7)];

						/* recalculate EG output */
						if (SLOT->ssgn ^ (SLOT->ssg&0x04))   /* SSG-EG Output Inversion */
							SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
						else
							SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}
					}
					else
					{
						/* update attenuation level */
						SLOT->volume += eg_inc[SLOT->eg_sel_d2r + ((OPN->eg_cnt>>SLOT->eg_sh_d2r)&7)];

						/* check phase transition*/
						if ( SLOT->volume >= MAX_ATT_INDEX )
							SLOT->volume = MAX_ATT_INDEX;
						/* do not change SLOT->state (verified on real chip) */

						/* recalculate EG output */
						SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
					}
			}
			break;

		case EG_REL:    /* release phase */
			if (!(OPN->eg_cnt & ((1<<SLOT->eg_sh_rr)-1)))
			{
					/* SSG EG type */
					if (SLOT->ssg&0x08)
					{
						/* update attenuation level */
						if (SLOT->volume < 0x200)
							SLOT->volume += 4 * eg_inc[SLOT->eg_sel_rr + ((OPN->eg_cnt>>SLOT->eg_sh_rr)&7)];
					/* check phase transition */
					if (SLOT->volume >= 0x200)
					{
						SLOT->volume = MAX_ATT_INDEX;
						SLOT->state = EG_OFF;
					}
					}
					else
					{
						/* update attenuation level */
						SLOT->volume += eg_inc[SLOT->eg_sel_rr + ((OPN->eg_cnt>>SLOT->eg_sh_rr)&7)];

						/* check phase transition*/
						if (SLOT->volume >= MAX_ATT_INDEX)
						{
							SLOT->volume = MAX_ATT_INDEX;
							SLOT->state = EG_OFF;
						}
					}

					/* recalculate EG output */
					SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;

			}
			break;
		}


		SLOT++;
		i--;
	}while (i);

}

/* SSG-EG update process */
/* The behavior is based upon Nemesis tests on real hardware */
/* This is actually executed before each samples */
INLINE void update_ssg_eg_channel(FM_SLOT *SLOT)
{
	unsigned int i = 4; /* four operators per channel */

	do
	{
		/* detect SSG-EG transition */
		/* this is not required during release phase as the attenuation has been forced to MAX and output invert flag is not used */
		/* if an Attack Phase is programmed, inversion can occur on each sample */
		if ((SLOT->ssg & 0x08) && (SLOT->volume >= 0x200) && (SLOT->state > EG_REL))
		{
			if (SLOT->ssg & 0x01)  /* bit 0 = hold SSG-EG */
			{
				/* set inversion flag */
					if (SLOT->ssg & 0x02)
						SLOT->ssgn = 4;

				/* force attenuation level during decay phases */
				if ((SLOT->state != EG_ATT) && !(SLOT->ssgn ^ (SLOT->ssg & 0x04)))
					SLOT->volume  = MAX_ATT_INDEX;
			}
			else  /* loop SSG-EG */
			{
				/* toggle output inversion flag or reset Phase Generator */
					if (SLOT->ssg & 0x02)
						SLOT->ssgn ^= 4;
					else
						SLOT->phase = 0;

				/* same as Key ON */
				if (SLOT->state != EG_ATT)
				{
					if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
					{
						SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
					}
					else
					{
						/* Attack Rate is maximal: directly switch to Decay or Substain */
						SLOT->volume = MIN_ATT_INDEX;
						SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
					}
				}
			}

			/* recalculate EG output */
			if (SLOT->ssgn ^ (SLOT->ssg&0x04))
				SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
			else
				SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
		}

		/* next slot */
		SLOT++;
		i--;
	} while (i);
}


INLINE void update_phase_lfo_slot(FM_OPN *OPN, FM_SLOT *SLOT, INT32 pms, UINT32 block_fnum)
{
	UINT32 fnum_lfo   = ((block_fnum & 0x7f0) >> 4) * 32 * 8;
	INT32  lfo_fn_table_index_offset = lfo_pm_table[ fnum_lfo + pms + OPN->LFO_PM ];

	block_fnum = block_fnum*2 + lfo_fn_table_index_offset;

	if (lfo_fn_table_index_offset)    /* LFO phase modulation active */
	{
		UINT8 blk = (block_fnum&0x7000) >> 12;
		UINT32 fn  = block_fnum & 0xfff;

		/* recalculate keyscale code */
		/*int kc = (blk<<2) | opn_fktable[fn >> 7];*/
		/* This really stupid bug caused a read outside of the
		   array [size 0x10] and returned invalid values.
		   This caused an annoying vibrato for some notes.
		   (Note: seems to be a copy-and-paste from OPNWriteReg -> case 0xA0)
		    Why are MAME cores always SOO buggy ?! */
		/* Oh, and before I forget: it's correct in fm.c */
		int kc = (blk<<2) | opn_fktable[fn >> 8];
		/* Thanks to Blargg - his patch that helped me to find this bug */

		/* recalculate (frequency) phase increment counter */
		int fc = (OPN->fn_table[fn]>>(7-blk)) + SLOT->DT[kc];

		/* (frequency) phase overflow (credits to Nemesis) */
		if (fc < 0) fc += OPN->fn_max;

		/* update phase */
		SLOT->phase += (fc * SLOT->mul) >> 1;
	}
	else    /* LFO phase modulation  = zero */
	{
		SLOT->phase += SLOT->Incr;
	}
}

INLINE void update_phase_lfo_channel(FM_OPN *OPN, FM_CH *CH)
{
	UINT32 block_fnum = CH->block_fnum;

	UINT32 fnum_lfo  = ((block_fnum & 0x7f0) >> 4) * 32 * 8;
	INT32  lfo_fn_table_index_offset = lfo_pm_table[ fnum_lfo + CH->pms + OPN->LFO_PM ];

	block_fnum = block_fnum*2 + lfo_fn_table_index_offset;

	if (lfo_fn_table_index_offset)    /* LFO phase modulation active */
	{
		UINT8 blk = (block_fnum&0x7000) >> 12;
		UINT32 fn  = block_fnum & 0xfff;

		/* recalculate keyscale code */
		/*int kc = (blk<<2) | opn_fktable[fn >> 7];*/
		/* the same stupid bug as above */
		int kc = (blk<<2) | opn_fktable[fn >> 8];

		/* recalculate (frequency) phase increment counter */
		int fc = (OPN->fn_table[fn]>>(7-blk));

		/* (frequency) phase overflow (credits to Nemesis) */
		int finc = fc + CH->SLOT[SLOT1].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT1].phase += (finc*CH->SLOT[SLOT1].mul) >> 1;

		finc = fc + CH->SLOT[SLOT2].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT2].phase += (finc*CH->SLOT[SLOT2].mul) >> 1;

		finc = fc + CH->SLOT[SLOT3].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT3].phase += (finc*CH->SLOT[SLOT3].mul) >> 1;

		finc = fc + CH->SLOT[SLOT4].DT[kc];
		if (finc < 0) finc += OPN->fn_max;
		CH->SLOT[SLOT4].phase += (finc*CH->SLOT[SLOT4].mul) >> 1;
	}
	else    /* LFO phase modulation  = zero */
	{
			CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
			CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
			CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
			CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
	}
}

/* update phase increment and envelope generator */
INLINE void refresh_fc_eg_slot(FM_OPN *OPN, FM_SLOT *SLOT , int fc , int kc )
{
	int ksr = kc >> SLOT->KSR;

	fc += SLOT->DT[kc];

	/* detects frequency overflow (credits to Nemesis) */
	if (fc < 0) fc += OPN->fn_max;

	/* (frequency) phase increment counter */
	SLOT->Incr = (fc * SLOT->mul) >> 1;

	if( SLOT->ksr != ksr )
	{
		SLOT->ksr = ksr;

		/* calculate envelope generator rates */
		if ((SLOT->ar + SLOT->ksr) < 32+62)
		{
			SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar  + SLOT->ksr ];
			SLOT->eg_sel_ar = eg_rate_select[SLOT->ar  + SLOT->ksr ];
		}
		else
		{
			SLOT->eg_sh_ar  = 0;
			SLOT->eg_sel_ar = 18*RATE_STEPS; /* verified by Nemesis on real hardware (Attack phase is blocked) */
		}

		SLOT->eg_sh_d1r = eg_rate_shift [SLOT->d1r + SLOT->ksr];
		SLOT->eg_sh_d2r = eg_rate_shift [SLOT->d2r + SLOT->ksr];
		SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr  + SLOT->ksr];

		SLOT->eg_sel_d1r= eg_rate_select[SLOT->d1r + SLOT->ksr];
		SLOT->eg_sel_d2r= eg_rate_select[SLOT->d2r + SLOT->ksr];
		SLOT->eg_sel_rr = eg_rate_select[SLOT->rr  + SLOT->ksr];
	}
}

/* update phase increment counters */
INLINE void refresh_fc_eg_chan(FM_OPN *OPN, FM_CH *CH )
{
	if( CH->SLOT[SLOT1].Incr==-1)
	{
		int fc = CH->fc;
		int kc = CH->kcode;
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT1] , fc , kc );
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT2] , fc , kc );
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT3] , fc , kc );
		refresh_fc_eg_slot(OPN, &CH->SLOT[SLOT4] , fc , kc );
	}
}

#define volume_calc(OP) ((OP)->vol_out + (AM & (OP)->AMmask))

INLINE signed int op_calc(UINT32 phase, unsigned int env, signed int pm)
{
	UINT32 p;

	p = (env<<3) + sin_tab[ ( ((signed int)((phase & ~FREQ_MASK) + (pm<<15))) >> FREQ_SH ) & SIN_MASK ];

	if (p >= TL_TAB_LEN)
		return 0;
	return tl_tab[p];
}

INLINE signed int op_calc1(UINT32 phase, unsigned int env, signed int pm)
{
	UINT32 p;

	p = (env<<3) + sin_tab[ ( ((signed int)((phase & ~FREQ_MASK) + pm      )) >> FREQ_SH ) & SIN_MASK ];

	if (p >= TL_TAB_LEN)
		return 0;
	return tl_tab[p];
}

INLINE void chan_calc(FM_OPN *OPN, FM_CH *CH, int chnum)
{
	UINT32 AM = OPN->LFO_AM >> CH->ams;
	unsigned int eg_out;

	if (CH->Muted)
		return;

	OPN->m2 = OPN->c1 = OPN->c2 = OPN->mem = 0;

	*CH->mem_connect = CH->mem_value;  /* restore delayed sample (MEM) value to m2 or c2 */

	eg_out = volume_calc(&CH->SLOT[SLOT1]);
	{
		INT32 out = CH->op1_out[0] + CH->op1_out[1];
		CH->op1_out[0] = CH->op1_out[1];

		if( !CH->connect1 )
		{
			/* algorithm 5  */
			OPN->mem = OPN->c1 = OPN->c2 = CH->op1_out[0];
		}
		else
		{
			/* other algorithms */
			*CH->connect1 += CH->op1_out[0];
		}


		CH->op1_out[1] = 0;
		if( eg_out < ENV_QUIET )  /* SLOT 1 */
		{
			if (!CH->FB)
				out=0;

			CH->op1_out[1] = op_calc1(CH->SLOT[SLOT1].phase, eg_out, (out<<CH->FB) );
		}
	}

	eg_out = volume_calc(&CH->SLOT[SLOT3]);
	if( eg_out < ENV_QUIET )    /* SLOT 3 */
		*CH->connect3 += op_calc(CH->SLOT[SLOT3].phase, eg_out, OPN->m2);

	eg_out = volume_calc(&CH->SLOT[SLOT2]);
	if( eg_out < ENV_QUIET )    /* SLOT 2 */
		*CH->connect2 += op_calc(CH->SLOT[SLOT2].phase, eg_out, OPN->c1);

	eg_out = volume_calc(&CH->SLOT[SLOT4]);
	if( eg_out < ENV_QUIET )    /* SLOT 4 */
		*CH->connect4 += op_calc(CH->SLOT[SLOT4].phase, eg_out, OPN->c2);


	/* store current MEM */
	CH->mem_value = OPN->mem;

	/* update phase counters AFTER output calculations */
	if(CH->pms)
	{
		/* add support for 3 slot mode */
		if ((OPN->ST.mode & 0xC0) && (chnum == 2))
		{
			update_phase_lfo_slot(OPN, &CH->SLOT[SLOT1], CH->pms, OPN->SL3.block_fnum[1]);
			update_phase_lfo_slot(OPN, &CH->SLOT[SLOT2], CH->pms, OPN->SL3.block_fnum[2]);
			update_phase_lfo_slot(OPN, &CH->SLOT[SLOT3], CH->pms, OPN->SL3.block_fnum[0]);
			update_phase_lfo_slot(OPN, &CH->SLOT[SLOT4], CH->pms, CH->block_fnum);
		}
		else update_phase_lfo_channel(OPN, CH);
	}
	else  /* no LFO phase modulation */
	{
		CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
		CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
		CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
		CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
	}
}


/* CSM Key Controll */
INLINE void CSMKeyControll(FM_OPN *OPN, FM_CH *CH)
{
	/* all key ON (verified by Nemesis on real hardware) */
	FM_KEYON_CSM(OPN,CH,SLOT1);
	FM_KEYON_CSM(OPN,CH,SLOT2);
	FM_KEYON_CSM(OPN,CH,SLOT3);
	FM_KEYON_CSM(OPN,CH,SLOT4);
	OPN->SL3.key_csm = 1;
}

#if BUILD_OPN

/* write a OPN mode register 0x20-0x2f */
static void OPNWriteMode(FM_OPN *OPN, int r, int v)
{
	UINT8 c;
	FM_CH *CH;

	switch(r)
	{
	case 0x21:  /* Test */
		break;
	case 0x22:  /* LFO FREQ (YM2608/YM2610/YM2610B/YM2612) */
		if( !(OPN->type & TYPE_LFOPAN) )
			break;
		if (v&8) /* LFO enabled ? */
		{
			OPN->lfo_timer_overflow = lfo_samples_per_step[v&7] << LFO_SH;
		}
		else
		{
			/* hold LFO waveform in reset state */
			OPN->lfo_timer_overflow = 0;
			OPN->lfo_timer = 0;
			OPN->lfo_cnt   = 0;
			OPN->LFO_PM    = 0;
			OPN->LFO_AM    = 126;
		}
		break;
	case 0x24:  /* timer A High 8*/
		OPN->ST.TA = (OPN->ST.TA & 0x03)|(((int)v)<<2);
		break;
	case 0x25:  /* timer A Low 2*/
		OPN->ST.TA = (OPN->ST.TA & 0x3fc)|(v&3);
		break;
	case 0x26:  /* timer B */
		OPN->ST.TB = v;
		break;
	case 0x27:  /* mode, timer control */
		set_timers( OPN, &(OPN->ST),OPN->ST.param,v );
		break;
	case 0x28:  /* key on / off */
		c = v & 0x03;
		if( c == 3 ) break;
		if( (v&0x04) && (OPN->type & TYPE_6CH) ) c+=3;
		CH = OPN->P_CH;
		CH = &CH[c];
		if(v&0x10) FM_KEYON(OPN,CH,SLOT1); else FM_KEYOFF(OPN,CH,SLOT1);
		if(v&0x20) FM_KEYON(OPN,CH,SLOT2); else FM_KEYOFF(OPN,CH,SLOT2);
		if(v&0x40) FM_KEYON(OPN,CH,SLOT3); else FM_KEYOFF(OPN,CH,SLOT3);
		if(v&0x80) FM_KEYON(OPN,CH,SLOT4); else FM_KEYOFF(OPN,CH,SLOT4);
		break;
	}
}

/* write a OPN register (0x30-0xff) */
static void OPNWriteReg(FM_OPN *OPN, int r, int v)
{
	FM_CH *CH;
	FM_SLOT *SLOT;

	UINT8 c = OPN_CHAN(r);

	if (c == 3) return; /* 0xX3,0xX7,0xXB,0xXF */

	if (r >= 0x100) c+=3;

	CH = OPN->P_CH;
	CH = &CH[c];

	SLOT = &(CH->SLOT[OPN_SLOT(r)]);

	switch( r & 0xf0 )
	{
	case 0x30:  /* DET , MUL */
		set_det_mul(&OPN->ST,CH,SLOT,v);
		break;

	case 0x40:  /* TL */
		set_tl(CH,SLOT,v);
		break;

	case 0x50:  /* KS, AR */
		set_ar_ksr(OPN->type,CH,SLOT,v);
		break;

	case 0x60:  /* bit7 = AM ENABLE, DR */
		set_dr(OPN->type, SLOT,v);

		if(OPN->type & TYPE_LFOPAN) /* YM2608/2610/2610B/2612 */
		{
			SLOT->AMmask = (v&0x80) ? ~0 : 0;
		}
		break;

	case 0x70:  /*     SR */
		set_sr(OPN->type,SLOT,v);
		break;

	case 0x80:  /* SL, RR */
		set_sl_rr(OPN->type,SLOT,v);
		break;

	case 0x90:  /* SSG-EG */
		SLOT->ssg  =  v&0x0f;

		/* recalculate EG output */
		if (SLOT->state > EG_REL)
		{
			if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
				SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
			else
				SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
		}

		/* SSG-EG envelope shapes :

		E AtAlH
		1 0 0 0  \\\\

		1 0 0 1  \___

		1 0 1 0  \/\/
		          ___
		1 0 1 1  \

		1 1 0 0  ////
		          ___
		1 1 0 1  /

		1 1 1 0  /\/\

		1 1 1 1  /___


		E = SSG-EG enable


		The shapes are generated using Attack, Decay and Sustain phases.

		Each single character in the diagrams above represents this whole
		sequence:

		- when KEY-ON = 1, normal Attack phase is generated (*without* any
		  difference when compared to normal mode),

		- later, when envelope level reaches minimum level (max volume),
		  the EG switches to Decay phase (which works with bigger steps
		  when compared to normal mode - see below),

		- later when envelope level passes the SL level,
		  the EG swithes to Sustain phase (which works with bigger steps
		  when compared to normal mode - see below),

		- finally when envelope level reaches maximum level (min volume),
		  the EG switches to Attack phase again (depends on actual waveform).

		Important is that when switch to Attack phase occurs, the phase counter
		of that operator will be zeroed-out (as in normal KEY-ON) but not always.
		(I havent found the rule for that - perhaps only when the output level is low)

		The difference (when compared to normal Envelope Generator mode) is
		that the resolution in Decay and Sustain phases is 4 times lower;
		this results in only 256 steps instead of normal 1024.
		In other words:
		when SSG-EG is disabled, the step inside of the EG is one,
		when SSG-EG is enabled, the step is four (in Decay and Sustain phases).

		Times between the level changes are the same in both modes.


		Important:
		Decay 1 Level (so called SL) is compared to actual SSG-EG output, so
		it is the same in both SSG and no-SSG modes, with this exception:

		when the SSG-EG is enabled and is generating raising levels
		(when the EG output is inverted) the SL will be found at wrong level !!!
		For example, when SL=02:
		    0 -6 = -6dB in non-inverted EG output
		    96-6 = -90dB in inverted EG output
		Which means that EG compares its level to SL as usual, and that the
		output is simply inverted afterall.


		The Yamaha's manuals say that AR should be set to 0x1f (max speed).
		That is not necessary, but then EG will be generating Attack phase.

		*/


		break;

	case 0xa0:
		switch( OPN_SLOT(r) )
		{
		case 0:     /* 0xa0-0xa2 : FNUM1 */
			if (OPN->LegacyMode)
				OPN->ST.fn_h = CH->block_fnum >> 8;
			{
				UINT32 fn = (((UINT32)( (OPN->ST.fn_h)&7))<<8) + v;
				UINT8 blk = OPN->ST.fn_h>>3;
				/* keyscale code */
				CH->kcode = (blk<<2) | opn_fktable[fn >> 7];
				/* phase increment counter */
				CH->fc = OPN->fn_table[fn*2]>>(7-blk);

				/* store fnum in clear form for LFO PM calculations */
				CH->block_fnum = (blk<<11) | fn;

				CH->SLOT[SLOT1].Incr=-1;
			}
			break;
		case 1:     /* 0xa4-0xa6 : FNUM2,BLK */
			OPN->ST.fn_h = v&0x3f;
			if (OPN->LegacyMode)	// behave like Gens (workaround for stupid Kega Fusion init block)
				CH->block_fnum = (OPN->ST.fn_h << 8) | (CH->block_fnum & 0xFF);
			break;
		case 2:     /* 0xa8-0xaa : 3CH FNUM1 */
			if (OPN->LegacyMode)
				OPN->SL3.fn_h = OPN->SL3.block_fnum[c] >> 8;
			if(r < 0x100)
			{
				UINT32 fn = (((UINT32)(OPN->SL3.fn_h&7))<<8) + v;
				UINT8 blk = OPN->SL3.fn_h>>3;
				/* keyscale code */
				OPN->SL3.kcode[c]= (blk<<2) | opn_fktable[fn >> 7];
				/* phase increment counter */
				OPN->SL3.fc[c] = OPN->fn_table[fn*2]>>(7-blk);
				OPN->SL3.block_fnum[c] = (blk<<11) | fn;
				(OPN->P_CH)[2].SLOT[SLOT1].Incr=-1;
			}
			break;
		case 3:     /* 0xac-0xae : 3CH FNUM2,BLK */
			if(r < 0x100)
			{
				OPN->SL3.fn_h = v&0x3f;
				if (OPN->LegacyMode)
					OPN->SL3.block_fnum[c] = (OPN->SL3.fn_h << 8) | (OPN->SL3.block_fnum[c] & 0xFF);
			}
			break;
		}
		break;

	case 0xb0:
		switch( OPN_SLOT(r) )
		{
		case 0:     /* 0xb0-0xb2 : FB,ALGO */
			{
				int feedback = (v>>3)&7;
				CH->ALGO = v&7;
				CH->FB   = feedback ? feedback+6 : 0;
				setup_connection( OPN, CH, c );
			}
			break;
		case 1:     /* 0xb4-0xb6 : L , R , AMS , PMS (YM2612/YM2610B/YM2610/YM2608) */
			if( OPN->type & TYPE_LFOPAN)
			{
				/* b0-2 PMS */
				CH->pms = (v & 7) * 32; /* CH->pms = PM depth * 32 (index in lfo_pm_table) */

				/* b4-5 AMS */
				CH->ams = lfo_ams_depth_shift[(v>>4) & 0x03];

				/* PAN :  b7 = L, b6 = R */
				OPN->pan[ c*2   ] = (v & 0x80) ? ~0 : 0;
				OPN->pan[ c*2+1 ] = (v & 0x40) ? ~0 : 0;

			}
			break;
		}
		break;
	}
}

/* initialize time tables */
static void init_timetables(FM_OPN *OPN)
{
	int i,d;
	double rate;

#if 0
	logerror("FM.C: samplerate=%8i chip clock=%8i  freqbase=%f  \n",
				ST->rate, ST->clock, ST->freqbase );
#endif

	/* DeTune table */
	for (d = 0;d <= 3;d++)
	{
		for (i = 0;i <= 31;i++)
		{
			rate = ((double)dt_tab[d*32 + i]) * OPN->ST.freqbase * (1<<(FREQ_SH-10)); /* -10 because chip works with 10.10 fixed point, while we use 16.16 */
			OPN->ST.dt_tab[d][i]   = (INT32) rate;
			OPN->ST.dt_tab[d+4][i] = -OPN->ST.dt_tab[d][i];
#if 0
			logerror("FM.C: DT [%2i %2i] = %8x  \n", d, i, ST->dt_tab[d][i] );
#endif
		}
	}

	/* there are 2048 FNUMs that can be generated using FNUM/BLK registers
	but LFO works with one more bit of a precision so we really need 4096 elements */
	/* calculate fnumber -> increment counter table */
	for(i = 0; i < 4096; i++)
	{
		/* freq table for octave 7 */
		/* OPN phase increment counter = 20bit */
		/* the correct formula is : F-Number = (144 * fnote * 2^20 / M) / 2^(B-1) */
		/* where sample clock is  M/144 */
		/* this means the increment value for one clock sample is FNUM * 2^(B-1) = FNUM * 64 for octave 7 */
		/* we also need to handle the ratio between the chip frequency and the emulated frequency (can be 1.0)  */
		OPN->fn_table[i] = (UINT32)( (double)i * 32 * OPN->ST.freqbase * (1<<(FREQ_SH-10)) ); /* -10 because chip works with 10.10 fixed point, while we use 16.16 */
	}

	/* maximal frequency is required for Phase overflow calculation, register size is 17 bits (Nemesis) */
	OPN->fn_max = (UINT32)( (double)0x20000 * OPN->ST.freqbase * (1<<(FREQ_SH-10)) );
}

/* prescaler set (and make time tables) */
static void OPNSetPres(FM_OPN *OPN, int pres, int timer_prescaler, int SSGpres)
{
	if (OPN->smpRateNative && OPN->smpRateFunc != NULL)
	{
		OPN->ST.rate = OPN->ST.clock / pres;
		if (OPN->smpRateFunc != NULL)
			OPN->smpRateFunc(OPN->smpRateData, OPN->ST.rate);
	}

	/* frequency base */
	OPN->ST.freqbase = (OPN->ST.rate) ? ((double)OPN->ST.clock / OPN->ST.rate) / pres : 0;
	if (fabs(OPN->ST.freqbase - 1.0) < 0.00005)
		OPN->ST.freqbase = 1.0;

#if 0
	OPN->ST.rate = (double)OPN->ST.clock / pres;
	OPN->ST.freqbase = 1.0;
#endif

	/* EG is updated every 3 samples */
	OPN->eg_timer_add  = (UINT32)((1<<EG_SH) * OPN->ST.freqbase);
	OPN->eg_timer_overflow = ( 3 ) * (1<<EG_SH);

	/* LFO timer increment (every samples) */
	OPN->lfo_timer_add  = (UINT32)((1<<LFO_SH) * OPN->ST.freqbase);

	/* Timer base time */
	OPN->ST.timer_prescaler = timer_prescaler;

	/* SSG part  prescaler set */
	if( SSGpres ) OPN->ST.SSG_funcs.set_clock( OPN->ST.SSG_param, OPN->ST.clock * 2 / SSGpres );

	/* make time tables */
	init_timetables( OPN );
}


static void reset_channels( FM_ST *ST , FM_CH *CH , int num )
{
	int c,s;

	ST->mode   = 0; /* normal mode */

	for( c = 0 ; c < num ; c++ )
	{
		//memset(&CH[c], 0x00, sizeof(FM_CH));
		CH[c].mem_value = 0;
		CH[c].op1_out[0] = 0;
		CH[c].op1_out[1] = 0;
		CH[c].fc = 0;
		for(s = 0 ; s < 4 ; s++ )
		{
			//memset(&CH[c].SLOT[s], 0x00, sizeof(FM_SLOT));
			CH[c].SLOT[s].DT = ST->dt_tab[0];
			CH[c].SLOT[s].Incr = -1;
			CH[c].SLOT[s].key = 0;
			CH[c].SLOT[s].phase = 0;
			CH[c].SLOT[s].ssg = 0;
			CH[c].SLOT[s].ssgn = 0;
			CH[c].SLOT[s].state= EG_OFF;
			CH[c].SLOT[s].volume = MAX_ATT_INDEX;
			CH[c].SLOT[s].vol_out= MAX_ATT_INDEX;
		}
	}
}

/* initialize generic tables */
static void init_tables(void)
{
	signed int i,x;
	signed int n;
	double o,m;

	if (tablesInit)
		return;
	tablesInit = 1;

	/* build Linear Power Table */
	for (x=0; x<TL_RES_LEN; x++)
	{
		m = (1<<16) / pow(2, (x+1) * (ENV_STEP/4.0) / 8.0);
		m = floor(m);

		/* we never reach (1<<16) here due to the (x+1) */
		/* result fits within 16 bits at maximum */

		n = (int)m;     /* 16 bits here */
		n >>= 4;        /* 12 bits here */
		if (n&1)        /* round to nearest */
			n = (n>>1)+1;
		else
			n = n>>1;
						/* 11 bits here (rounded) */
		n <<= 2;        /* 13 bits here (as in real chip) */


		/* 14 bits (with sign bit) */
		tl_tab[ x*2 + 0 ] = n;
		tl_tab[ x*2 + 1 ] = -tl_tab[ x*2 + 0 ];

		/* one entry in the 'Power' table use the following format, xxxxxyyyyyyyys with:            */
		/*        s = sign bit                                                                      */
		/* yyyyyyyy = 8-bits decimal part (0-TL_RES_LEN)                                            */
		/* xxxxx    = 5-bits integer 'shift' value (0-31) but, since Power table output is 13 bits, */
		/*            any value above 13 (included) would be discarded.                             */
		for (i=1; i<13; i++)
		{
			tl_tab[ x*2+0 + i*2*TL_RES_LEN ] =  tl_tab[ x*2+0 ]>>i;
			tl_tab[ x*2+1 + i*2*TL_RES_LEN ] = -tl_tab[ x*2+0 + i*2*TL_RES_LEN ];
		}
#if 0
		logerror("tl %04i", x);
		for (i=0; i<13; i++)
			logerror(", [%02i] %4x", i*2, tl_tab[ x*2 /*+1*/ + i*2*TL_RES_LEN ]);
		logerror("\n");
#endif
	}
	/*logerror("FM.C: TL_TAB_LEN = %i elements (%i bytes)\n",TL_TAB_LEN, (int)sizeof(tl_tab));*/


	/* build Logarithmic Sinus table */
	for (i=0; i<SIN_LEN; i++)
	{
		/* non-standard sinus */
		m = sin( ((i*2)+1) * M_PI / SIN_LEN ); /* checked against the real chip */
		/* we never reach zero here due to ((i*2)+1) */

		if (m>0.0)
			o = 8*log(1.0/m)/log(2.0);  /* convert to 'decibels' */
		else
			o = 8*log(-1.0/m)/log(2.0); /* convert to 'decibels' */

		o = o / (ENV_STEP/4);

		n = (int)(2.0*o);
		if (n&1)            /* round to nearest */
			n = (n>>1)+1;
		else
			n = n>>1;

		/* 13-bits (8.5) value is formatted for above 'Power' table */
		sin_tab[ i ] = n*2 + (m>=0.0? 0: 1 );
		/*logerror("FM.C: sin [%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[i],tl_tab[sin_tab[i]]);*/
	}

	/*logerror("FM.C: ENV_QUIET= %08x\n",ENV_QUIET );*/


	/* build LFO PM modulation table */
	for(i = 0; i < 8; i++) /* 8 PM depths */
	{
		UINT8 fnum;
		for (fnum=0; fnum<128; fnum++) /* 7 bits meaningful of F-NUMBER */
		{
			UINT8 value;
			UINT8 step;
			UINT32 offset_depth = i;
			UINT32 offset_fnum_bit;
			UINT32 bit_tmp;

			for (step=0; step<8; step++)
			{
				value = 0;
				for (bit_tmp=0; bit_tmp<7; bit_tmp++) /* 7 bits */
				{
					if (fnum & (1<<bit_tmp)) /* only if bit "bit_tmp" is set */
					{
						offset_fnum_bit = bit_tmp * 8;
						value += lfo_pm_output[offset_fnum_bit + offset_depth][step];
					}
				}
				/* 32 steps for LFO PM (sinus) */
				lfo_pm_table[(fnum*32*8) + (i*32) + step   + 0] = value;
				lfo_pm_table[(fnum*32*8) + (i*32) +(step^7)+ 8] = value;
				lfo_pm_table[(fnum*32*8) + (i*32) + step   +16] = -value;
				lfo_pm_table[(fnum*32*8) + (i*32) +(step^7)+24] = -value;
			}
#if 0
			logerror("LFO depth=%1x FNUM=%04x (<<4=%4x): ", i, fnum, fnum<<4);
			for (step=0; step<16; step++) /* dump only positive part of waveforms */
				logerror("%02x ", lfo_pm_table[(fnum*32*8) + (i*32) + step] );
			logerror("\n");
#endif

		}
	}
}

#endif /* BUILD_OPN */



#if BUILD_OPN_PRESCALER
/*
  prescaler circuit (best guess to verified chip behaviour)

               +--------------+  +-sel2-+
               |              +--|in20  |
         +---+ |  +-sel1-+       |      |
M-CLK -+-|1/2|-+--|in10  | +---+ |   out|--INT_CLOCK
       | +---+    |   out|-|1/3|-|in21  |
       +----------|in11  | +---+ +------+
                  +------+

reg.2d : sel2 = in21 (select sel2)
reg.2e : sel1 = in11 (select sel1)
reg.2f : sel1 = in10 , sel2 = in20 (clear selector)
reset  : sel1 = in11 , sel2 = in21 (clear both)

*/
static void OPNPrescaler_w(FM_OPN *OPN , int addr, int pre_divider)
{
	static const int opn_pres[4] = { 2*12 , 2*12 , 6*12 , 3*12 };
	static const int ssg_pres[4] = { 1    ,    1 ,    4 ,    2 };
	int sel;

	switch(addr)
	{
	case 0:     /* when reset */
		OPN->ST.prescaler_sel = 2;
		break;
	case 1:     /* when postload */
		break;
	case 0x2d:  /* divider sel : select 1/1 for 1/3line    */
		OPN->ST.prescaler_sel |= 0x02;
		break;
	case 0x2e:  /* divider sel , select 1/3line for output */
		OPN->ST.prescaler_sel |= 0x01;
		break;
	case 0x2f:  /* divider sel , clear both selector to 1/2,1/2 */
		OPN->ST.prescaler_sel = 0;
		break;
	}
	sel = OPN->ST.prescaler_sel & 3;
	/* update prescaler */
	OPNSetPres( OPN,	opn_pres[sel]*pre_divider,
						opn_pres[sel]*pre_divider,
						ssg_pres[sel]*pre_divider );
}
#endif /* BUILD_OPN_PRESCALER */

static void ssgdummy_set_clock(void* param, UINT32 clock)
{
	(void)param;
	(void)clock;
	return;
}

static void ssgdummy_write(void* param, UINT8 address, UINT8 data)
{
	(void)param;
	(void)address;
	(void)data;
	return;
}

static UINT8 ssgdummy_read(void* param)
{
	(void)param;
	return 0x00;
}

static void ssgdummy_reset(void* param)
{
	(void)param;
	return;
}

static const ssg_callbacks ssg_dummy_funcs =
{
	ssgdummy_set_clock,
	ssgdummy_write,
	ssgdummy_read,
	ssgdummy_reset
};

static void OPNLinkSSG(FM_OPN *OPN, const ssg_callbacks *ssg_cb, void *ssg_param)
{
	if (ssg_cb == NULL)
	{
		OPN->ST.SSG_funcs = ssg_dummy_funcs;
		OPN->ST.SSG_param = NULL;
	}
	else
	{
		OPN->ST.SSG_funcs = *ssg_cb;
		OPN->ST.SSG_param = ssg_param;
	}
	return;
}

static void OPNCheckNativeSampleRate(FM_OPN *OPN)
{
	UINT32 pres;
	UINT32 nativeSRate;
	int sRateDiff;
	
	pres = 6*12;	// default prescaler
	if (OPN->type & TYPE_6CH)
		pres *= 2;
	nativeSRate = OPN->ST.clock / pres;
	sRateDiff = (int)OPN->ST.rate - (int)nativeSRate;
	OPN->smpRateNative = (abs(sRateDiff) <= 2);
	
	return;
}

static void OPNSetSmplRateChgCallback(FM_OPN *OPN, DEVCB_SRATE_CHG cbFunc, void* dataPtr)
{
	// set Sample Rate Change Callback routine
	OPN->smpRateFunc = cbFunc;
	OPN->smpRateData = dataPtr;
	return;
}


#if BUILD_YM2203
/*****************************************************************************/
/*      YM2203 local section                                                 */
/*****************************************************************************/

/* here's the virtual YM2203(OPN) */
typedef struct
{
	DEV_DATA _devData;

	UINT8 REGS[256];        /* registers         */
	FM_OPN OPN;             /* OPN state         */
	FM_CH CH[3];            /* channel state     */
} YM2203;

/* Generate samples for one of the YM2203s */
void ym2203_update_one(void *chip, UINT32 length, DEV_SMPL **buffer)
{
	YM2203 *F2203 = (YM2203 *)chip;
	FM_OPN *OPN =   &F2203->OPN;
	UINT32 i;
	DEV_SMPL  *bufL,*bufR;
	FM_CH   *cch[3];

	if (buffer != NULL)
	{
		bufL = buffer[0];
		bufR = buffer[1];
	}
	else
	{
		bufL = bufR = NULL;
	}
	cch[0]   = &F2203->CH[0];
	cch[1]   = &F2203->CH[1];
	cch[2]   = &F2203->CH[2];


	/* refresh PG and EG */
	refresh_fc_eg_chan( OPN, cch[0] );
	refresh_fc_eg_chan( OPN, cch[1] );
	if( (OPN->ST.mode & 0xc0) )
	{
		/* 3SLOT MODE */
		if( cch[2]->SLOT[SLOT1].Incr==-1)
		{
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT1] , OPN->SL3.fc[1] , OPN->SL3.kcode[1] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT2] , OPN->SL3.fc[2] , OPN->SL3.kcode[2] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT3] , OPN->SL3.fc[0] , OPN->SL3.kcode[0] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT4] , cch[2]->fc , cch[2]->kcode );
		}
	}
	else
		refresh_fc_eg_chan( OPN, cch[2] );
	if (! length)
	{
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
	}


	/* YM2203 doesn't have LFO so we must keep these globals at 0 level */
	OPN->LFO_AM = 0;
	OPN->LFO_PM = 0;

	/* buffering */
	for (i=0; i < length ; i++)
	{
		/* clear outputs */
		OPN->out_fm[0] = 0;
		OPN->out_fm[1] = 0;
		OPN->out_fm[2] = 0;

		/* update SSG-EG output */
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);

		/* calculate FM */
		chan_calc(OPN, cch[0], 0 );
		chan_calc(OPN, cch[1], 1 );
		chan_calc(OPN, cch[2], 2 );

		/* advance envelope generator */
		OPN->eg_timer += OPN->eg_timer_add;
		while (OPN->eg_timer >= OPN->eg_timer_overflow)
		{
			OPN->eg_timer -= OPN->eg_timer_overflow;
			OPN->eg_cnt++;

			advance_eg_channel(OPN, &cch[0]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[1]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[2]->SLOT[SLOT1]);
		}

		/* buffering */
		{
			DEV_SMPL lt;

			lt = OPN->out_fm[0] + OPN->out_fm[1] + OPN->out_fm[2];

			/* buffering */
			bufL[i] = lt;
			bufR[i] = lt;
		}

		/* CSM mode: if CSM Key ON has occured, CSM Key OFF need to be sent       */
		/* only if Timer A does not overflow again (i.e CSM Key ON not set again) */
		OPN->SL3.key_csm <<= 1;

		/* timer A control */
		INTERNAL_TIMER_A( &OPN->ST , cch[2] )

		/* CSM Mode Key ON still disabled */
		if (OPN->SL3.key_csm & 2)
		{
			/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
			FM_KEYOFF_CSM(cch[2],SLOT1);
			FM_KEYOFF_CSM(cch[2],SLOT2);
			FM_KEYOFF_CSM(cch[2],SLOT3);
			FM_KEYOFF_CSM(cch[2],SLOT4);
			OPN->SL3.key_csm = 0;
		}
	}

	/* timer B control */
	INTERNAL_TIMER_B(&OPN->ST,length)
}

static void ym2203_update_req(void *param)
{
	ym2203_update_one(param, 0, NULL);
}

/* ---------- reset one of chip ---------- */
void ym2203_reset_chip(void *chip)
{
	int i;
	YM2203 *F2203 = (YM2203 *)chip;
	FM_OPN *OPN = &F2203->OPN;

	/* Reset Prescaler */
	OPNPrescaler_w(OPN, 0 , 1 );
	/* reset SSG section */
	OPN->ST.SSG_funcs.reset(OPN->ST.SSG_param);
	/* status clear */
	FM_IRQMASK_SET(&OPN->ST,0x03);
	FM_BUSY_CLEAR(&OPN->ST);

	OPN->eg_timer = 0;
	OPN->eg_cnt   = 0;

	OPN->ST.TAC    = 0;
	OPN->ST.TBC    = 0;

	OPN->SL3.key_csm = 0;

	OPN->ST.status = 0;
	OPN->ST.mode = 0;
	OPN->ST.irq = 0;

	memset(F2203->REGS, 0x00, sizeof(UINT8) * 256);

	reset_channels( &OPN->ST , F2203->CH , 3 );
	/* reset Operator paramater */
	for(i = 0xb2 ; i >= 0x30 ; i-- ) OPNWriteReg(OPN,i,0);
	OPNWriteMode(OPN,0x27,0x30); /* mode 0 , timer reset */
	for(i = 0x26 ; i >= 0x20 ; i-- ) OPNWriteReg(OPN,i,0);
}

/* ----------  Initialize YM2203 emulator(s) ----------
   'num' is the number of virtual YM2203s to allocate
   'clock' is the chip clock in Hz
   'rate' is sampling rate
*/
void * ym2203_init(void *param, UINT32 clock, UINT32 rate,
				FM_TIMERHANDLER timer_handler,FM_IRQHANDLER IRQHandler)
{
	YM2203 *F2203;

	/* allocate ym2203 state space */
	F2203 = (YM2203 *)calloc(1,sizeof(YM2203));
	if( F2203==NULL)
		return NULL;

	/* allocate total level table (128kb space) */
	init_tables();

	F2203->OPN.ST.param = param;
	F2203->OPN.type = TYPE_YM2203;
	F2203->OPN.P_CH = F2203->CH;
	F2203->OPN.ST.clock = clock;
	F2203->OPN.ST.rate = rate;
	OPNCheckNativeSampleRate(&F2203->OPN);

	F2203->OPN.ST.timer_handler = timer_handler;
	F2203->OPN.ST.IRQ_Handler   = IRQHandler;
	OPNLinkSSG(&F2203->OPN, NULL, NULL);
	OPNSetSmplRateChgCallback(&F2203->OPN, NULL, NULL);

	ym2203_set_mutemask(F2203, 0x00);

	return F2203;
}

/* link SSG emulator */
void ym2203_link_ssg(void *chip, const ssg_callbacks *ssg, void *ssg_param)
{
	YM2203 *F2203 = (YM2203 *)chip;

	OPNLinkSSG(&F2203->OPN, ssg, ssg_param);
	OPNPrescaler_w(&F2203->OPN, 1, 1);
	return;
}

/* set sample rate change callback */
void ym2203_set_srchg_cb(void *chip, DEVCB_SRATE_CHG cbFunc, void* dataPtr)
{
	YM2203 *F2203 = (YM2203 *)chip;
	OPNSetSmplRateChgCallback(&F2203->OPN, cbFunc, dataPtr);
	return;
}

/* shut down emulator */
void ym2203_shutdown(void *chip)
{
	YM2203 *F2203 = (YM2203 *)chip;

	free(F2203);
}

/* YM2203 I/O interface */
void ym2203_write(void *chip,UINT8 a,UINT8 v)
{
	YM2203 *F2203 = (YM2203 *)chip;
	FM_OPN *OPN = &F2203->OPN;

	if (a>=2)return;
	if( !(a&1) )
	{   /* address port */
		OPN->ST.address = v;

		/* Write register to SSG emulator */
		if( v < 16 ) OPN->ST.SSG_funcs.write(OPN->ST.SSG_param,a,v);

		/* prescaler select : 2d,2e,2f  */
		if( v >= 0x2d && v <= 0x2f )
			OPNPrescaler_w(OPN , v , 1);
	}
	else
	{   /* data port */
		UINT8 addr = OPN->ST.address;
		F2203->REGS[addr] = v;
		switch( addr & 0xf0 )
		{
		case 0x00:  /* 0x00-0x0f : SSG section */
			/* Write data to SSG emulator */
			OPN->ST.SSG_funcs.write(OPN->ST.SSG_param,a,v);
			break;
		case 0x20:  /* 0x20-0x2f : Mode section */
			ym2203_update_req(F2203);
			/* write register */
			OPNWriteMode(OPN,addr,v);
			break;
		default:    /* 0x30-0xff : OPN section */
			ym2203_update_req(F2203);
			/* write register */
			OPNWriteReg(OPN,addr,v);
		}
		FM_BUSY_SET(&OPN->ST,1);
	}
	//return OPN->ST.irq;
	return;
}

UINT8 ym2203_read(void *chip,UINT8 a)
{
	YM2203 *F2203 = (YM2203 *)chip;
	UINT8 addr = F2203->OPN.ST.address;
	UINT8 ret = 0;

	if( !(a&1) )
	{   /* status port */
		ret = FM_STATUS_FLAG(&F2203->OPN.ST);
	}
	else
	{   /* data port (only SSG) */
		if( addr < 16 ) ret = F2203->OPN.ST.SSG_funcs.read(F2203->OPN.ST.SSG_param, 0);
	}
	return ret;
}

UINT8 ym2203_timer_over(void *chip,UINT8 c)
{
	YM2203 *F2203 = (YM2203 *)chip;

	if( c )
	{   /* Timer B */
		TimerBOver( &(F2203->OPN.ST) );
	}
	else
	{   /* Timer A */
		ym2203_update_req(F2203);
		/* timer update */
		TimerAOver( &(F2203->OPN.ST) );
		/* CSM mode key,TL controll */
		if ((F2203->OPN.ST.mode & 0xc0) == 0x80)
		{   /* CSM mode total level latch and auto key on */
			CSMKeyControll( &F2203->OPN, &(F2203->CH[2]) );
		}
	}
	return F2203->OPN.ST.irq;
}

void ym2203_set_mutemask(void *chip, UINT32 MuteMask)
{
	YM2203 *F2203 = (YM2203 *)chip;
	UINT8 CurChn;
	
	for (CurChn = 0; CurChn < 3; CurChn ++)
		F2203->CH[CurChn].Muted = (MuteMask >> CurChn) & 0x01;
	
	return;
}
#endif /* BUILD_YM2203 */



#if (BUILD_YM2608||BUILD_YM2610||BUILD_YM2610B)

/* ADPCM type A channel struct */
typedef struct
{
	UINT8       flag;           /* port state               */
	UINT8       flagMask;       /* arrived flag mask        */
	UINT8       now_data;       /* current ROM data         */
	UINT32      now_addr;       /* current ROM address      */
	UINT32      now_step;
	UINT32      step;
	UINT32      start;          /* sample data start address*/
	UINT32      end;            /* sample data end address  */
	UINT8       IL;             /* Instrument Level         */
	INT32       adpcm_acc;      /* accumulator              */
	INT32       adpcm_step;     /* step                     */
	INT32       adpcm_out;      /* (speedup) hiro-shi!!     */
	INT8        vol_mul;        /* volume in "0.75dB" steps */
	UINT8       vol_shift;      /* volume in "-6dB" steps   */
	INT32       *pan;           /* &out_adpcm[OPN_xxxx]     */
	UINT8       Muted;
} ADPCM_CH;

/* here's the virtual YM2610 */
typedef struct
{
//	DEV_DATA _devData;

	UINT8       REGS[512];          /* registers            */
	FM_OPN      OPN;                /* OPN state            */
	FM_CH       CH[6];              /* channel state        */
	UINT8       addr_A1;            /* address line A1      */

	/* ADPCM-A unit */
	UINT8       *pcmbuf;            /* pcm rom buffer       */
	UINT32      pcm_size;           /* size of pcm rom      */
	UINT8       adpcmTL;            /* adpcmA total level   */
	ADPCM_CH    adpcm[6];           /* adpcm channels       */
	UINT32      adpcmreg[0x30];     /* registers            */
	UINT8       adpcm_arrivedEndAddress;
	YM_DELTAT   deltaT;             /* Delta-T ADPCM unit   */
	UINT8       MuteDeltaT;

	UINT8       flagmask;           /* YM2608 only */
	UINT8       irqmask;            /* YM2608 only */
} YM2610;

/* here is the virtual YM2608 */
typedef YM2610 YM2608;


/**** YM2610 ADPCM defines ****/
#define ADPCM_SHIFT    (16)      /* frequency step rate   */
#define ADPCMA_ADDRESS_SHIFT 8   /* adpcm A address shift */

/* Algorithm and tables verified on real YM2608 and YM2610 */

/* usual ADPCM table (16 * 1.1^N) */
static const int steps[49] =
{
	 16,  17,   19,   21,   23,   25,   28,
	 31,  34,   37,   41,   45,   50,   55,
	 60,  66,   73,   80,   88,   97,  107,
	118, 130,  143,  157,  173,  190,  209,
	230, 253,  279,  307,  337,  371,  408,
	449, 494,  544,  598,  658,  724,  796,
	876, 963, 1060, 1166, 1282, 1411, 1552
};

/* different from the usual ADPCM table */
static const int step_inc[8] = { -1*16, -1*16, -1*16, -1*16, 2*16, 5*16, 7*16, 9*16 };

/* speedup purposes only */
static int jedi_table[ 49*16 ];


static void Init_ADPCMATable(void)
{
	int step, nib;

	for (step = 0; step < 49; step++)
	{
		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			int value = (2*(nib & 0x07) + 1) * steps[step] / 8;
			jedi_table[step*16 + nib] = (nib&0x08) ? -value : value;
		}
	}
}

/* ADPCM A (Non control type) : calculate one channel output */
INLINE void ADPCMA_calc_chan( YM2610 *F2610, ADPCM_CH *ch )
{
	UINT32 step;
	UINT8  data;

	if (ch->Muted)
		return;


	ch->now_step += ch->step;
	if ( ch->now_step >= (1<<ADPCM_SHIFT) )
	{
		step = ch->now_step >> ADPCM_SHIFT;
		ch->now_step &= (1<<ADPCM_SHIFT)-1;
		do{
			/* end check */
			/* 11-06-2001 JB: corrected comparison. Was > instead of == */
			/* YM2610 checks lower 20 bits only, the 4 MSB bits are sample bank */
			/* Here we use 1<<21 to compensate for nibble calculations */

			if (   (ch->now_addr & ((1<<21)-1)) == ((ch->end<<1) & ((1<<21)-1))	   )
			{
				ch->flag = 0;
				F2610->adpcm_arrivedEndAddress |= ch->flagMask;
				return;
			}
#if 0
			if ( ch->now_addr > (F2610->pcmsizeA<<1) )
			{
				LOG(LOG_WAR,("YM2610: Attempting to play past adpcm rom size!\n" ));
				return;
			}
#endif
			if ( ch->now_addr&1 )
				data = ch->now_data & 0x0f;
			else
			{
				ch->now_data = *(F2610->pcmbuf+(ch->now_addr>>1));
				data = (ch->now_data >> 4) & 0x0f;
			}

			ch->now_addr++;

			ch->adpcm_acc += jedi_table[ch->adpcm_step + data];

			/* the 12-bit accumulator wraps on the ym2610 and ym2608 (like the msm5205), it does not saturate (like the msm5218) */
			ch->adpcm_acc &= 0xfff;

			/* extend 12-bit signed int */
			if (ch->adpcm_acc & 0x800)
				ch->adpcm_acc |= ~0xfff;

			ch->adpcm_step += step_inc[data & 7];
			Limit( ch->adpcm_step, 48*16, 0*16 );

		}while(--step);

		/* calc pcm * volume data */
		ch->adpcm_out = ((ch->adpcm_acc * ch->vol_mul) >> ch->vol_shift) & ~3;	/* multiply, shift and mask out 2 LSB bits */
	}

	/* output for work of output channels (out_adpcm[OPNxxxx])*/
	*(ch->pan) += ch->adpcm_out;
}

/* ADPCM type A Write */
static void FM_ADPCMAWrite(YM2610 *F2610,int r,int v)
{
	ADPCM_CH *adpcm = F2610->adpcm;
	UINT8 c = r&0x07;

	F2610->adpcmreg[r] = v&0xff; /* stock data */
	switch( r )
	{
	case 0x00: /* DM,--,C5,C4,C3,C2,C1,C0 */
		if( !(v&0x80) )
		{
			/* KEY ON */
			for( c = 0; c < 6; c++ )
			{
				if( (v>>c)&1 )
				{
					/**** start adpcm ****/
					// The .step variable is already set and for the YM2608 it is different on channels 4 and 5.
					//adpcm[c].step      = (UINT32)((float)(1<<ADPCM_SHIFT)*((float)F2610->OPN.ST.freqbase)/3.0);
					adpcm[c].now_addr  = adpcm[c].start<<1;
					adpcm[c].now_step  = 0;
					adpcm[c].adpcm_acc = 0;
					adpcm[c].adpcm_step= 0;
					adpcm[c].adpcm_out = 0;
					adpcm[c].flag      = 1;

					if(F2610->pcmbuf==NULL)
					{                   /* Check ROM Mapped */
						//logerror("YM2608-YM2610: ADPCM-A rom not mapped\n");
						adpcm[c].flag = 0;
					}
					else
					{
						if(adpcm[c].end >= F2610->pcm_size)
						{   /* Check End in Range */
							//logerror("YM2610: ADPCM-A end out of range: $%08x\n",adpcm[c].end);
							/*adpcm[c].end = F2610->pcm_size-1;*/ /* JB: DO NOT uncomment this, otherwise you will break the comparison in the ADPCM_CALC_CHA() */
						}
						if(adpcm[c].start >= F2610->pcm_size)   /* Check Start in Range */
						{
							//logerror("YM2608-YM2610: ADPCM-A start out of range: $%08x\n",adpcm[c].start);
							adpcm[c].flag = 0;
						}
					}
				}
			}
		}
		else
		{
			/* KEY OFF */
			for( c = 0; c < 6; c++ )
				if( (v>>c)&1 )
					adpcm[c].flag = 0;
		}
		break;
	case 0x01:  /* B0-5 = TL */
		F2610->adpcmTL = (v & 0x3f) ^ 0x3f;
		for( c = 0; c < 6; c++ )
		{
			int volume = F2610->adpcmTL + adpcm[c].IL;

			if ( volume >= 63 ) /* This is correct, 63 = quiet */
			{
				adpcm[c].vol_mul   = 0;
				adpcm[c].vol_shift = 0;
			}
			else
			{
				adpcm[c].vol_mul   = 15 - (volume & 7);     /* so called 0.75 dB */
				adpcm[c].vol_shift =  1 + (volume >> 3);    /* Yamaha engineers used the approximation: each -6 dB is close to divide by two (shift right) */
			}

			/* calc pcm * volume data */
			adpcm[c].adpcm_out = ((adpcm[c].adpcm_acc * adpcm[c].vol_mul) >> adpcm[c].vol_shift) & ~3;  /* multiply, shift and mask out low 2 bits */
		}
		break;
	default:
		c = r&0x07;
		if( c >= 0x06 ) return;
		switch( r&0x38 )
		{
		case 0x08:  /* B7=L,B6=R, B4-0=IL */
		{
			int volume;

			adpcm[c].IL = (v & 0x1f) ^ 0x1f;

			volume = F2610->adpcmTL + adpcm[c].IL;

			if ( volume >= 63 ) /* This is correct, 63 = quiet */
			{
				adpcm[c].vol_mul   = 0;
				adpcm[c].vol_shift = 0;
			}
			else
			{
				adpcm[c].vol_mul   = 15 - (volume & 7);     /* so called 0.75 dB */
				adpcm[c].vol_shift =  1 + (volume >> 3);    /* Yamaha engineers used the approximation: each -6 dB is close to divide by two (shift right) */
			}

			adpcm[c].pan    = &F2610->OPN.out_adpcm[(v>>6)&0x03];

			/* calc pcm * volume data */
			adpcm[c].adpcm_out = ((adpcm[c].adpcm_acc * adpcm[c].vol_mul) >> adpcm[c].vol_shift) & ~3;  /* multiply, shift and mask out low 2 bits */
		}
			break;
		case 0x10:
		case 0x18:
			adpcm[c].start  = ( (F2610->adpcmreg[0x18 + c]*0x0100 | F2610->adpcmreg[0x10 + c]) << ADPCMA_ADDRESS_SHIFT);
			break;
		case 0x20:
		case 0x28:
			adpcm[c].end    = ( (F2610->adpcmreg[0x28 + c]*0x0100 | F2610->adpcmreg[0x20 + c]) << ADPCMA_ADDRESS_SHIFT);
			adpcm[c].end   += (1<<ADPCMA_ADDRESS_SHIFT) - 1;
			break;
		}
	}
}

#endif /* (BUILD_YM2608||BUILD_YM2610||BUILD_YM2610B) */


#if BUILD_YM2608
/*****************************************************************************/
/*      YM2608 local section                                                 */
/*****************************************************************************/



const unsigned int YM2608_ADPCM_ROM_addr[2*6] = {
0x0000, 0x01bf, /* bass drum  */
0x01c0, 0x043f, /* snare drum */
0x0440, 0x1b7f, /* top cymbal */
0x1b80, 0x1cff, /* high hat */
0x1d00, 0x1f7f, /* tom tom  */
0x1f80, 0x1fff  /* rim shot */
};


#include "fmopn_2608rom.h"



/* flag enable control 0x110 */
INLINE void YM2608IRQFlagWrite(FM_OPN *OPN, YM2608 *F2608, int v)
{
	if( v & 0x80 )
	{   /* Reset IRQ flag */
		FM_STATUS_RESET(&OPN->ST, 0xf7); /* don't touch BUFRDY flag otherwise we'd have to call ymdeltat module to set the flag back */
	}
	else
	{   /* Set status flag mask */
		F2608->flagmask = (~(v&0x1f));
		FM_IRQMASK_SET(&OPN->ST, (F2608->irqmask & F2608->flagmask) );
	}
}

/* compatible mode & IRQ enable control 0x29 */
INLINE void YM2608IRQMaskWrite(FM_OPN *OPN, YM2608 *F2608, int v)
{
	/* SCH,xx,xxx,EN_ZERO,EN_BRDY,EN_EOS,EN_TB,EN_TA */

	/* extend 3ch. enable/disable */
	if(v&0x80)
		OPN->type |= TYPE_6CH;  /* OPNA mode - 6 FM channels */
	else
		OPN->type &= ~TYPE_6CH; /* OPN mode - 3 FM channels */

	/* IRQ MASK store and set */
	F2608->irqmask = v&0x1f;
	FM_IRQMASK_SET(&OPN->ST, (F2608->irqmask & F2608->flagmask) );
}

/* Generate samples for one of the YM2608s */
void ym2608_update_one(void *chip, UINT32 length, stream_sample_t **buffer)
{
	YM2608 *F2608 = (YM2608 *)chip;
	FM_OPN *OPN   = &F2608->OPN;
	YM_DELTAT *DELTAT = &F2608->deltaT;
	UINT32 i;
	UINT8 j;
	stream_sample_t  *bufL,*bufR;
	FM_CH   *cch[6];
	INT32 *out_fm = OPN->out_fm;

	/* set bufer */
	if (buffer != NULL)
	{
		bufL = buffer[0];
		bufR = buffer[1];
	}
	else
	{
		bufL = bufR = NULL;
	}

	cch[0]   = &F2608->CH[0];
	cch[1]   = &F2608->CH[1];
	cch[2]   = &F2608->CH[2];
	cch[3]   = &F2608->CH[3];
	cch[4]   = &F2608->CH[4];
	cch[5]   = &F2608->CH[5];

	/* refresh PG and EG */
	refresh_fc_eg_chan( OPN, cch[0] );
	refresh_fc_eg_chan( OPN, cch[1] );
	if( (OPN->ST.mode & 0xc0) )
	{
		/* 3SLOT MODE */
		if( cch[2]->SLOT[SLOT1].Incr==-1)
		{
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT1] , OPN->SL3.fc[1] , OPN->SL3.kcode[1] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT2] , OPN->SL3.fc[2] , OPN->SL3.kcode[2] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT3] , OPN->SL3.fc[0] , OPN->SL3.kcode[0] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT4] , cch[2]->fc , cch[2]->kcode );
		}
	}
	else
		refresh_fc_eg_chan( OPN, cch[2] );
	refresh_fc_eg_chan( OPN, cch[3] );
	refresh_fc_eg_chan( OPN, cch[4] );
	refresh_fc_eg_chan( OPN, cch[5] );
	if (! length)
	{
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[4]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[5]->SLOT[SLOT1]);
	}


	/* buffering */
	for(i=0; i < length ; i++)
	{
		/* clear output acc. */
		OPN->out_adpcm[OUTD_LEFT] = OPN->out_adpcm[OUTD_RIGHT] = OPN->out_adpcm[OUTD_CENTER] = 0;
		OPN->out_delta[OUTD_LEFT] = OPN->out_delta[OUTD_RIGHT] = OPN->out_delta[OUTD_CENTER] = 0;
		/* clear outputs */
		out_fm[0] = 0;
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[3] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		/* update SSG-EG output */
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[4]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[5]->SLOT[SLOT1]);

		/* calculate FM */
		chan_calc(OPN, cch[0], 0 );
		chan_calc(OPN, cch[1], 1 );
		chan_calc(OPN, cch[2], 2 );
		chan_calc(OPN, cch[3], 3 );
		chan_calc(OPN, cch[4], 4 );
		chan_calc(OPN, cch[5], 5 );

		/* deltaT ADPCM */
		if( DELTAT->portstate&0x80 && ! F2608->MuteDeltaT )
			YM_DELTAT_ADPCM_CALC(DELTAT);

		/* ADPCMA */
		for( j = 0; j < 6; j++ )
		{
			if( F2608->adpcm[j].flag )
				ADPCMA_calc_chan( F2608, &F2608->adpcm[j]);
		}

		/* advance LFO */
		advance_lfo(OPN);

		/* advance envelope generator */
		OPN->eg_timer += OPN->eg_timer_add;
		while (OPN->eg_timer >= OPN->eg_timer_overflow)
		{
			OPN->eg_timer -= OPN->eg_timer_overflow;
			OPN->eg_cnt++;

			advance_eg_channel(OPN, &cch[0]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[1]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[2]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[3]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[4]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[5]->SLOT[SLOT1]);
		}

		/* buffering */
		{
			stream_sample_t lt,rt;

			lt =  (OPN->out_adpcm[OUTD_LEFT]  + OPN->out_adpcm[OUTD_CENTER])<<1;
			rt =  (OPN->out_adpcm[OUTD_RIGHT] + OPN->out_adpcm[OUTD_CENTER])<<1;
			lt += (OPN->out_delta[OUTD_LEFT]  + OPN->out_delta[OUTD_CENTER])>>8;
			rt += (OPN->out_delta[OUTD_RIGHT] + OPN->out_delta[OUTD_CENTER])>>8;
			lt += (out_fm[0] & OPN->pan[0]);
			rt += (out_fm[0] & OPN->pan[1]);
			lt += (out_fm[1] & OPN->pan[2]);
			rt += (out_fm[1] & OPN->pan[3]);
			lt += (out_fm[2] & OPN->pan[4]);
			rt += (out_fm[2] & OPN->pan[5]);
			lt += (out_fm[3] & OPN->pan[6]);
			rt += (out_fm[3] & OPN->pan[7]);
			lt += (out_fm[4] & OPN->pan[8]);
			rt += (out_fm[4] & OPN->pan[9]);
			lt += (out_fm[5] & OPN->pan[10]);
			rt += (out_fm[5] & OPN->pan[11]);

			// I like that emulation can have more precision than the real thing.
			//lt >>= 1; // shift right verified on real YM2608
			//rt >>= 1;

			/* buffering */
			bufL[i] = lt;
			bufR[i] = rt;
		}

		/* CSM mode: if CSM Key ON has occured, CSM Key OFF need to be sent       */
		/* only if Timer A does not overflow again (i.e CSM Key ON not set again) */
		OPN->SL3.key_csm <<= 1;

		/* timer A control */
		INTERNAL_TIMER_A( &OPN->ST , cch[2] )

		/* CSM Mode Key ON still disabled */
		if (OPN->SL3.key_csm & 2)
		{
			/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
			FM_KEYOFF_CSM(cch[2],SLOT1);
			FM_KEYOFF_CSM(cch[2],SLOT2);
			FM_KEYOFF_CSM(cch[2],SLOT3);
			FM_KEYOFF_CSM(cch[2],SLOT4);
			OPN->SL3.key_csm = 0;
		}
	}

	/* timer B control */
	INTERNAL_TIMER_B(&OPN->ST,length)

	/* check IRQ for DELTA-T EOS */
	FM_STATUS_SET(&OPN->ST, 0);
}

static void ym2608_update_req(void *param)
{
	ym2608_update_one(param, 0, NULL);
}

static void YM2608_deltat_status_set(void *chip, UINT8 changebits)
{
	YM2608 *F2608 = (YM2608 *)chip;
	FM_STATUS_SET(&(F2608->OPN.ST), changebits);
}
static void YM2608_deltat_status_reset(void *chip, UINT8 changebits)
{
	YM2608 *F2608 = (YM2608 *)chip;
	FM_STATUS_RESET(&(F2608->OPN.ST), changebits);
}
/* YM2608(OPNA) */
void * ym2608_init(void *param, UINT32 clock, UINT32 rate,
				FM_TIMERHANDLER timer_handler,FM_IRQHANDLER IRQHandler)
{
	YM2608 *F2608;

	/* allocate extend state space */
	F2608 = (YM2608 *)calloc(1,sizeof(YM2608));
	if( F2608==NULL)
		return NULL;

	/* allocate total level table (128kb space) */
	init_tables();

	F2608->OPN.ST.param = param;
	F2608->OPN.type = TYPE_YM2608;
	F2608->OPN.P_CH = F2608->CH;
	F2608->OPN.ST.clock = clock;
	F2608->OPN.ST.rate = rate;
	OPNCheckNativeSampleRate(&F2608->OPN);

	/* External handlers */
	F2608->OPN.ST.timer_handler = timer_handler;
	F2608->OPN.ST.IRQ_Handler   = IRQHandler;
	OPNLinkSSG(&F2608->OPN, NULL, NULL);
	OPNSetSmplRateChgCallback(&F2608->OPN, NULL, NULL);

	/* DELTA-T */
	F2608->deltaT.memory = NULL;
	F2608->deltaT.memory_size = 0x00;
	F2608->deltaT.memory_mask = 0x00;

	//F2608->deltaT.write_time = 20.0 / clock;  /* a single byte write takes 20 cycles of main clock */
	//F2608->deltaT.read_time  = 18.0 / clock;  /* a single byte read takes 18 cycles of main clock */

	F2608->deltaT.status_set_handler = YM2608_deltat_status_set;
	F2608->deltaT.status_reset_handler = YM2608_deltat_status_reset;
	F2608->deltaT.status_change_which_chip = F2608;
	F2608->deltaT.status_change_EOS_bit = 0x04; /* status flag: set bit2 on End Of Sample */
	F2608->deltaT.status_change_BRDY_bit = 0x08;    /* status flag: set bit3 on BRDY */
	F2608->deltaT.status_change_ZERO_bit = 0x10;    /* status flag: set bit4 if silence continues for more than 290 miliseconds while recording the ADPCM */

	YM_DELTAT_ADPCM_Init(&F2608->deltaT,YM_DELTAT_EMULATION_MODE_NORMAL,5,F2608->OPN.out_delta,1<<23);

	/* ADPCM Rhythm */
	F2608->pcmbuf   = (UINT8*)YM2608_ADPCM_ROM;
	F2608->pcm_size = 0x2000;

	Init_ADPCMATable();

	ym2608_set_mutemask(F2608, 0x00);

	return F2608;
}

/* link SSG emulator */
void ym2608_link_ssg(void *chip, const ssg_callbacks *ssg, void *ssg_param)
{
	YM2608 *F2608 = (YM2608 *)chip;

	OPNLinkSSG(&F2608->OPN, ssg, ssg_param);
	OPNPrescaler_w(&F2608->OPN, 1, 2);
	return;
}

/* set sample rate change callback */
/*void ym2608_set_srchg_cb(void *chip, DEVCB_SRATE_CHG cbFunc, void* dataPtr)
{
	YM2608 *F2608 = (YM2608 *)chip;
	OPNSetSmplRateChgCallback(&F2608->OPN, cbFunc, dataPtr);
	return;
}*/

/* shut down emulator */
void ym2608_shutdown(void *chip)
{
	YM2608 *F2608 = (YM2608 *)chip;

	free(F2608->deltaT.memory); F2608->deltaT.memory = NULL;

	free(F2608);
}

/* reset one of chips */
void ym2608_reset_chip(void *chip)
{
	int i;
	YM2608 *F2608 = (YM2608 *)chip;
	FM_OPN *OPN   = &F2608->OPN;
	YM_DELTAT *DELTAT = &F2608->deltaT;

	/* Reset Prescaler */
	OPNPrescaler_w(OPN , 0 , 2);
	F2608->deltaT.freqbase = OPN->ST.freqbase;
	/* reset SSG section */
	OPN->ST.SSG_funcs.reset(OPN->ST.SSG_param);

	/* status clear */
	FM_BUSY_CLEAR(&OPN->ST);

	/* register 0x29 - default value after reset is:
		enable only 3 FM channels and enable all the status flags */
	YM2608IRQMaskWrite(OPN, F2608, 0x1f );  /* default value for D4-D0 is 1 */

	/* register 0x10, A1=1 - default value is 1 for D4, D3, D2, 0 for the rest */
	YM2608IRQFlagWrite(OPN, F2608, 0x1c );  /* default: enable timer A and B, disable EOS, BRDY and ZERO */

	OPN->eg_timer = 0;
	OPN->eg_cnt   = 0;

	OPN->ST.TAC    = 0;
	OPN->ST.TBC    = 0;

	OPN->SL3.key_csm = 0;

	OPN->ST.status = 0;
	OPN->ST.mode = 0;
	OPN->ST.irq = 0;

	memset(F2608->REGS, 0x00, sizeof(UINT8) * 512);

	reset_channels( &OPN->ST , F2608->CH , 6 );
	/* reset Operator paramater */
	for(i = 0xb6 ; i >= 0xb4 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0xc0);
		OPNWriteReg(OPN,i|0x100,0xc0);
	}
	for(i = 0xb2 ; i >= 0x30 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0);
		OPNWriteReg(OPN,i|0x100,0);
	}
	OPNWriteMode(OPN,0x27,0x30); /* mode 0 , timer reset */
	for(i = 0x26 ; i >= 0x20 ; i-- ) OPNWriteReg(OPN,i,0);

	/* ADPCM - percussion sounds */
	for( i = 0; i < 6; i++ )
	{
		if (i<=3)   /* channels 0,1,2,3 */
			F2608->adpcm[i].step      = (UINT32)((float)(1<<ADPCM_SHIFT)*((float)F2608->OPN.ST.freqbase)/3.0);
		else        /* channels 4 and 5 work with slower clock */
			F2608->adpcm[i].step      = (UINT32)((float)(1<<ADPCM_SHIFT)*((float)F2608->OPN.ST.freqbase)/6.0);

		F2608->adpcm[i].start     = YM2608_ADPCM_ROM_addr[i*2];
		F2608->adpcm[i].end       = YM2608_ADPCM_ROM_addr[i*2+1];

		F2608->adpcm[i].now_addr  = 0;
		F2608->adpcm[i].now_step  = 0;
		// F2608->adpcm[i].delta     = 21866;
		F2608->adpcm[i].vol_mul   = 0;
		F2608->adpcm[i].pan       = &OPN->out_adpcm[OUTD_CENTER]; /* default center */
		F2608->adpcm[i].flagMask  = 0;
		F2608->adpcm[i].flag      = 0;
		F2608->adpcm[i].adpcm_acc = 0;
		F2608->adpcm[i].adpcm_step= 0;
		F2608->adpcm[i].adpcm_out = 0;
	}
	F2608->adpcmTL = 0x3f;

	F2608->adpcm_arrivedEndAddress = 0; /* not used */

	/* DELTA-T unit */
	DELTAT->freqbase = OPN->ST.freqbase;
	YM_DELTAT_ADPCM_Reset(DELTAT,OUTD_CENTER);
}

/* YM2608 write */
/* n = number  */
/* a = address */
/* v = value   */
void ym2608_write(void *chip, UINT8 a,UINT8 v)
{
	YM2608 *F2608 = (YM2608 *)chip;
	FM_OPN *OPN   = &F2608->OPN;
	int addr;

	//v &= 0xff;  /*adjust to 8 bit bus */


	switch(a&3)
	{
	case 0: /* address port 0 */
		OPN->ST.address = v;
		F2608->addr_A1 = 0;

		/* Write register to SSG emulator */
		if( v < 16 ) OPN->ST.SSG_funcs.write(OPN->ST.SSG_param,a,v);
		/* prescaler selecter : 2d,2e,2f  */
		if( v >= 0x2d && v <= 0x2f )
		{
			OPNPrescaler_w(OPN , v , 2);
			//TODO: set ADPCM[c].step
			F2608->deltaT.freqbase = OPN->ST.freqbase;
		}
		break;

	case 1: /* data port 0    */
		if (F2608->addr_A1 != 0)
			break;  /* verified on real YM2608 */

		addr = OPN->ST.address;
		F2608->REGS[addr] = v;
		switch(addr & 0xf0)
		{
		case 0x00:  /* SSG section */
			/* Write data to SSG emulator */
			OPN->ST.SSG_funcs.write(OPN->ST.SSG_param,a,v);
			break;
		case 0x10:  /* 0x10-0x1f : Rhythm section */
			ym2608_update_req(F2608);
			FM_ADPCMAWrite(F2608,addr-0x10,v);
			break;
		case 0x20:  /* Mode Register */
			switch(addr)
			{
			case 0x29:  /* SCH,xx,xxx,EN_ZERO,EN_BRDY,EN_EOS,EN_TB,EN_TA */
				YM2608IRQMaskWrite(OPN, F2608, v);
				break;
			default:
				ym2608_update_req(F2608);
				OPNWriteMode(OPN,addr,v);
			}
			break;
		default:    /* OPN section */
			ym2608_update_req(F2608);
			OPNWriteReg(OPN,addr,v);
		}
		break;

	case 2: /* address port 1 */
		OPN->ST.address = v;
		F2608->addr_A1 = 1;
		break;

	case 3: /* data port 1    */
		if (F2608->addr_A1 != 1)
			break;  /* verified on real YM2608 */

		addr = OPN->ST.address;
		F2608->REGS[addr | 0x100] = v;
		ym2608_update_req(F2608);
		switch( addr & 0xf0 )
		{
		case 0x00:  /* DELTAT PORT */
			switch( addr )
			{
			case 0x0e:  /* DAC data */
				//logerror("YM2608: write to DAC data (unimplemented) value=%02x\n",v);
				break;
			default:
				/* 0x00-0x0d */
				YM_DELTAT_ADPCM_Write(&F2608->deltaT,addr,v);
			}
			break;
		case 0x10:  /* IRQ Flag control */
			if( addr == 0x10 )
			{
				YM2608IRQFlagWrite(OPN, F2608, v);
			}
			break;
		default:
			OPNWriteReg(OPN,addr | 0x100,v);
		}
	}
	//return OPN->ST.irq;
	return;
}

UINT8 ym2608_read(void *chip,UINT8 a)
{
	YM2608 *F2608 = (YM2608 *)chip;
	UINT8 addr = F2608->OPN.ST.address;
	UINT8 ret = 0;

	switch( a&3 )
	{
	case 0: /* status 0 : YM2203 compatible */
		/* BUSY:x:x:x:x:x:FLAGB:FLAGA */
		ret = FM_STATUS_FLAG(&F2608->OPN.ST) & 0x83;
		break;

	case 1: /* status 0, ID  */
		if( addr < 16 ) ret = F2608->OPN.ST.SSG_funcs.read(F2608->OPN.ST.SSG_param);
		else if(addr == 0xff) ret = 0x01; /* ID code */
		break;

	case 2: /* status 1 : status 0 + ADPCM status */
		/* BUSY : x : PCMBUSY : ZERO : BRDY : EOS : FLAGB : FLAGA */
		ret = (FM_STATUS_FLAG(&F2608->OPN.ST) & (F2608->flagmask|0x80)) | ((F2608->deltaT.PCM_BSY & 1)<<5) ;
		break;

	case 3:
		if(addr == 0x08)
		{
			ret = YM_DELTAT_ADPCM_Read(&F2608->deltaT);
		}
		else if(addr == 0x0f)
		{
			//logerror("YM2608 A/D conversion is accessed but not implemented !\n");
			ret = 0x80; /* 2's complement PCM data - result from A/D conversion */
		}
		break;
	}
	return ret;
}

UINT8 ym2608_timer_over(void *chip,UINT8 c)
{
	YM2608 *F2608 = (YM2608 *)chip;

	switch(c)
	{
#if 0
	case 2:
		{   /* BUFRDY flag */
			YM_DELTAT_BRDY_callback( &F2608->deltaT );
		}
		break;
#endif
	case 1:
		{   /* Timer B */
			TimerBOver( &(F2608->OPN.ST) );
		}
		break;
	case 0:
		{   /* Timer A */
			ym2608_update_req(F2608);
			/* timer update */
			TimerAOver( &(F2608->OPN.ST) );
			/* CSM mode key,TL controll */
			if ((F2608->OPN.ST.mode & 0xc0) == 0x80)
			{   /* CSM mode total level latch and auto key on */
				CSMKeyControll( &F2608->OPN, &(F2608->CH[2]) );
			}
		}
		break;
	default:
		break;
	}

	return F2608->OPN.ST.irq;
}

void ym2608_alloc_pcmromb(void* chip, UINT32 memsize)
{
	YM2608* F2608 = (YM2608*)chip;
	
	if (F2608->deltaT.memory_size == memsize)
		return;
	F2608->deltaT.memory = (UINT8*)realloc(F2608->deltaT.memory, memsize);
	F2608->deltaT.memory_size = memsize;
	memset(F2608->deltaT.memory, 0xFF, memsize);
	YM_DELTAT_calc_mem_mask(&F2608->deltaT);
	
	return;
}

void ym2608_write_pcmromb(void* chip, UINT32 offset, UINT32 length, const UINT8* data)
{
	YM2608* F2608 = (YM2608*)chip;
	
	if (offset > F2608->deltaT.memory_size)
		return;
	if (offset + length > F2608->deltaT.memory_size)
		length = F2608->deltaT.memory_size - offset;
	
	memcpy(F2608->deltaT.memory + offset, data, length);
	
	return;
}

void ym2608_set_mutemask(void *chip, UINT32 MuteMask)
{
	YM2608 *F2608 = (YM2608 *)chip;
	UINT8 CurChn;
	
	for (CurChn = 0; CurChn < 6; CurChn ++)
		F2608->CH[CurChn].Muted = (MuteMask >> CurChn) & 0x01;
	for (CurChn = 0; CurChn < 6; CurChn ++)
		F2608->adpcm[CurChn].Muted = (MuteMask >> (CurChn + 6)) & 0x01;
	F2608->MuteDeltaT = (MuteMask >> 12) & 0x01;
	
	return;
}
#endif /* BUILD_YM2608 */



#if (BUILD_YM2610||BUILD_YM2610B)
/* YM2610(OPNB) */

/* Generate samples for one of the YM2610s */
void ym2610_update_one(void *chip, UINT32 length, DEV_SMPL **buffer)
{
	YM2610 *F2610 = (YM2610 *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	YM_DELTAT *DELTAT = &F2610->deltaT;
	UINT32 i;
	UINT8 j;
	DEV_SMPL  *bufL,*bufR;
	FM_CH   *cch[4];
	INT32 *out_fm = OPN->out_fm;

	/* buffer setup */
	if (buffer != NULL)
	{
		bufL = buffer[0];
		bufR = buffer[1];
	}
	else
	{
		bufL = bufR = NULL;
	}

	cch[0] = &F2610->CH[1];
	cch[1] = &F2610->CH[2];
	cch[2] = &F2610->CH[4];
	cch[3] = &F2610->CH[5];

#ifdef YM2610B_WARNING
#define FM_KEY_IS(SLOT) ((SLOT)->key)
#define FM_MSG_YM2610B "YM2610-%p.CH%d is playing,Check whether the type of the chip is YM2610B\n"
	/* Check YM2610B warning message */
	if( FM_KEY_IS(&F2610->CH[0].SLOT[3]) )
	{
		LOG(LOG_WAR,(FM_MSG_YM2610B,F2610->OPN.ST.param,0));
		FM_KEY_IS(&F2610->CH[0].SLOT[3]) = 0;
	}
	if( FM_KEY_IS(&F2610->CH[3].SLOT[3]) )
	{
		LOG(LOG_WAR,(FM_MSG_YM2610B,F2610->OPN.ST.param,3));
		FM_KEY_IS(&F2610->CH[3].SLOT[3]) = 0;
	}
#endif

	/* refresh PG and EG */
	refresh_fc_eg_chan( OPN, cch[0] );
	if( (OPN->ST.mode & 0xc0) )
	{
		/* 3SLOT MODE */
		if( cch[1]->SLOT[SLOT1].Incr==-1)
		{
			refresh_fc_eg_slot(OPN, &cch[1]->SLOT[SLOT1] , OPN->SL3.fc[1] , OPN->SL3.kcode[1] );
			refresh_fc_eg_slot(OPN, &cch[1]->SLOT[SLOT2] , OPN->SL3.fc[2] , OPN->SL3.kcode[2] );
			refresh_fc_eg_slot(OPN, &cch[1]->SLOT[SLOT3] , OPN->SL3.fc[0] , OPN->SL3.kcode[0] );
			refresh_fc_eg_slot(OPN, &cch[1]->SLOT[SLOT4] , cch[1]->fc , cch[1]->kcode );
		}
	}
	else
		refresh_fc_eg_chan( OPN, cch[1] );
	refresh_fc_eg_chan( OPN, cch[2] );
	refresh_fc_eg_chan( OPN, cch[3] );
	if (! length)
	{
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
	}

	/* buffering */
	for(i=0; i < length ; i++)
	{
		/* clear output acc. */
		OPN->out_adpcm[OUTD_LEFT] = OPN->out_adpcm[OUTD_RIGHT] = OPN->out_adpcm[OUTD_CENTER] = 0;
		OPN->out_delta[OUTD_LEFT] = OPN->out_delta[OUTD_RIGHT] = OPN->out_delta[OUTD_CENTER] = 0;
		/* clear outputs */
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		/* update SSG-EG output */
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);

		/* calculate FM */
		chan_calc(OPN, cch[0], 1 );	/*remapped to 1*/
		chan_calc(OPN, cch[1], 2 );	/*remapped to 2*/
		chan_calc(OPN, cch[2], 4 );	/*remapped to 4*/
		chan_calc(OPN, cch[3], 5 );	/*remapped to 5*/

		/* deltaT ADPCM */
		if( DELTAT->portstate&0x80 && ! F2610->MuteDeltaT )
			YM_DELTAT_ADPCM_CALC(DELTAT);

		/* ADPCMA */
		for( j = 0; j < 6; j++ )
		{
			if( F2610->adpcm[j].flag )
				ADPCMA_calc_chan( F2610, &F2610->adpcm[j]);
		}

		/* advance LFO */
		advance_lfo(OPN);

		/* advance envelope generator */
		OPN->eg_timer += OPN->eg_timer_add;
		while (OPN->eg_timer >= OPN->eg_timer_overflow)
		{
			OPN->eg_timer -= OPN->eg_timer_overflow;
			OPN->eg_cnt++;

			advance_eg_channel(OPN, &cch[0]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[1]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[2]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[3]->SLOT[SLOT1]);
		}

		/* buffering */
		{
			DEV_SMPL lt,rt;

			lt =  (OPN->out_adpcm[OUTD_LEFT]  + OPN->out_adpcm[OUTD_CENTER])<<1;
			rt =  (OPN->out_adpcm[OUTD_RIGHT] + OPN->out_adpcm[OUTD_CENTER])<<1;
			lt += (OPN->out_delta[OUTD_LEFT]  + OPN->out_delta[OUTD_CENTER])>>8;
			rt += (OPN->out_delta[OUTD_RIGHT] + OPN->out_delta[OUTD_CENTER])>>8;


			lt += (out_fm[1] & OPN->pan[2]);
			rt += (out_fm[1] & OPN->pan[3]);
			lt += (out_fm[2] & OPN->pan[4]);
			rt += (out_fm[2] & OPN->pan[5]);

			lt += (out_fm[4] & OPN->pan[8]);
			rt += (out_fm[4] & OPN->pan[9]);
			lt += (out_fm[5] & OPN->pan[10]);
			rt += (out_fm[5] & OPN->pan[11]);

			//lt >>= 1; // the shift right was verified on real chip
			//rt >>= 1;

			/* buffering */
			bufL[i] = lt;
			bufR[i] = rt;
		}

		/* CSM mode: if CSM Key ON has occured, CSM Key OFF need to be sent       */
		/* only if Timer A does not overflow again (i.e CSM Key ON not set again) */
		OPN->SL3.key_csm <<= 1;

		/* timer A control */
		INTERNAL_TIMER_A( &OPN->ST , cch[1] )

		/* CSM Mode Key ON still disabled */
		if (OPN->SL3.key_csm & 2)
		{
			/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
			FM_KEYOFF_CSM(cch[1],SLOT1);
			FM_KEYOFF_CSM(cch[1],SLOT2);
			FM_KEYOFF_CSM(cch[1],SLOT3);
			FM_KEYOFF_CSM(cch[1],SLOT4);
			OPN->SL3.key_csm = 0;
		}
	}

	/* timer B control */
	INTERNAL_TIMER_B(&OPN->ST,length)
}

static void ym2610_update_req(void *param)
{
	ym2610b_update_one(param, 0, NULL);
}

#if BUILD_YM2610B
/* Generate samples for one of the YM2610Bs */
void ym2610b_update_one(void *chip, UINT32 length, DEV_SMPL **buffer)
{
	YM2610 *F2610 = (YM2610 *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	YM_DELTAT *DELTAT = &F2610->deltaT;
	UINT32 i;
	UINT8 j;
	DEV_SMPL  *bufL,*bufR;
	FM_CH   *cch[6];
	INT32 *out_fm = OPN->out_fm;

	/* buffer setup */
	if (buffer != NULL)
	{
		bufL = buffer[0];
		bufR = buffer[1];
	}
	else
	{
		bufL = bufR = NULL;
	}

	cch[0] = &F2610->CH[0];
	cch[1] = &F2610->CH[1];
	cch[2] = &F2610->CH[2];
	cch[3] = &F2610->CH[3];
	cch[4] = &F2610->CH[4];
	cch[5] = &F2610->CH[5];

	/* refresh PG and EG */
	refresh_fc_eg_chan( OPN, cch[0] );
	refresh_fc_eg_chan( OPN, cch[1] );
	if( (OPN->ST.mode & 0xc0) )
	{
		/* 3SLOT MODE */
		if( cch[2]->SLOT[SLOT1].Incr==-1)
		{
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT1] , OPN->SL3.fc[1] , OPN->SL3.kcode[1] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT2] , OPN->SL3.fc[2] , OPN->SL3.kcode[2] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT3] , OPN->SL3.fc[0] , OPN->SL3.kcode[0] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT4] , cch[2]->fc , cch[2]->kcode );
		}
	}
	else
		refresh_fc_eg_chan( OPN, cch[2] );
	refresh_fc_eg_chan( OPN, cch[3] );
	refresh_fc_eg_chan( OPN, cch[4] );
	refresh_fc_eg_chan( OPN, cch[5] );
	if (! length)
	{
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[4]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[5]->SLOT[SLOT1]);
	}

	/* buffering */
	for(i=0; i < length ; i++)
	{
		/* clear output acc. */
		OPN->out_adpcm[OUTD_LEFT] = OPN->out_adpcm[OUTD_RIGHT] = OPN->out_adpcm[OUTD_CENTER] = 0;
		OPN->out_delta[OUTD_LEFT] = OPN->out_delta[OUTD_RIGHT] = OPN->out_delta[OUTD_CENTER] = 0;
		/* clear outputs */
		out_fm[0] = 0;
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[3] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		/* update SSG-EG output */
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[4]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[5]->SLOT[SLOT1]);

		/* calculate FM */
		chan_calc(OPN, cch[0], 0 );
		chan_calc(OPN, cch[1], 1 );
		chan_calc(OPN, cch[2], 2 );
		chan_calc(OPN, cch[3], 3 );
		chan_calc(OPN, cch[4], 4 );
		chan_calc(OPN, cch[5], 5 );

		/* deltaT ADPCM */
		if( DELTAT->portstate&0x80 && ! F2610->MuteDeltaT )
			YM_DELTAT_ADPCM_CALC(DELTAT);

		/* ADPCMA */
		for( j = 0; j < 6; j++ )
		{
			if( F2610->adpcm[j].flag )
				ADPCMA_calc_chan( F2610, &F2610->adpcm[j]);
		}

		/* advance LFO */
		advance_lfo(OPN);

		/* advance envelope generator */
		OPN->eg_timer += OPN->eg_timer_add;
		while (OPN->eg_timer >= OPN->eg_timer_overflow)
		{
			OPN->eg_timer -= OPN->eg_timer_overflow;
			OPN->eg_cnt++;

			advance_eg_channel(OPN, &cch[0]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[1]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[2]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[3]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[4]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[5]->SLOT[SLOT1]);
		}


		/* buffering */
		{
			DEV_SMPL lt,rt;

			lt =  (OPN->out_adpcm[OUTD_LEFT]  + OPN->out_adpcm[OUTD_CENTER])<<1;
			rt =  (OPN->out_adpcm[OUTD_RIGHT] + OPN->out_adpcm[OUTD_CENTER])<<1;
			lt += (OPN->out_delta[OUTD_LEFT]  + OPN->out_delta[OUTD_CENTER])>>8;
			rt += (OPN->out_delta[OUTD_RIGHT] + OPN->out_delta[OUTD_CENTER])>>8;

			lt += (out_fm[0] & OPN->pan[0]);
			rt += (out_fm[0] & OPN->pan[1]);
			lt += (out_fm[1] & OPN->pan[2]);
			rt += (out_fm[1] & OPN->pan[3]);
			lt += (out_fm[2] & OPN->pan[4]);
			rt += (out_fm[2] & OPN->pan[5]);
			lt += (out_fm[3] & OPN->pan[6]);
			rt += (out_fm[3] & OPN->pan[7]);
			lt += (out_fm[4] & OPN->pan[8]);
			rt += (out_fm[4] & OPN->pan[9]);
			lt += (out_fm[5] & OPN->pan[10]);
			rt += (out_fm[5] & OPN->pan[11]);

			//lt >>= 1; // the shift right is verified on YM2610
			//rt >>= 1;

			/* buffering */
			bufL[i] = lt;
			bufR[i] = rt;
		}

		/* CSM mode: if CSM Key ON has occured, CSM Key OFF need to be sent       */
		/* only if Timer A does not overflow again (i.e CSM Key ON not set again) */
		OPN->SL3.key_csm <<= 1;

		/* timer A control */
		INTERNAL_TIMER_A( &OPN->ST , cch[2] )

		/* CSM Mode Key ON still disabled */
		if (OPN->SL3.key_csm & 2)
		{
			/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
			FM_KEYOFF_CSM(cch[2],SLOT1);
			FM_KEYOFF_CSM(cch[2],SLOT2);
			FM_KEYOFF_CSM(cch[2],SLOT3);
			FM_KEYOFF_CSM(cch[2],SLOT4);
			OPN->SL3.key_csm = 0;
		}
	}

	/* timer B control */
	INTERNAL_TIMER_B(&OPN->ST,length)
}
#endif /* BUILD_YM2610B */


static void YM2610_deltat_status_set(void *chip, UINT8 changebits)
{
	YM2610 *F2610 = (YM2610 *)chip;
	F2610->adpcm_arrivedEndAddress |= changebits;
}
static void YM2610_deltat_status_reset(void *chip, UINT8 changebits)
{
	YM2610 *F2610 = (YM2610 *)chip;
	F2610->adpcm_arrivedEndAddress &= (~changebits);
}

void *ym2610_init(void *param, UINT32 clock, UINT32 rate,
				FM_TIMERHANDLER timer_handler,FM_IRQHANDLER IRQHandler)
{
	YM2610 *F2610;

	/* allocate extend state space */
	F2610 = (YM2610 *)calloc(1,sizeof(YM2610));
	if( F2610==NULL)
		return NULL;

	/* allocate total level table (128kb space) */
	init_tables();

	/* FM */
	F2610->OPN.ST.param = param;
	F2610->OPN.type = TYPE_YM2610;
	F2610->OPN.P_CH = F2610->CH;
	F2610->OPN.ST.clock = clock;
	F2610->OPN.ST.rate = rate;
	OPNCheckNativeSampleRate(&F2610->OPN);
	/* Extend handler */
	F2610->OPN.ST.timer_handler = timer_handler;
	F2610->OPN.ST.IRQ_Handler   = IRQHandler;
	OPNLinkSSG(&F2610->OPN, NULL, NULL);
	OPNSetSmplRateChgCallback(&F2610->OPN, NULL, NULL);
	/* ADPCM */
	F2610->pcmbuf   = NULL;
	F2610->pcm_size = 0x00;
	/* DELTA-T */
	F2610->deltaT.memory = NULL;
	F2610->deltaT.memory_size = 0x00;
	F2610->deltaT.memory_mask = 0x00;

	F2610->deltaT.status_set_handler = YM2610_deltat_status_set;
	F2610->deltaT.status_reset_handler = YM2610_deltat_status_reset;
	F2610->deltaT.status_change_which_chip = F2610;
	F2610->deltaT.status_change_EOS_bit = 0x80; /* status flag: set bit7 on End Of Sample */

	YM_DELTAT_ADPCM_Init(&F2610->deltaT,YM_DELTAT_EMULATION_MODE_YM2610,8,F2610->OPN.out_delta,1<<23);

	Init_ADPCMATable();

	ym2610_set_mutemask(F2610, 0x00);

	return F2610;
}

/* link SSG emulator */
void ym2610_link_ssg(void *chip, const ssg_callbacks *ssg, void *ssg_param)
{
	YM2610 *F2610 = (YM2610 *)chip;

	OPNLinkSSG(&F2610->OPN, ssg, ssg_param);
	OPNPrescaler_w(&F2610->OPN, 1, 2);
	return;
}

/* shut down emulator */
void ym2610_shutdown(void *chip)
{
	YM2610 *F2610 = (YM2610 *)chip;

	free(F2610->pcmbuf);        F2610->pcmbuf = NULL;
	free(F2610->deltaT.memory); F2610->deltaT.memory = NULL;

	free(F2610);
}

/* reset one of chip */
void ym2610_reset_chip(void *chip)
{
	int i;
	YM2610 *F2610 = (YM2610 *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	YM_DELTAT *DELTAT = &F2610->deltaT;

	/* Reset Prescaler */
	OPNPrescaler_w( OPN, 0, 2); /* OPN 1/6 , SSG 1/4 */
	/* reset SSG section */
	OPN->ST.SSG_funcs.reset(OPN->ST.SSG_param);
	/* status clear */
	FM_IRQMASK_SET(&OPN->ST,0x03);
	FM_BUSY_CLEAR(&OPN->ST);

	OPN->eg_timer = 0;
	OPN->eg_cnt   = 0;

	OPN->ST.TAC    = 0;
	OPN->ST.TBC    = 0;

	OPN->SL3.key_csm = 0;

	OPN->ST.status = 0;
	OPN->ST.mode = 0;
	OPN->ST.irq = 0;

	memset(F2610->REGS, 0x00, sizeof(UINT8) * 512);

	reset_channels( &OPN->ST , F2610->CH , 6 );
	/* reset Operator paramater */
	for(i = 0xb6 ; i >= 0xb4 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0xc0);
		OPNWriteReg(OPN,i|0x100,0xc0);
	}
	for(i = 0xb2 ; i >= 0x30 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0);
		OPNWriteReg(OPN,i|0x100,0);
	}
	OPNWriteMode(OPN,0x27,0x30); /* mode 0 , timer reset */
	for(i = 0x26 ; i >= 0x20 ; i-- ) OPNWriteReg(OPN,i,0);

	/**** ADPCM work initial ****/
	for( i = 0; i < 6 ; i++ )
	{
		F2610->adpcm[i].step      = (UINT32)((float)(1<<ADPCM_SHIFT)*((float)F2610->OPN.ST.freqbase)/3.0);
		F2610->adpcm[i].now_addr  = 0;
		F2610->adpcm[i].now_step  = 0;
		F2610->adpcm[i].start     = 0;
		F2610->adpcm[i].end       = 0;
		// F2610->adpcm[i].delta     = 21866;
		F2610->adpcm[i].vol_mul   = 0;
		F2610->adpcm[i].pan       = &OPN->out_adpcm[OUTD_CENTER]; /* default center */
		F2610->adpcm[i].flagMask  = 1<<i;
		F2610->adpcm[i].flag      = 0;
		F2610->adpcm[i].adpcm_acc = 0;
		F2610->adpcm[i].adpcm_step= 0;
		F2610->adpcm[i].adpcm_out = 0;
	}
	F2610->adpcmTL = 0x3f;

	F2610->adpcm_arrivedEndAddress = 0;

	/* DELTA-T unit */
	DELTAT->freqbase = OPN->ST.freqbase;
	YM_DELTAT_ADPCM_Reset(DELTAT,OUTD_CENTER);
}

/* YM2610 write */
/* n = number  */
/* a = address */
/* v = value   */
void ym2610_write(void *chip, UINT8 a, UINT8 v)
{
	YM2610 *F2610 = (YM2610 *)chip;
	FM_OPN *OPN   = &F2610->OPN;
	int addr;
	int ch;

	//v &= 0xff;  /* adjust to 8 bit bus */

	switch( a&3 )
	{
	case 0: /* address port 0 */
		OPN->ST.address = v;
		F2610->addr_A1 = 0;

		/* Write register to SSG emulator */
		if( v < 16 ) OPN->ST.SSG_funcs.write(OPN->ST.SSG_param,a,v);
		break;

	case 1: /* data port 0    */
		if (F2610->addr_A1 != 0)
			break;  /* verified on real YM2608 */

		addr = OPN->ST.address;
		F2610->REGS[addr] = v;
		switch(addr & 0xf0)
		{
		case 0x00:  /* SSG section */
			/* Write data to SSG emulator */
			OPN->ST.SSG_funcs.write(OPN->ST.SSG_param,a,v);
			break;
		case 0x10: /* DeltaT ADPCM */
			ym2610_update_req(F2610);

			switch(addr)
			{
			case 0x10:  /* control 1 */
			case 0x11:  /* control 2 */
			case 0x12:  /* start address L */
			case 0x13:  /* start address H */
			case 0x14:  /* stop address L */
			case 0x15:  /* stop address H */

			case 0x19:  /* delta-n L */
			case 0x1a:  /* delta-n H */
			case 0x1b:  /* volume */
				{
					YM_DELTAT_ADPCM_Write(&F2610->deltaT,addr-0x10,v);
				}
				break;

			case 0x1c: /*  FLAG CONTROL : Extend Status Clear/Mask */
				{
					UINT8 statusmask = ~v;
					/* set arrived flag mask */
					for(ch=0;ch<6;ch++)
						F2610->adpcm[ch].flagMask = statusmask&(1<<ch);

					F2610->deltaT.status_change_EOS_bit = statusmask & 0x80;    /* status flag: set bit7 on End Of Sample */

					/* clear arrived flag */
					F2610->adpcm_arrivedEndAddress &= statusmask;
				}
				break;

			default:
				logerror("YM2610: write to unknown deltat register %02x val=%02x\n",addr,v);
				break;
			}

			break;
		case 0x20:  /* Mode Register */
			ym2610_update_req(F2610);
			OPNWriteMode(OPN,addr,v);
			break;
		default:    /* OPN section */
			ym2610_update_req(F2610);
			/* write register */
			OPNWriteReg(OPN,addr,v);
		}
		break;

	case 2: /* address port 1 */
		OPN->ST.address = v;
		F2610->addr_A1 = 1;
		break;

	case 3: /* data port 1    */
		if (F2610->addr_A1 != 1)
			break;  /* verified on real YM2608 */

		ym2610_update_req(F2610);
		addr = OPN->ST.address;
		F2610->REGS[addr | 0x100] = v;
		if( addr < 0x30 )
			/* 100-12f : ADPCM A section */
			FM_ADPCMAWrite(F2610,addr,v);
		else
			OPNWriteReg(OPN,addr | 0x100,v);
	}
	//return OPN->ST.irq;
	return;
}

UINT8 ym2610_read(void *chip,UINT8 a)
{
	YM2610 *F2610 = (YM2610 *)chip;
	UINT8 addr = F2610->OPN.ST.address;
	UINT8 ret = 0;

	switch( a&3)
	{
	case 0: /* status 0 : YM2203 compatible */
		ret = FM_STATUS_FLAG(&F2610->OPN.ST) & 0x83;
		break;
	case 1: /* data 0 */
		if( addr < 16 ) ret = F2610->OPN.ST.SSG_funcs.read(F2610->OPN.ST.SSG_param, 0);
		else if( addr == 0xff ) ret = 0x01;
		break;
	case 2: /* status 1 : ADPCM status */
		/* ADPCM STATUS (arrived End Address) */
		/* B,--,A5,A4,A3,A2,A1,A0 */
		/* B     = ADPCM-B(DELTA-T) arrived end address */
		/* A0-A5 = ADPCM-A          arrived end address */
		ret = F2610->adpcm_arrivedEndAddress;
		break;
	case 3:
		ret = 0;
		break;
	}
	return ret;
}

UINT8 ym2610_timer_over(void *chip,UINT8 c)
{
	YM2610 *F2610 = (YM2610 *)chip;

	if( c )
	{   /* Timer B */
		TimerBOver( &(F2610->OPN.ST) );
	}
	else
	{   /* Timer A */
		ym2610_update_req(F2610);
		/* timer update */
		TimerAOver( &(F2610->OPN.ST) );
		/* CSM mode key,TL controll */
		if ((F2610->OPN.ST.mode & 0xc0) == 0x80)
		{   /* CSM mode total level latch and auto key on */
			CSMKeyControll( &F2610->OPN, &(F2610->CH[2]) );
		}
	}
	return F2610->OPN.ST.irq;
}

void ym2610_alloc_pcmroma(void* chip, UINT32 memsize)
{
	YM2610 *F2610 = (YM2610 *)chip;
	
	if (F2610->pcm_size == memsize)
		return;
	F2610->pcmbuf = (UINT8*)realloc(F2610->pcmbuf, memsize);
	F2610->pcm_size = memsize;
	memset(F2610->pcmbuf, 0xFF, memsize);
	
	return;
}

void ym2610_write_pcmroma(void* chip, UINT32 offset, UINT32 length, const UINT8* data)
{
	YM2610 *F2610 = (YM2610 *)chip;
	
	if (offset > F2610->pcm_size)
		return;
	if (offset + length > F2610->pcm_size)
		length = F2610->pcm_size - offset;
	
	memcpy(F2610->pcmbuf + offset, data, length);
	
	return;
}

void ym2610_alloc_pcmromb(void* chip, UINT32 memsize)
{
	YM2610 *F2610 = (YM2610 *)chip;
	
	if (F2610->deltaT.memory_size == memsize)
		return;
	F2610->deltaT.memory = (UINT8*)realloc(F2610->deltaT.memory, memsize);
	F2610->deltaT.memory_size = memsize;
	memset(F2610->deltaT.memory, 0xFF, memsize);
	YM_DELTAT_calc_mem_mask(&F2610->deltaT);
	
	return;
}

void ym2610_write_pcmromb(void* chip, UINT32 offset, UINT32 length, const UINT8* data)
{
	YM2610 *F2610 = (YM2610 *)chip;
	
	if (offset > F2610->deltaT.memory_size)
		return;
	if (offset + length > F2610->deltaT.memory_size)
		length = F2610->deltaT.memory_size - offset;
	
	memcpy(F2610->deltaT.memory + offset, data, length);
	
	return;
}

void ym2610_set_mutemask(void *chip, UINT32 MuteMask)
{
	YM2610 *F2610 = (YM2610 *)chip;
	UINT8 CurChn;
	
	for (CurChn = 0; CurChn < 6; CurChn ++)
		F2610->CH[CurChn].Muted = (MuteMask >> CurChn) & 0x01;
	for (CurChn = 0; CurChn < 6; CurChn ++)
		F2610->adpcm[CurChn].Muted = (MuteMask >> (CurChn + 6)) & 0x01;
	F2610->MuteDeltaT = (MuteMask >> 12) & 0x01;
	
	return;
}
#endif /* (BUILD_YM2610||BUILD_YM2610B) */


#if (BUILD_YM2612)
/*******************************************************************************/
/*      YM2612 local section                                                   */
/*******************************************************************************/

/* here's the virtual YM2612 */
typedef struct
{
	DEV_DATA _devData;

	UINT8       REGS[512];          /* registers            */
	FM_OPN      OPN;                /* OPN state            */
	FM_CH       CH[6];              /* channel state        */
	UINT8       addr_A1;            /* address line A1      */

	/* dac output (YM2612) */
	UINT8       dacen;
	UINT8       dac_test;
	INT32       dacout;
	UINT8       MuteDAC;
	
	UINT8       WaveOutMode;
	INT32       WaveL;
	INT32       WaveR;
} YM2612;

/* Generate samples for one of the YM2612s */
void ym2612_update_one(void *chip, UINT32 length, DEV_SMPL **buffer)
{
	YM2612 *F2612 = (YM2612 *)chip;
	FM_OPN *OPN  = &F2612->OPN;
	INT32 *out_fm = OPN->out_fm;
	UINT32 i;
	DEV_SMPL  *bufL,*bufR;
	INT32 dacout;
	FM_CH   *cch[6];
	INT32 lt,rt;

	/* set buffer */
	if (buffer != NULL)
	{
		bufL = buffer[0];
		bufR = buffer[1];
	}
	else
	{
		// for internal 0-sample update
		bufL = bufR = NULL;
	}

	cch[0]   = &F2612->CH[0];
	cch[1]   = &F2612->CH[1];
	cch[2]   = &F2612->CH[2];
	cch[3]   = &F2612->CH[3];
	cch[4]   = &F2612->CH[4];
	cch[5]   = &F2612->CH[5];
	
	if (! F2612->MuteDAC)
		dacout = F2612->dacout << 5;    /* level unknown */
	else
		dacout = 0;

	/* refresh PG and EG */
	refresh_fc_eg_chan( OPN, cch[0] );
	refresh_fc_eg_chan( OPN, cch[1] );
	if( (OPN->ST.mode & 0xc0) )
	{
		/* 3SLOT MODE */
		if( cch[2]->SLOT[SLOT1].Incr==-1)
		{
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT1] , OPN->SL3.fc[1] , OPN->SL3.kcode[1] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT2] , OPN->SL3.fc[2] , OPN->SL3.kcode[2] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT3] , OPN->SL3.fc[0] , OPN->SL3.kcode[0] );
			refresh_fc_eg_slot(OPN, &cch[2]->SLOT[SLOT4] , cch[2]->fc , cch[2]->kcode );
		}
	}
	else
		refresh_fc_eg_chan( OPN, cch[2] );
	refresh_fc_eg_chan( OPN, cch[3] );
	refresh_fc_eg_chan( OPN, cch[4] );
	refresh_fc_eg_chan( OPN, cch[5] );
	if (! length)
	{
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[4]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[5]->SLOT[SLOT1]);
	}


	/* buffering */
	for(i=0; i < length ; i++)
	{
		/* clear outputs */
		out_fm[0] = 0;
		out_fm[1] = 0;
		out_fm[2] = 0;
		out_fm[3] = 0;
		out_fm[4] = 0;
		out_fm[5] = 0;

		/* update SSG-EG output */
		update_ssg_eg_channel(&cch[0]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[1]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[2]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[3]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[4]->SLOT[SLOT1]);
		update_ssg_eg_channel(&cch[5]->SLOT[SLOT1]);

		/* calculate FM */
		if (! F2612->dac_test)
		{
			chan_calc(OPN, cch[0], 0 );
			chan_calc(OPN, cch[1], 1 );
			chan_calc(OPN, cch[2], 2 );
			chan_calc(OPN, cch[3], 3 );
			chan_calc(OPN, cch[4], 4 );
			if( F2612->dacen )
				*cch[5]->connect4 += dacout;
			else
				chan_calc(OPN, cch[5], 5 );
		}
		else
		{
			out_fm[0] = out_fm[1] = dacout;
			out_fm[2] = out_fm[3] = dacout;
			out_fm[5] = dacout;
		}

		/* advance LFO */
		advance_lfo(OPN);

		/* advance envelope generator */
		OPN->eg_timer += OPN->eg_timer_add;
		while (OPN->eg_timer >= OPN->eg_timer_overflow)
		{
			OPN->eg_timer -= OPN->eg_timer_overflow;
			OPN->eg_cnt++;

			advance_eg_channel(OPN, &cch[0]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[1]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[2]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[3]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[4]->SLOT[SLOT1]);
			advance_eg_channel(OPN, &cch[5]->SLOT[SLOT1]);
		}

		if (out_fm[0] > 8192) out_fm[0] = 8192;
		else if (out_fm[0] < -8192) out_fm[0] = -8192;
		if (out_fm[1] > 8192) out_fm[1] = 8192;
		else if (out_fm[1] < -8192) out_fm[1] = -8192;
		if (out_fm[2] > 8192) out_fm[2] = 8192;
		else if (out_fm[2] < -8192) out_fm[2] = -8192;
		if (out_fm[3] > 8192) out_fm[3] = 8192;
		else if (out_fm[3] < -8192) out_fm[3] = -8192;
		if (out_fm[4] > 8192) out_fm[4] = 8192;
		else if (out_fm[4] < -8192) out_fm[4] = -8192;
		if (out_fm[5] > 8192) out_fm[5] = 8192;
		else if (out_fm[5] < -8192) out_fm[5] = -8192;

		/* 6-channels mixing  */
		lt  = ((out_fm[0]>>0) & OPN->pan[0]);
		rt  = ((out_fm[0]>>0) & OPN->pan[1]);
		lt += ((out_fm[1]>>0) & OPN->pan[2]);
		rt += ((out_fm[1]>>0) & OPN->pan[3]);
		lt += ((out_fm[2]>>0) & OPN->pan[4]);
		rt += ((out_fm[2]>>0) & OPN->pan[5]);
		lt += ((out_fm[3]>>0) & OPN->pan[6]);
		rt += ((out_fm[3]>>0) & OPN->pan[7]);
		if (! F2612->dac_test)
		{
			lt += ((out_fm[4]>>0) & OPN->pan[8]);
			rt += ((out_fm[4]>>0) & OPN->pan[9]);
		}
		else
		{
			// DAC test mode ignores panning for channel 4
			lt += dacout;
			lt += dacout;
		}
		lt += ((out_fm[5]>>0) & OPN->pan[10]);
		rt += ((out_fm[5]>>0) & OPN->pan[11]);

		/* buffering */
		if (F2612->WaveOutMode)
		{
			if (F2612->WaveOutMode & 0x01)
				F2612->WaveL = lt;
			if (F2612->WaveOutMode & 0x02)
				F2612->WaveR = rt;
			F2612->WaveOutMode ^= 0x03;
		}
		else
		{
			F2612->WaveL = lt;
			F2612->WaveR = rt;
		}
		bufL[i] = F2612->WaveL;
		bufR[i] = F2612->WaveR;

		/* CSM mode: if CSM Key ON has occured, CSM Key OFF need to be sent       */
		/* only if Timer A does not overflow again (i.e CSM Key ON not set again) */
		OPN->SL3.key_csm <<= 1;

		/* timer A control */
		INTERNAL_TIMER_A( &OPN->ST , cch[2] )

		/* CSM Mode Key ON still disabled */
		if (OPN->SL3.key_csm & 2)
		{
			/* CSM Mode Key OFF (verified by Nemesis on real hardware) */
			FM_KEYOFF_CSM(cch[2],SLOT1);
			FM_KEYOFF_CSM(cch[2],SLOT2);
			FM_KEYOFF_CSM(cch[2],SLOT3);
			FM_KEYOFF_CSM(cch[2],SLOT4);
			OPN->SL3.key_csm = 0;
		}
	}

	/* timer B control */
	INTERNAL_TIMER_B(&OPN->ST,length)
}

static void ym2612_update_req(void *param)
{
	ym2612_update_one(param, 0, NULL);
}

/* initialize YM2612 emulator(s) */
void * ym2612_init(void *param, UINT32 clock, UINT32 rate,
				FM_TIMERHANDLER timer_handler,FM_IRQHANDLER IRQHandler)
{
	YM2612 *F2612;

	/* allocate extend state space */
	F2612 = (YM2612 *)calloc(1, sizeof(YM2612));
	if (F2612 == NULL)
		return NULL;

	/* allocate total level table (128kb space) */
	init_tables();

	/* FM */
	F2612->OPN.ST.param = param;
	F2612->OPN.type = TYPE_YM2612;
	F2612->OPN.P_CH = F2612->CH;
	F2612->OPN.ST.clock = clock;
	F2612->OPN.ST.rate = rate;
	OPNCheckNativeSampleRate(&F2612->OPN);
	/* Extend handler */
	F2612->OPN.ST.timer_handler = timer_handler;
	F2612->OPN.ST.IRQ_Handler   = IRQHandler;
	F2612->OPN.LegacyMode = 0x00;
	F2612->WaveOutMode = 0x00;
	OPNLinkSSG(&F2612->OPN, NULL, NULL);
	OPNSetSmplRateChgCallback(&F2612->OPN, NULL, NULL);

	ym2612_set_mutemask(F2612, 0x00);

	return F2612;
}

/* shut down emulator */
void ym2612_shutdown(void *chip)
{
	YM2612 *F2612 = (YM2612 *)chip;

	free(F2612);
}

/* reset one of chip */
void ym2612_reset_chip(void *chip)
{
	int i;
	YM2612 *F2612 = (YM2612 *)chip;
	FM_OPN *OPN  = &F2612->OPN;

	/* Reset Prescaler */
	OPNSetPres( OPN, 6*24, 6*24, 0);
	/* status clear */
	FM_IRQMASK_SET(&OPN->ST,0x03);
	FM_BUSY_CLEAR(&OPN->ST);

	OPN->eg_timer = 0;
	OPN->eg_cnt   = 0;

	OPN->ST.TAC    = 0;
	OPN->ST.TBC    = 0;

	OPN->SL3.key_csm = 0;

	OPN->ST.status = 0;
	OPN->ST.mode = 0;
	OPN->ST.irq = 0;

	memset(F2612->REGS, 0x00, sizeof(UINT8) * 512);

	reset_channels( &OPN->ST , F2612->CH , 6 );
	/* reset Operator paramater */
	for(i = 0xb6 ; i >= 0xb4 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0xc0);
		OPNWriteReg(OPN,i|0x100,0xc0);
	}
	for(i = 0xb2 ; i >= 0x30 ; i-- )
	{
		OPNWriteReg(OPN,i      ,0);
		OPNWriteReg(OPN,i|0x100,0);
	}
	OPNWriteMode(OPN,0x27,0x30); /* mode 0 , timer reset */
	for(i = 0x26 ; i >= 0x20 ; i-- ) OPNWriteReg(OPN,i,0);

	/* DAC mode clear */
	F2612->dacen = 0;
	F2612->dac_test = 0;
	F2612->dacout = 0;
	
	F2612->WaveOutMode >>= 1;
	F2612->WaveL = F2612->WaveR = 0;
}

/* YM2612 write */
/* n = number  */
/* a = address */
/* v = value   */
void ym2612_write(void *chip, UINT8 a, UINT8 v)
{
	YM2612 *F2612 = (YM2612 *)chip;
	FM_OPN *OPN   = &F2612->OPN;
	int addr;

	//v &= 0xff;  /* adjust to 8 bit bus */

	switch( a&3 )
	{
	case 0: /* address port 0 */
		OPN->ST.address = v;
		F2612->addr_A1 = 0;

		/* Write register to SSG emulator */
		if( v < 16 ) OPN->ST.SSG_funcs.write(OPN->ST.SSG_param,a,v);
		break;

	case 1: /* data port 0    */
		if (F2612->addr_A1 != 0)
			break;  /* verified on real YM2608 */

		addr = OPN->ST.address;
		F2612->REGS[addr] = v;
		switch(addr & 0xf0)
		{
		case 0x20:  /* 0x20-0x2f Mode */
			switch( addr )
			{
			case 0x2a:  /* DAC data (YM2612) */
				//ym2612_update_req(F2612);
				F2612->dacout &= 0x01;
				// set high 8 bit of the 9-bit DAC
				F2612->dacout |= ((int)v - 0x80) << 1;   /* level unknown */
				break;
			case 0x2b:  /* DAC Sel  (YM2612) */
				/* b7 = dac enable */
				F2612->dacen = v & 0x80;
				break;
			case 0x2C:  // undocumented: DAC Test Register
				// http://gendev.spritesmind.net/forum/viewtopic.php?p=26996#p26996
				// b7/b6/b4 = various test bits
				// b5 = makes DAC go to channels 0..3 and 5.
				// b3 = 9th DAC bit
				F2612->dac_test = v & 0x20;
				F2612->dacout &= ~0x01;
				// set DAC's 9th bit
				F2612->dacout |= (v & 0x08) >> 3;
				break;
			default:    /* OPN section */
				ym2612_update_req(F2612);
				/* write register */
				OPNWriteMode(OPN,addr,v);
			}
			break;
		default:    /* 0x30-0xff OPN section */
			ym2612_update_req(F2612);
			/* write register */
			OPNWriteReg(OPN,addr,v);
		}
		break;

	case 2: /* address port 1 */
		OPN->ST.address = v;
		F2612->addr_A1 = 1;
		break;

	case 3: /* data port 1    */
		if (F2612->addr_A1 != 1)
			break;  /* verified on real YM2608 */

		ym2612_update_req(F2612);
		addr = OPN->ST.address;
		F2612->REGS[addr | 0x100] = v;
		OPNWriteReg(OPN,addr | 0x100,v);
		break;
	}
	//return OPN->ST.irq;
	return;
}

UINT8 ym2612_read(void *chip, UINT8 a)
{
	YM2612 *F2612 = (YM2612 *)chip;

	switch( a&3)
	{
	case 0: /* status 0 : YM2203 compatible */
		return FM_STATUS_FLAG(&F2612->OPN.ST);
	case 1:
	case 2:
	case 3:
		//LOG(LOG_WAR,("YM2612 #%p:A=%d read unmapped area\n",F2612->OPN.ST.param,a));
		return FM_STATUS_FLAG(&F2612->OPN.ST);
	}
	return 0;
}

UINT8 ym2612_timer_over(void *chip,UINT8 c)
{
	YM2612 *F2612 = (YM2612 *)chip;

	if( c )
	{   /* Timer B */
		TimerBOver( &(F2612->OPN.ST) );
	}
	else
	{   /* Timer A */
		ym2612_update_req(F2612);
		/* timer update */
		TimerAOver( &(F2612->OPN.ST) );
		/* CSM mode key,TL controll */
		if ((F2612->OPN.ST.mode & 0xc0) == 0x80)
		{   /* CSM mode total level latch and auto key on */
			CSMKeyControll( &F2612->OPN, &(F2612->CH[2]) );
		}
	}
	return F2612->OPN.ST.irq;
}


void ym2612_set_mutemask(void *chip, UINT32 MuteMask)
{
	YM2612 *F2612 = (YM2612 *)chip;
	UINT8 CurChn;
	
	for (CurChn = 0; CurChn < 6; CurChn ++)
		F2612->CH[CurChn].Muted = (MuteMask >> CurChn) & 0x01;
	F2612->MuteDAC = (MuteMask >> 6) & 0x01;
	
	return;
}

void ym2612_setoptions(void *chip, UINT32 Flags)
{
	YM2612 *F2612 = (YM2612 *)chip;
	UINT8 PseudoStereo;
	
	PseudoStereo = (Flags >> 2) & 0x01;
	F2612->WaveOutMode = (PseudoStereo) ? 0x01 : 0x00;
	F2612->OPN.LegacyMode = (Flags >> 7) & 0x01;
	
	return;
}
#endif /* (BUILD_YM2612) */
