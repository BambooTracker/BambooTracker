#pragma once

enum
{
	MidiBufferSize = 8192
};

#define MIDI_INP_CLIENT_NAME "BambooTracker Rx"
#define MIDI_INP_PORT_NAME "BambooTracker MIDI In"

#define MIDI_INP_IGNORE_SYSEX false
#define MIDI_INP_IGNORE_TIME false
#define MIDI_INP_IGNORE_SENSE true
