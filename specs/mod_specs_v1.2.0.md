***NOTE: THIS IS A DRAFT!***
# BambooTracker Module File (.btm) Format Specification
 v1.2.0 - 2019-03-30

- All data are little endian.
- Unless otherwise noted, character encoding of string is ASCII.

---

| Type              | Field           | Description                                                               |
| ----------------- | --------------- | ------------------------------------------------------------------------- |
| string (16 bytes) | File identifier | Format string, must be `BambooTrackerMod`.                                |
| uint32            | EOF offset      | Relative offset to end of file. i.e. File length - 18.                    |
| uint32            | File version    | Version number in BCD-Code. e.g. Version 1.1.1 is stored as `0x00010101`. |


## Module Section
| Type             | Field                       | Description                                                                                             |
| ---------------- | --------------------------- | ------------------------------------------------------------------------------------------------------- |
| string (8 bytes) | Section identifier          | Must be `MODULE  `.                                                                                     |
| uint32           | Module section offset       | Relative offset to end of module section.                                                               |
| uint32           | Title length                | Byte length of title string.                                                                            |
| string (N bytes) | Title                       | Title string. Character encoding is UTF-8. If module is untitled, it is omitted.                        |
| uint32           | Author length               | Byte length of author string.                                                                           |
| string (N bytes) | Author                      | String of author. Character encoding is UTF-8. If author anme is not described, this field is omitted.  |
| uint32           | Copyright length            | Byte length of copyright string.                                                                        |
| string (N bytes) | Copyright                   | String of copyright. Character encoding is UTF-8. If copyright is not described, this field is omitted. |
| uint32           | Comment length              | Byte length of module comment.                                                                          |
| string (N bytes) | Comment                     | String of Module comment. Character encoding is UTF-8. If there is no comment, this field is omitted.   |
| uint32           | Tick frequency              | Tick frequency. e.g. `0x0000003C` (60) is NTSC, `0x00000032` (50) is PAL.                               |
| uint32           | 1st step highlight distance | 1st step highlight distance.                                                                            |
| uint32           | 2nd step highlight distance | 2nd step highlight distance.                                                                            |


## Instrument Section
| Type             | Field                     | Description                                   |
| ---------------- | ------------------------- | --------------------------------------------- |
| string (8 bytes) | Section identifier        | Must be `INSTRMNT`.                           |
| uint32           | Instrument section offset | Relative offset to end of instrument section. |
| uint8            | Instrument count          | Number of instruments.                        |

After instrument count, details of each instrument are described.

| Type             | Field                  | Description                                                                                                   |
| ---------------- | ---------------------- | ------------------------------------------------------------------------------------------------------------- |
| uint8            | Instrument index       | Index of the instrument.                                                                                      |
| uint32           | instrument offset      | Relative offset to end of the instrument details.                                                             |
| uint32           | Instrument name length | Length of instrument name.                                                                                    |
| string (N bytes) | Instrument name        | String of instrument name. Character encoding is UTF-8. If instrument name is not set, this field is omitted. |
| uint8            | Instrument type        | Sound souce of the instrument. `0x00` is FM, and `0x01` is SSG.                                               |

The following data change depending on sound source of the instrument.

### FM
| Type  | Field              | Description                                                                                      |
| ----- | ------------------ | ------------------------------------------------------------------------------------------------ |
| uint8 | Envelope number    | Envelope number.                                                                                 |
| uint8 | LFO number         | Bit 0-6 is LFO number, and bit 7 is flag. If bit 7 is clear, it uses LFO.                        |
| uint8 | AL sequence number | Bit 0-6 is algorithm sequence number, and bit 7 is flag. If bit 7 is clear, it uses AL sequence. |
| uint8 | FB sequence number | Bit 0-6 is feedback sequence number, and bit 7 is flag. If bit 7 is clear, it uses FB sequence.  |

After FB sequence number, it repeats 9 operator's parameters for each operator (1-4).

