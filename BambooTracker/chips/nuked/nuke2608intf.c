#include <stdlib.h>	/* for free */
#include <string.h>	/* for memset */
#include <stddef.h>	/* for NULL */
#include "nuke2608intf.h"
#include "ym3438.h"

/*// Only use EC_EMU2149
//#define ENABLE_ALL_CORES*/

#ifdef ENABLE_ALL_CORES
#define EC_MAME		0x01	/* AY8910 core from MAME */
#endif
#define EC_EMU2149	0x00

typedef struct _ym2608_state ym2608_state;
struct _ym2608_state
{
	ym3438_t *		chip;
	void *			psg;
	int			clock;
	ym2608_interface intf;
	uint32_t dramSize;
};

static uint8_t AY_EMU_CORE = 0x00;

#define MAX_CHIPS	0x02
static ym2608_state YM2608Data[MAX_CHIPS];

static void psg_set_clock(void *param, int clock)
{
	ym2608_state *info = (ym2608_state *)param;
	if (info->psg != NULL)
	{
		switch(AY_EMU_CORE)
		{
#ifdef ENABLE_ALL_CORES
		case EC_MAME:
			ay8910_set_clock_ym(info->psg, clock);
			break;
#endif
		case EC_EMU2149:
			PSG_set_clock((PSG*)info->psg, clock);
			break;
		}
	}
}

static void psg_write(void *param, int address, int data)
{
	ym2608_state *info = (ym2608_state *)param;
	if (info->psg != NULL)
	{
		switch(AY_EMU_CORE)
		{
#ifdef ENABLE_ALL_CORES
		case EC_MAME:
			ay8910_write_ym(info->psg, address, data);
			break;
#endif
		case EC_EMU2149:
			PSG_writeIO((PSG*)info->psg, address, data);
			break;
		}
	}
}

static int psg_read(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	if (info->psg != NULL)
	{
		switch(AY_EMU_CORE)
		{
#ifdef ENABLE_ALL_CORES
		case EC_MAME:
			return ay8910_read_ym(info->psg);
#endif
		case EC_EMU2149:
			return PSG_readIO((PSG*)info->psg);
		}
	}
	return 0x00;
}

static void psg_reset(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	if (info->psg != NULL)
	{
		switch(AY_EMU_CORE)
		{
#ifdef ENABLE_ALL_CORES
		case EC_MAME:
			ay8910_reset_ym(info->psg);
			break;
#endif
		case EC_EMU2149:
			PSG_reset((PSG*)info->psg);
			break;
		}
	}
}

static const struct OPN2mod_psg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};

void nuke2608_set_ay_emu_core(uint8_t Emulator)
{
#ifdef ENABLE_ALL_CORES
	AY_EMU_CORE = (Emulator < 0x02) ? Emulator : 0x00;
#else
	(void)Emulator;
	AY_EMU_CORE = EC_EMU2149;
#endif

	return;
}

int device_start_nuke2608(uint8_t ChipID, int clock, uint8_t AYDisable, uint8_t AYFlags, int *AYrate, uint32_t dramSize)
{
	static const ym2608_interface generic_2608 =
	{
		{
			AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
			AY8910_DEFAULT_LOADS
			/*DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL*/
		},
		NULL
	};
	ym2608_interface *intf;
	ym2608_state *info;
	int rate;
	int ay_clock;

	if (ChipID >= MAX_CHIPS)
		return 0;

	info = &YM2608Data[ChipID];
	info->clock = clock;
	rate = clock / 144;	/* FM synthesis rate is clock / 2 / 72 */

	info->intf = generic_2608;
	intf = &info->intf;
	if (AYFlags)
		intf->ay8910_intf.flags = AYFlags;

	info->chip = (ym3438_t *)calloc(1, sizeof(ym3438_t));
	if (!info->chip)
		return 0;

	/* FIXME: Force to use single output */
	/*info->psg = ay8910_start_ym(NULL, SOUND_YM2608, clock, &intf->ay8910_intf);*/
	if (! AYDisable)
	{
		ay_clock = clock / 4;
		*AYrate = ay_clock / 8;
		switch(AY_EMU_CORE)
		{
#ifdef ENABLE_ALL_CORES
		case EC_MAME:
			info->psg = ay8910_start_ym(NULL, CHTYPE_YM2608, ay_clock, &intf->ay8910_intf);
			break;
#endif
		case EC_EMU2149:
			info->psg = PSG_new(ay_clock, *AYrate);
			if (info->psg == NULL)
				return 0;
			PSG_setVolumeMode((PSG*)info->psg, 1);	/* YM2149 volume mode */
			break;
		}
	}
	else
	{
		info->psg = NULL;
		*AYrate = 0;
	}

	info->dramSize = dramSize;
	OPN2_Reset(info->chip, clock, &psgintf, info, dramSize);

	return rate;
}

