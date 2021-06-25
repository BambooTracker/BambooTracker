# BambooTracker Instrument File (.bti) Format Specification
v1.5.0 - 2021-06-13

- All data are little endian.
- Unless otherwise noted, character encoding of string is ASCII.

---

## Header
| Type              | Field           | Description                                                               |
| ----------------- | --------------- | ------------------------------------------------------------------------- |
| string (16 bytes) | File identifier | Format string, must be `BambooTrackerIst`.                                |
| uint32            | EOF offset      | Relative offset to end of file. i.e. File length - 18.                    |
| uint32            | File version    | Version number in BCD-Code. e.g. Version 1.5.0 is stored as `0x00010500`. |


## Instrument Section
| Type             | Field                     | Description                                                                                                   |
| ---------------- | ------------------------- | ------------------------------------------------------------------------------------------------------------- |
| string (8 bytes) | Section identifier        | Must be `INSTRMNT`.                                                                                           |
| uint32           | Instrument section offset | Relative offset to end of instrument section.                                                                 |
| uint32           | Instrument name length    | Length of instrument name.                                                                                    |
| string (N bytes) | Instrument name           | String of instrument name. Character encoding is UTF-8. If instrument name is not set, this field is omitted. |
| uint8            | Instrument type           | Type of the instrument. `0x00`: FM, `0x01`: SSG, `0x02`: ADPCM, `0x03`: Drumkit.                              |

The following data change depending on sound source of the instrument.


### FM
| Type  | Field                         | Description                                                                                                                                                            |
| ----- | ----------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| uint8 | Envelope reset flag           | Flag for envelope reset. Bit 0 is for all operators, bit 1 is for operator 1 and bit 3 is for operator 4. If bit is set, envelope reset is enabled for correspondings. |
| uint8 | All operators arpeggio number | If bit 7 is clear, arpeggio for all operators is enabled. bit 0-6 are the number: n-th FM arpeggio property. n have to be 0 if it is unused.                           |
| uint8 | Operator 1 arpeggio number    | If bit 7 is clear, operator 1 arpeggio is enabled. bit 0-6 are the number: n-th FM arpeggio property. n have to be 0 if it is unused.                                  |
| uint8 | Operator 2 arpeggio number    | If bit 7 is clear, operator 2 arpeggio is enabled. bit 0-6 are the number: n-th FM arpeggio property. n have to be 0 if it is unused.                                  |
| uint8 | Operator 3 arpeggio number    | If bit 7 is clear, operator 3 arpeggio is enabled. bit 0-6 are the number: n-th FM arpeggio property. n have to be 0 if it is unused.                                  |
| uint8 | Operator 4 arpeggio number    | If bit 7 is clear, operator 4 arpeggio is enabled. bit 0-6 are the number: n-th FM arpeggio property. n have to be 0 if it is unused.                                  |
| uint8 | All operators pitch number    | If bit 7 is clear, pitch for all operators is enabled. bit 0-6 are the number: n-th FM pitch property. n have to be 0 if it is unused.                                 |
| uint8 | Operator 1 pitch number       | If bit 7 is clear, operator 1 pitch is enabled. bit 0-6 are the number: n-th FM pitch property. n have to be 0 if it is unused.                                        |
| uint8 | Operator 2 pitch number       | If bit 7 is clear, operator 2 pitch is enabled. bit 0-6 are the number: n-th FM pitch property. n have to be 0 if it is unused.                                        |
| uint8 | Operator 3 pitch number       | If bit 7 is clear, operator 3 pitch is enabled. bit 0-6 are the number: n-th FM pitch property. n have to be 0 if it is unused.                                        |
| uint8 | Operator 4 pitch number       | If bit 7 is clear, operator 4 pitch is enabled. bit 0-6 are the number: n-th FM pitch property. n have to be 0 if it is unused.                                        |

### SSG
SSG instrument has no specific data.


### ADPCM
ADPCM instrument has no specific data.


### ADPCM Drumkit
| Type  | Field     | Description                 |
| ----- | --------- | --------------------------- |
| uint8 | Key count | The count of assigned keys. |