| Type  | Field              | Description                                                                                                          |
| ----- | ------------------ | -------------------------------------------------------------------------------------------------------------------- |
| uint8 | AR sequence number | Bit 0-6 is attack rate sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses AR sequence.   |
| uint8 | DR sequence number | Bit 0-6 is decay rate sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses DR sequence.    |
| uint8 | SR sequence number | Bit 0-6 is sustain rate sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses SR sequence.  |
| uint8 | RR sequence number | Bit 0-6 is release rate sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses RR sequence.  |
| uint8 | SL sequence number | Bit 0-6 is sustain level sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses SL sequence. |
| uint8 | TL sequence number | Bit 0-6 is total level sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses TL sequence.   |
| uint8 | KS sequence number | Bit 0-6 is key scale sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses KS sequence.     |
| uint8 | ML sequence number | Bit 0-6 is multiple sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses ML sequence.      |
| uint8 | DT sequence number | Bit 0-6 is detune sequence number of the operator, and bit 7 is flag. If bit 7 is clear, it uses DT sequence.        |

| Type  | Field                               | Description                                                                                                                                                            |
| ----- | ----------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| uint8 | Arpeggio sequence number            | Bit 0-6 is arpeggio sequence number, and bit 7 is flag. If bit 7 is clear, it uses arpeggio sequence for all operators.                                                |
| uint8 | Pitch sequence number               | Bit 0-6 is pitch sequence number, and bit 7 is flag. If bit 7 is clear, it uses pitch sequence for all operators.                                                      |
| uint8 | Envelope reset flag                 | Flag for envelope reset. Bit 0 is for all operators, bit 1 is for operator 1 and bit 3 is for operator 4. If bit is set, envelope reset is enabled for correspondings. |
| uint8 | Operator 1 arpeggio sequence number | Bit 0-6 is arpeggio sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 1 arpeggio sequence in FM 3ch expansion mode.                              |
| uint8 | Operator 2 arpeggio sequence number | Bit 0-6 is arpeggio sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 2 arpeggio sequence in FM 3ch expansion mode.                              |
| uint8 | Operator 3 arpeggio sequence number | Bit 0-6 is arpeggio sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 3 arpeggio sequence in FM 3ch expansion mode.                              |
| uint8 | Operator 4 arpeggio sequence number | Bit 0-6 is arpeggio sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 4 arpeggio sequence in FM 3ch expansion mode.                              |
| uint8 | Operator 1 pitch sequence number    | Bit 0-6 is pitch sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 1 pitch sequence in FM 3ch expansion mode.                                    |
| uint8 | Operator 2 pitch sequence number    | Bit 0-6 is pitch sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 2 pitch sequence in FM 3ch expansion mode.                                    |
| uint8 | Operator 3 pitch sequence number    | Bit 0-6 is pitch sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 3 pitch sequence in FM 3ch expansion mode.                                    |
| uint8 | Operator 4 pitch sequence number    | Bit 0-6 is pitch sequence number, and bit 7 is flag. If bit 7 is clear, it uses operator 4 pitch sequence in FM 3ch expansion mode.                                    |


### SSG
| Type  | Field                      | Description                                                                                               |
| ----- | -------------------------- | --------------------------------------------------------------------------------------------------------- |
| uint8 | Wave form sequence number  | Bit 0-6 is wave form sequence number, and bit 7 is flag. If bit 7 is clear, it uses wave form sequence.   |
| uint8 | Tone/Noise sequence number | Bit 0-6 is tone/noise sequence number, and bit 7 is flag. If bit 7 is clear, it uses tone/noise sequence. |
| uint8 | Envelope sequence number   | Bit 0-6 is envelope sequence number, and bit 7 is flag. If bit 7 is clear, it uses envelope sequence.     |
| uint8 | Arpeggio sequence number   | Bit 0-6 is arpeggio sequence number, and bit 7 is flag. If bit 7 is clear, it uses arpeggio sequence.     |
| uint8 | Pitch sequence number      | Bit 0-6 is pitch sequence number, and bit 7 is flag. If bit 7 is clear, it uses pitch sequence.           |


