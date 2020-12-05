#pragma once

#include <stdint.h>

typedef int32_t	sample;

enum { CHIP_SMPL_BUF_SIZE_ = 0x10000 };

enum Stereo { STEREO_LEFT, STEREO_RIGHT };

struct intf2608
{
	void (*set_ay_emu_core)(uint8_t Emulator);
	int (*device_start)(uint8_t ChipID, int clock, uint8_t AYDisable, uint8_t AYFlags, int* AYrate, uint32_t dramSize);
	void (*device_stop)(uint8_t ChipID);
	void (*device_reset)(uint8_t ChipID);
	void (*control_port_a_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	void (*control_port_b_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	void (*data_port_a_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	void (*data_port_b_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	uint8_t (*read_port_r)(uint8_t ChipID, uint32_t offset);
	void (*stream_update)(uint8_t ChipID, sample **outputs, int samples);
	void (*stream_update_ay)(uint8_t ChipID, sample **outputs, int samples);
};

#ifndef INCLUDE_AY8910_H
/* Copied from VGMPlay/chips/ay8910.h */
/*
* Default values for resistor loads.
* The macro should be used in AY8910interface if
* the real values are unknown.
*/
#define AY8910_DEFAULT_LOADS		{1000, 1000, 1000}

/*
* The following is used by all drivers not reviewed yet.
* This will like the old behaviour, output between
* 0 and 7FFF
*/
#define AY8910_LEGACY_OUTPUT		(1)

/*
* Specifing the next define will simulate the special
* cross channel mixing if outputs are tied together.
* The driver will only provide one stream in this case.
*/
#define AY8910_SINGLE_OUTPUT		(2)

typedef struct _ay8910_interface ay8910_interface;
struct _ay8910_interface
{
	int					flags;			/* Flags */
	int					res_load[3]; 	/* Load on channel in ohms */
										/*devcb_read8		portAread;*/
										/*devcb_read8		portBread;*/
										/*devcb_write8		portAwrite;*/
										/*devcb_write8		portBwrite;*/
};
#endif

typedef struct _ym2608_interface ym2608_interface;
struct _ym2608_interface
{
	ay8910_interface ay8910_intf;
	/*void ( *handler )( const device_config *device, int irq );*/	/* IRQ handler for the YM2608 */
	void ( *handler )( int irq );	/* IRQ handler for the YM2608 */
};