After key count, it repeats data of assigned key.

| Type  | Field         | Description                       |
| ----- | ------------- | --------------------------------- |
| uint8 | Key number    | Assigned key number.              |
| uint8 | Sample ID     | Sample identifier.                |
| int8  | Key pitch     | Assigned key pitch.               |
| uint8 | Panning flags | Bit 0 is right and bit 1 is left. |


## Instrument Property Section
| Type            | Field                              | Description                                            |
| --------------- | ---------------------------------- | ------------------------------------------------------ |
| string (8bytes) | Seciton identifier                 | Must be `INSTPROP`.                                    |
| uint32          | Instrument property section offset | Relative offset to end of instrument property section. |

This section contains subsections of each instrument property.

| Type  | Field                 | Description                                            |
| ----- | --------------------- | ------------------------------------------------------ |
| uint8 | Subsection identifier | Identify subsection type. See table below for details. |

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
| `0x2A`        | FM panning sequence                                                  |
| `0x30`        | SSG waveform sequence                                                |
| `0x31`        | SSG tone/noise sequence                                              |
| `0x32`        | SSG envelope sequence                                                |
| `0x33`        | SSG arpeggio sequence                                                |
| `0x34`        | SSG pitch sequence                                                   |
| `0x40`        | ADPCM sample                                                         |
| `0x41`        | ADPCM envelope sequence                                              |
| `0x42`        | ADPCM arpeggio sequence                                              |
| `0x43`        | ADPCM pitch sequence                                                 |
| `0x44`        | ADPCM panning sequence                                               |

And repeats sequence data block.  
Note that multiple FM arpeggio and pitch sequences can be described for each operator.
ADPCM samples are assigned IDs in the order in which they are stored.


### FM envelope
| Type  | Field  | Description                                           |
| ----- | ------ | ----------------------------------------------------- |
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


### FM LFO
| Type  | Field         | Description                                                                                                                                                      |
| ----- | ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| uint8 | Offset        | Relative offset to end of the LFO block.                                                                                                                         |
| uint8 | Frequency/PMS | High nibble is LFO moduletion frequency, and low nibble is phase modulation sensitivity.                                                                         |
| uint8 | AMops/AMS     | high nibble is AM operator flags and low nibble is amplitude modulation sensitivity. Bit 4 is operator 1 and bit 7 is operator 4. If flag is set, AM is enabled. |
| uint8 | Start count   | Tick wait count before beginning LFO.                                                                                                                            |


### ADPCM sample
| Type      | Field         | Description                                              |
| --------- | ------------- | -------------------------------------------------------- |
| uint32    | Offset        | Relative offset to end of the sample block.              |
| uint8     | Root key      | Root key number.                                         |
| uint16    | Root delta-N  | Delta-N (sample rate) in root key.                       |
| uint8     | Repeat flag   | If bit 0 is set, this ADPCM sample is played repeatedly. |
| uint32    | Sample length | Length of ADPCM sample.                                  |
| uint8 x N | Sample        | Raw ADPCM sample is stored.                              |


### Sequence
Sequence-type data block (e.g. FM arpeggio, SSG envelope) is defined as:

| Type   | Field           | Description                                   |
| ------ | --------------- | --------------------------------------------- |
| uint16 | Offset          | Relative offset to end of the sequence block. |
| uint16 | Sequence length | Length of sequence.                           |

And repeat sequence data units.

| Type   | Field        | Description                                                                                                                            |
| ------ | ------------ | -------------------------------------------------------------------------------------------------------------------------------------- |
| uint16 | Unit data    | Value of unit. This also indicates row number of sequence editor. For details, see the subsection *Sequence Unit*.                     |
| int32  | Unit subdata | Unit subdata. Only used by SSG waveform and envelope, and omitted in other sequences. For details, see the subsection *Sequence Unit*. |

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