## Instrument Property Section
| Type            | Field                              | Description                                            |
| --------------- | ---------------------------------- | ------------------------------------------------------ |
| string (8bytes) | Seciton identifier                 | Must be `INSTPROP`.                                    |
| uint32          | Instrument property section offset | Relative offset to end of instrument property section. |

This section contains subsections of each instrument property.

| Type  | Field                 | Description                                            |
| ----- | --------------------- | ------------------------------------------------------ |
| uint8 | Subsection identifier | Identify subsection type. See table below for details. |
| uint8 | block count           | Number of property blocks.                             |

Subsection identifier is defined as:

| Value         | Subsection type                                                      |
| ------------- | -------------------------------------------------------------------- |
| `0x00`        | FM envelope                                                          |
| `0x01`        | FM LFO                                                               |
| `0x02`        | FM AL sequence                                                       |
| `0x03`        | FM FB sequence                                                       |
| `0x04`-`0x0C` | FM operator 1 sequences (in the order defined in instrument section) |
| `0x0E`-`0x15` | FM operator 2 sequences (in the order defined in instrument section) |
| `0x16`-`0x1E` | FM operator 3 sequences (in the order defined in instrument section) |
| `0x1F`-`0x27` | FM operator 4 sequences (in the order defined in instrument section) |
| `0x28`        | FM arpeggio sequence                                                 |
| `0x29`        | FM pitch sequence                                                    |
| `0x30`        | SSG wave form sequence                                               |
| `0x31`        | SSG tone/noise sequence                                              |
| `0x32`        | SSG envelope sequence                                                |
| `0x33`        | SSG arpeggio sequence                                                |
| `0x34`        | SSG pitch sequence                                                   |

 And repeats sequence data block.

### FM Envelope
| Type  | Field  | Description                                           |
| ----- | ------ | ----------------------------------------------------- |
| uint8 | Index  | Envelope index number.                                |
| uint8 | Offset | Relative offset to end of the envelope block.         |
| uint8 | AL/FB  | High nibble is algorithm, and low nibble is feedback. |

After this, repeat parameters in the table below for each operator.

| Type  | Field     | Description                                                                                       |
| ----- | --------- | ------------------------------------------------------------------------------------------------- |
| uint8 | Enable/AR | Flag which the operator is enabled when bit 5 is set, and attack rate is bit 0-4.                 |
| uint8 | KS/DR     | Bit 0-4 is decay rate, and bit 5-6 is key scale.                                                  |
| uint8 | DT/SR     | Bit 0-4 is sustain rate, and bit 5-7 is detune.                                                   |
| uint8 | SL/RR     | Low nibble is release, and High nibble is sustain level.                                          |
| uint8 | TL        | Total level.                                                                                      |
| uint8 | SSGEGs/ML | Low nibble is multiple, high nibble is type of SSGEG. If SSGEG is disabled, high nibble is `0x8`. |

### LFO
| Type  | Field         | Description                                                                                                                                                      |
| ----- | ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| uint8 | Index         | LFO index number.                                                                                                                                                |
| uint8 | Offset        | Relative offset to end of the LFO block.                                                                                                                         |
| uint8 | Frequency/PMS | High nibble is LFO moduletion frequency, and low nibble is phase modulation sensitivity.                                                                         |
| uint8 | AMops/AMS     | high nibble is AM operator flags and low nibble is amplitude modulation sensitivity. Bit 4 is operator 1 and bit 7 is operator 4. If flag is set, AM is enabled. |
| uint8 | Start count   | Tick wait count before beginning LFO.                                                                                                                            |

### Sequence
Sequence-type data block (e.g. FM arpeggio, SSG envelope) is defined as:

| Type   | Field           | Description                                   |
| ------ | --------------- | --------------------------------------------- |
| uint8  | Index           | Index number.                                 |
| uint16 | Offset          | Relative offset to end of the sequence block. |
| uint16 | Sequence length | Length of sequence.                           |

And repeat sequence data units.

| Type   | Field        | Description                                                                           |
| ------ | ------------ | ------------------------------------------------------------------------------------- |
| uint16 | Unit data    | Value of unit. This also indicates row number of sequence editor.                     |
| int32  | Unit subdata | Unit subdata. Only used by SSG waveform and envelope, and omitted in other sequences. |

Unit subdata is used in SSG waveform and envelope sequence.  
In waveform sequence, it indecates square mask frequency.  
In envelope sequence, it indecates hardware envelope frequency.

After sequences, loops are stored.

| Type   | Field       | Description      |
| ------ | ----------- | ---------------- |
| uint16 | Loop counts | Number of loops. |

Loop unit is defined as the table below. If it is stored `0x00` in loop counts i.g. there is no loop, the unit is omitted.

| Type   | Field        | Description                                                                             |
| ------ | ------------ | --------------------------------------------------------------------------------------- |
| uint16 | Begin point  | Count from head of sequence where loop starts.                                          |
| uint16 | End point    | Count from head of sequence where loop stops.                                           |
| uint8  | Repeat count | Count of loop repeating. Note: If `0x01` is stored, it is interpreted as infinity loop. |

There is release details in the end of subsequence block.

| Type   | Field         | Description                                                                                      |
| ------ | ------------- | ------------------------------------------------------------------------------------------------ |
| uint8  | Release type  | `0x00`: no release, `0x01`: fix, `0x02`: absolute, `0x03`: releative.                            |
| uint16 | Release point | Count from head of sequence where loop starts. If release type is `0x00`, this field is omitted. |

| Type  | Field         | Description                                    |
| ----- | ------------- | ---------------------------------------------- |
| uint8 | Sequence type | Type of sequence. See table below for details. |

Sequence type is defined as:

| Value  | Type      |
| ------ | --------- |
| `0x00` | Absolute. |
| `0x01` | Fix.      |
| `0x02` | Relative. |

FM/SSG arpeggio can be selected from all of these, FM/SSG pitch can be absolute or relative, and other properties must be set to `0x00`.

## Groove Section
| Type            | Field                 | Description                               |
| --------------- | --------------------- | ----------------------------------------- |
| string (8bytes) | Seciton identifier    | Must be `GROOVE  `.                       |
| uint32          | Groove section offset | Relative offset to end of groove section. |
| uint8           | Groove count          | Number of groove sequences - 1.           |

Groove sequences are repeated after groove count.

| Type  | Field           | Description         |
| ----- | --------------- | ------------------- |
| uint8 | Index           | Index of sequence.  |
| uint8 | Sequence length | Length of sequence. |

And sequence are stored after sequence length as uint8.


## Song Section
| Type            | Field               | Description                             |
| --------------- | ------------------- | --------------------------------------- |
| string (8bytes) | Seciton identifier  | Must be `SONG    `.                     |
| uint32          | Song section offset | Relative offset to end of song section. |
| uint8           | Song count          | Number of songs.                        |

Each song block is defined as:

| Type             | Field                           | Description                                                                                                  |
| ---------------- | ------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| uint8            | Index                           | Index number.                                                                                                |
| uint32           | Offset                          | Relative offset to end of the song block.                                                                    |
| uint32           | Title length                    | Length of song title.                                                                                        |
| string (N bytes) | Title                           | Song title string. Character encoding is UTF-8. If song is untitled, it is omitted.                          |
| uint32           | Tempo                           | Tempo.                                                                                                       |
| uint8            | Groove index, Tempo/Groove flag | Bit 0-6 is index of using groove. If bit 7 is set, Tempo is enabled. If bit 7 is cleared, groove is enabled. |
| uint32           | Speed                           | Speed.                                                                                                       |
| uint8            | Pattern size                    | Stored default pattern size - 1.                                                                             |
| uint8            | Song type                       | Type of tracks. See table below for details.                                                                 |

