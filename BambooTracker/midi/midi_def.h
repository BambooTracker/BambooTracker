#pragma once

enum
{
	MidiBufferSize = 8192
};

#define MIDI_INP_CLIENT_NAME "BambooTracker Rx"
#define MIDI_OUT_CLIENT_NAME "BambooTracker Tx"
#define MIDI_INP_PORT_NAME "BambooTracker MIDI In"
#define MIDI_OUT_PORT_NAME "BambooTracker MIDI Out"

#define MIDI_INP_IGNORE_SYSEX false
#define MIDI_INP_IGNORE_TIME false
#define MIDI_INP_IGNORE_SENSE true