| Type   | Field         | Description                                                                                                                                                                 |
| ------ | ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| uint8  | Release type  | `0x00`: no release, `0x01`: fixed, `0x02`: absolute, `0x03`: releative. SSG envelope can be selected from all of these, and other properties can use only `0x00` or `0x01`. |
| uint16 | Release point | Count from head of sequence where loop starts. If release type is `0x00`, this field is omitted.                                                                            |

| Type  | Field         | Description                                    |
| ----- | ------------- | ---------------------------------------------- |
| uint8 | Sequence type | Type of sequence. See table below for details. |

Sequence type is defined as:

| Value  | Type      |
| ------ | --------- |
| `0x00` | Absolute. |
| `0x01` | Fixed.    |
| `0x02` | Relative. |

FM/SSG arpeggio can be selected from all of these, FM/SSG pitch can be absolute or relative, and other properties must be set to `0x00`.


#### Sequence Unit
In FM Operator sequence, unit data is the value of operator parameter.

In FM and SSG arpeggio, unit data has 2 interpretations depending on its sequence type:

- Absolute, Relative: Tone distance from the criterion 0.
- Fixed: Tone distance from C4.

In FM and SSG pitch, unit data is the tone distance from the criterion 0.

In SSG waveform, unit data represents the waveform:

| Unit data | Waveform                        |
| --------- | ------------------------------- |
| `0x00`    | Square                          |
| `0x01`    | Triangle                        |
| `0x02`    | Sawtooth                        |
| `0x03`    | Inversed sawtooth               |
| `0x04`    | Square-masked triangle          |
| `0x05`    | Square-masked sawtooth          |
| `0x06`    | Square-masked inversed sawtooth |

When waveform is square-masked, unit subdata is set one of the 2 types of data:

- If bit 16 is 0, it is raw data. Bit 0-11 is square mask period.
- If bit 16 is 1, it is tone/mask ratio. Bit 0-7 is mask part and bit 8-15 is tone part.

The other waveform set unit subdata to `-1`.

In SSG tone/noise, unit data defined as:

| Unit data     | Type                                               |
| ------------- | -------------------------------------------------- |
| `0x00`        | Tone.                                              |
| `0x01`-`0x20` | Noise. The period is set as `value` - 1.           |
| `0x21`-`0x40` | Tone & Noise. Noise period is set as `value` - 33. |

In SSG envelope, unit data defined as:

| Unit data     | Type                                                                                     |
| ------------- | ---------------------------------------------------------------------------------------- |
| `0x00`-`0x0F` | Software envelope. The value represents SSG channel volume. Unit subdata is set as `-1`. |
| `0x10`-`0x17` | Hardware envelope. The envelope shape number is specified as `value` - 16.               |

When unit data is set to use hardware envelope, unit subdata is set one of the 2 types of data:

- If bit 16 is 0, it is raw data. Bit 0-15 is hardware envelope period.
- If bit 16 is 1, it is tone/hard ratio. Bit 0-7 is hard part and bit 8-15 is tone part.

---

## History
| Version | Date       | Detail                                                                        |
| ------- | ---------- | ----------------------------------------------------------------------------- |
| 1.5.0   | 2021-06-13 | Added FM/ADPCM panning sequence, drumkit panning, and removed unused subdata of ADPCM envelope. |
| 1.4.0   | 2020-04-28 | Added ADPCM drumkit instrument.                                               |
| 1.3.0   | 2020-02-25 | Added ADPCM instrument.                                                       |
| 1.2.3   | 2019-12-16 | Revised to fix the deep copy of instrument sequence types.                    |
| 1.2.2   | 2019-11-09 | Reversed SSG noise pitch order.                                               |
| 1.2.1   | 2019-06-07 | Revised to fix unit data skipping bug of FM operator sequence.                |
| 1.2.0   | 2019-04-10 | Added and changed for SSG tone/hard or square-mask ratio settings.            |
| 1.1.0   | 2019-03-24 | Added fields for FM3ch expanded mode.                                         |
| 1.0.1   | 2018-12-10 | Added instrument sequence type.                                               |
| 1.0.0   | 2018-11-23 | Initial release.                                                              |