Song type defined number and order of tracks.

| Type                    | Track0 | Track1 | Track2    | Track3    | Track4    | Track5    | Track6 | Track7 | Track8 | Track9 | Track10 | Track11 | Track12 | Track13 | Track14 | Track15 | Track16 | Track17 |
| ----------------------- | ------ | ------ | --------- | --------- | --------- | --------- | ------ | ------ | ------ | ------ | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- |
| `0x00` (Standard)       | FM1ch  | FM2ch  | FM3ch     | FM4ch     | FM5ch     | FM6ch     | SSG1ch | SSG2ch | SSG3ch | BD     | SD      | TOP     | HH      | TOM     | RIM     | -       | -       | -       |
| `0x01` (FM3ch expanded) | FM1ch  | FM2ch  | FM3ch-op1 | FM3ch-op2 | FM3ch-op3 | FM3ch-op4 | FM4ch  | FM5ch  | FM6ch  | SSG1ch | SSG2ch  | SSG3ch  | BD      | SD      | TOP     | HH      | TOM     | RIM     |

Song section includes some track subblock.

| Type      | Field        | Description                               |
| --------- | ------------ | ----------------------------------------- |
| uint8     | Track number | Number of the track.                      |
| uint32    | Track offset | Relative offset to end of track subblock. |
| uint8     | Order length | Set Length of order - 1.                  |
| uint8 x N | Order data   | Stored order data list.                   |

After order data, repeat pattern subblocks.

| Type   | Field          | Description                                 |
| ------ | -------------- | ------------------------------------------- |
| uint8  | Pattern number | Number of the pattern.                      |
| uint32 | Pattern offset | Relative offset to end of pattern subblock. |

And repeat step unit which is stored event.

| Type   | Field      | Description                                                 |
| ------ | ---------- | ----------------------------------------------------------- |
| uint8  | Step index | Index of the step.                                          |
| uint16 | Event flag | Flag whether event is entried. See table below for details. |

Event flag is flags of step events. If flag is set (1), the event is described after event flag.

| bit | Event          |
| --- | -------------- |
| 0   | Key (Note)     |
| 1   | Instrument     |
| 2   | Volume         |
| 3   | Effect 1 ID    |
| 4   | Effect 1 value |
| 5   | Effect 2 ID    |
| 6   | Effect 2 value |
| 7   | Effect 3 ID    |
| 8   | Effect 3 value |
| 9   | Effect 4 ID    |
| 10  | Effect 4 value |

Step events stored in the order of event flag (Key -> Instrument -> Volume -> ...).

| Type             | Event                                   |
| ---------------- | --------------------------------------- |
| int8             | Key event. See table below for details. |
| uint8            | Instrument number.                      |
| uint8            | Volume.                                 |
| string (2 bytes) | Effect ID.                              |
| uint8            | Effect value.                           |

Key event details:

| Value | Description                                                                         |
| ----- | ----------------------------------------------------------------------------------- |
| 0<=   | Key on. Octave is `value % 12`. Note is `value / 12`. Note `0` is C, and `11` is B. |
| -2    | Key off.                                                                            |
| -3    | Echo buffer 0 access.                                                               |
| -4    | Echo buffer 1 access.                                                               |
| -5    | Echo buffer 2 access.                                                               |
| -6    | Echo buffer 3 access.                                                               |

---

## History
| Version | Date       | Detail                                     |
| ------- | ---------- | ------------------------------------------ |
| 1.1.0   | 2019-03-24 | Added fields for FM3ch expanded mode.      |
| 1.0.3   | 2019-03-18 | Added 2nd step hilight.                    |
| 1.0.2   | 2018-12-29 | Revised for the change of FM octave range. |
| 1.0.1   | 2018-12-10 | Added instrument sequence type.            |
| 1.0.0   | 2018-11-23 | Initial release.                           |
