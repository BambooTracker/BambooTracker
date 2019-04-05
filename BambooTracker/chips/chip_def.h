#pragma once

#include <stdint.h>

typedef int32_t	sample;

struct intf2608
{
	void (*set_ay_emu_core)(uint8_t Emulator);
	int (*device_start)(uint8_t ChipID, int clock, uint8_t AYDisable, uint8_t AYFlags, int* AYrate);
	void (*device_stop)(uint8_t ChipID);
	void (*device_reset)(uint8_t ChipID);
	void (*control_port_a_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	void (*data_port_a_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	void (*control_port_b_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	void (*data_port_b_w)(uint8_t ChipID, uint32_t offset, uint8_t data);
	uint8_t (*read_port_r)(uint8_t ChipID, uint32_t offset);
	void (*stream_update)(uint8_t ChipID, sample **outputs, int samples);
	void (*stream_update_ay)(uint8_t ChipID, sample **outputs, int samples);
};