void device_stop_nuke2608(uint8_t ChipID)
{
	ym2608_state *info = &YM2608Data[ChipID];
	OPN2_Destroy(info->chip);
	free(info->chip);
	if (info->psg != NULL)
	{
		switch(AY_EMU_CORE)
		{
#ifdef ENABLE_ALL_CORES
		case EC_MAME:
			ay8910_stop_ym(info->psg);
			break;
#endif
		case EC_EMU2149:
			PSG_delete((PSG*)info->psg);
			break;
		}
		info->psg = NULL;
	}
}

void device_reset_nuke2608(uint8_t ChipID)
{
	ym2608_state *info = &YM2608Data[ChipID];
	OPN2_FlushBuffer(info->chip);
	OPN2_Reset(info->chip, info->clock, &psgintf, info, info->dramSize);
}

void nuke2608_control_port_a_w(uint8_t ChipID, uint32_t offset, uint8_t data)
{
	ym2608_state *info = &YM2608Data[ChipID];
	(void)offset;
	OPN2_WriteBuffered(info->chip, 0, data);
}

void nuke2608_control_port_b_w(uint8_t ChipID, uint32_t offset, uint8_t data)
{
	ym2608_state *info = &YM2608Data[ChipID];
	(void)offset;
	OPN2_WriteBuffered(info->chip, 2, data);
}

void nuke2608_data_port_a_w(uint8_t ChipID, uint32_t offset, uint8_t data)
{
	ym2608_state *info = &YM2608Data[ChipID];
	(void)offset;
	OPN2_WriteBuffered(info->chip, 1, data);
}

void nuke2608_data_port_b_w(uint8_t ChipID, uint32_t offset, uint8_t data)
{
	ym2608_state *info = &YM2608Data[ChipID];
	(void)offset;
	OPN2_WriteBuffered(info->chip, 3, data);
}

uint8_t nuke2608_read_port_r(uint8_t ChipID, uint32_t offset)
{
	ym2608_state *info = &YM2608Data[ChipID];
	(void)offset;
	return OPN2_Read(info->chip, 1);
}

void nuke2608_stream_update(uint8_t ChipID, sample **outputs, int samples)
{
	int i;
	ym2608_state *info = &YM2608Data[ChipID];
	sample *bufl = outputs[0];
	sample *bufr = outputs[1];

	for (i = 0; i < samples; ++i)
	{
		sample lr[2];
		OPN2_Generate(info->chip, lr);
		*bufl++ = lr[0];
		*bufr++ = lr[1];
	}
}

void nuke2608_stream_update_ay(uint8_t ChipID, sample **outputs, int samples)
{
	ym2608_state *info = &YM2608Data[ChipID];

	if (info->psg != NULL)
	{
		switch(AY_EMU_CORE)
		{
#ifdef ENABLE_ALL_CORES
		case EC_MAME:
			ay8910_update_one(info->psg, outputs, samples);
			break;
#endif
		case EC_EMU2149:
			PSG_calc_stereo((PSG*)info->psg, outputs, samples);
			break;
		}
	}
	else
	{
		memset(outputs[0], 0x00, samples * sizeof(sample));
		memset(outputs[1], 0x00, samples * sizeof(sample));
	}
}

struct intf2608 nuked_intf2608 =
{
	/*.set_ay_emu_core =*/ &nuke2608_set_ay_emu_core,
	/*.device_start =*/ &device_start_nuke2608,
	/*.device_stop =*/ &device_stop_nuke2608,
	/*.device_reset =*/ &device_reset_nuke2608,
	/*.control_port_a_w =*/ &nuke2608_control_port_a_w,
	/*.control_port_b_w =*/ &nuke2608_control_port_b_w,
	/*.data_port_a_w =*/ &nuke2608_data_port_a_w,
	/*.data_port_b_w =*/ &nuke2608_data_port_b_w,
	/*.read_port_r =*/ &nuke2608_read_port_r,
	/*.stream_update =*/ &nuke2608_stream_update,
	/*.stream_update_ay =*/ &nuke2608_stream_update_ay,
};
