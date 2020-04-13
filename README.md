# ![icon](./img/icon.png) BambooTracker
[![GitHub release](https://img.shields.io/badge/release-v0.4.0-brightgreen.svg)](https://github.com/rerrahkr/BambooTracker/releases)
![Platform: windows | macos | linux](https://img.shields.io/badge/platform-windows%20|%20macos%20|%20linux-lightgrey.svg)
[![Travis CI Build Status](https://travis-ci.com/rerrahkr/BambooTracker.svg?branch=master)](https://travis-ci.com/rerrahkr/BambooTracker)
[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/jgg75iyduc1ij7ew?svg=true)](https://ci.appveyor.com/project/rerrahkr/bambootracker)
[![LICENSE](https://img.shields.io/github/license/rerrahkr/BambooTracker.svg)](./LICENSE)

![Example](./img/overview.gif)

BambooTracker is a music tracker for the Yamaha YM2608 (OPNA) sound chip which was used in NEC PC-8801/9801 series computers.

[日本語](./README_ja.md)

## Downloads
**On Windows**:

- <https://github.com/rerrahkr/BambooTracker/releases>
- *Development builds*: get "artifacts" from [Appveyor](https://ci.appveyor.com/project/rerrahkr/bambootracker)

**On macOS**:

- <https://github.com/rerrahkr/BambooTracker/releases>

**On Linux**:

- See below chapters "Build on Linux" - "Install package on Debian or Ubuntu".

## Wiki
- [BambooTracker Wiki (GitHub Wiki)](https://github.com/rerrahkr/BambooTracker/wiki)

## Glossary
The files created by this tracker are called "modules". One such module contains songs (song data), instruments (tone data) and settings common to each song.
In a song, the channel of each sound source is assigned to a track, and the track holds multiple patterns (performance patterns).
The patterns are played by registering them in the order they appear the song. (from beginning to end)
A pattern describes a structure in which steps are arranged in chronological order. Key On / off and most effects are described in step units.
A tick is the minimum performance unit, one step = n ticks. The effects (such as vibrato) which change with the count unit are based on ticks.

## Interface Overview
### Instrument List
In the Instrument List, you control the instruments (tone data) used in the module. All songs in the module share the instruments registered here. Up to 128 instruments can be registered (`$00` - `$7F`).

### Instrument Editor
Double-clicking an instrument opens the Instrument Editor and allows you to edit said instrument.
Instruments can share their settings (properties) with other instruments. For instruments that share properties, a list of all users (instruments) of each property are displayed.

Some instrument editors can set performance sequences. One column corresponds to one tick in the sequence editor.
It also corresponds to specifying the sequence loop / release point. Left click to create point or increase count / type change, right click to delete point or decrease count. You can move the position by dragging the edge of the point.
For the release type, you can select from the following three types only for envelope setting of SSG.

- Fixed: Run at specified volume from release point at Key Off
- Absolute: Run from the first point after the release point to reach the volume at Key Off
- Relative: Execute from the release point with the volume set as the maximum volume at Key Off

#### FM Editor
In the FM Editor, you can configure the

1. Envelope
2. LFO
3. Operator Sequence
4. Arpeggio
5. Pitch

of an FM instrument.

It enables to paste plain text of envelope like MML. You can add and fix text formats in configuration. There are 8 default formats: FMP, FMP7, MMLDRV, MUCOM88, MXDRV, NRTDRV(`VOICE_MODE=0`), PMD, VOPM.  
Note the tracker reads digits from text in the order of appearance. Please remove needless digits contained in tone name or comments upon using default format.

#### SSG Editor
In the SSG Editor, you can configure the

1. Waveform
2. Tone/Noise
3. Envelope
4. Arpeggio
5. Pitch

of an SSG instrument.
The software envelope will be invalid while using waveforms other than rectangular waves.

### Order List
In the Order List, we will register the pattern numbers in the order in which they will be played. The rows correspond to the order, and the columns correspond to tracks.
The maximum length of the list is 256 (`$FF`).

### Pattern Editor
In the pattern Editor, events such as Key On are registered in chronological order. One line represents one step. The columns of each track, from left to right, are:

1.  Notes
2.  Instrument numbers
3.  Volume
4.  Two characters before effect 1
5.  Two characters after effect 1
6.  Two characters before effect 2
7.  Two characters after effect 2
8.  Two characters before effect 3
9.  Two characters after effect 3
10. Two characters before effect 4
11. Two characters after effect 4

Effects 2-3 can be displayed and hidden by pressing the + and - buttons on the header.
Up to 256 patterns (`$00` - `$FF`) can be created for each track.

### Settings Field
In the Settings Field you can adjust the module's metadata and performance settings of the song.

#### Tempo
Specify the tempo of the song. The tempo here is different from bpm, it is simply an indicator of speed.
It can also be specified with the `0Fxx` effect.

#### Speed
Set the number of ticks for rough estimate in 1 step. Tempo may cause this value to change at run time.
It can also be specified with the `0Fxx` effect.

#### Pattern Size
Sets the number of steps in the default pattern. The minimum value is 1, the maximum value is 256.

#### Groove
Specify the groove number set in the Groove Editor. When using groove, all other performance speed settings are fixed. 
It can also be specified with `0Oxx` effect.

## Key commands
Please replace some keys with the following in macOS:

- Ctrl → command
- Alt → option
- BackSpace → delete
- Delete → fn+delete

### General
| Key         | Command                     |
| ----------- | --------------------------- |
| Ctrl+N      | Create new module           |
| Ctrl+O      | Open module                 |
| Ctrl+S      | Save module                 |
| Ctrl+P      | Open module property dialog |
| Return      | Play/stop song              |
| Space       | Toggle jam/edit mode        |
| F1          | Show effect list dialog     |
| F2          | Focus on pattern editor     |
| F3          | Focus on order list         |
| F4          | Focus on instrument list    |
| F5          | Play from start             |
| F6          | Play pattern                |
| F7          | Play from current position  |
| Ctrl+F7     | Play from marker            |
| Ctrl+Return | Play only current step      |
| F8          | Stop song                   |
| F12         | Kill sound                  |

### Instrument list
| Key    | Command                        |
| ------ | ------------------------------ |
| Insert | Add instrument                 |
| Delete | Remove instrument              |
| Ctrl+I | Open current instrument editor |

### Order list
| Key             | Command             |
| --------------- | ------------------- |
| Ctrl+C          | Copy                |
| Ctrl+V          | Paste               |
| Ctrl+A          | Select track/all    |
| Ctrl+D          | Duplicate order     |
| Alt+D           | Clone patterns      |
| Home            | Jump to first order |
| End             | Jump to last order  |
| PageUp          | Jump to upper oder  |
| PageDown        | Jump to lower oder  |
| Insert or Alt+B | Insert order below  |
| Delete          | Delete order        |
| Escape          | Deselect            |

### Pattern editor
| Key                           | Command                                     |
| ----------------------------- | ------------------------------------------- |
| Ctrl+C                        | Copy                                        |
| Ctrl+X                        | Cut                                         |
| Ctrl+V                        | Paste                                       |
| Ctrl+M                        | Paste and mix                               |
| Ctrl+A                        | Select track/all                            |
| Ctrl+G                        | Interpolate                                 |
| Ctrl+R                        | Reverse                                     |
| Ctrl+F1 or Ctrl+scroll up     | Decrease note                               |
| Ctrl+F2 or Ctrl+scroll down   | Increase note                               |
| Ctrl+F3                       | Decrease octave                             |
| Ctrl+F4                       | Increase octave                             |
| Shift+F1 or Shift+scroll up   | Fine decrease values                        |
| Shift+F2 or Shift+scroll down | Fine increase values                        |
| Shift+F3                      | Coarse decrease values                      |
| Shift+F4                      | Coarse increase values                      |
| Alt+F9                        | Toggle track                                |
| Alt+F10                       | Solo track                                  |
| Alt+Left                      | Select previous instrument                  |
| Alt+Right                     | Select next instrument                      |
| Alt+S                         | Replace instrument                          |
| Alt+L                         | Expand effect column                        |
| Alt+K                         | Shrink effect column                        |
| Ctrl+K                        | Add/Remove bookmark on the current position |
| Ctrl+B                        | Set/Delete marker                           |
| Alt+G                         | Show Go To dialog                           |
| Tab                           | Jump to right track                         |
| BackTab                       | Jump to left track                          |
| Home                          | Jump to first step                          |
| End                           | Jump to last step                           |
| PageUp                        | Jump to upper step                          |
| PageDown                      | Jump to lower step                          |
| Ctrl+Up                       | Jump to upper 1st highlighted step          |
| Ctrl+Down                     | Jump to lower 1st highlighted step          |
| Ctrl+PageUp                   | Jump to previous bookmark                   |
| Ctrl+PageDown                 | Jump to next bookmark                       |
| Ctrl+Left                     | Go to previous order                        |
| Ctrl+Right                    | Go to next order                            |
| Insert or Alt+Down            | Insert step                                 |
| BackSpace or Alt+Up           | Delete the step above                       |
| Delete                        | Delete commands                             |
| Escape                        | Deselection                                 |
| -                             | Key off                                     |
| \* (numpad)                   | Increase octave/echo buffer number          |
| / (numpad)                    | Decrease octave/echo buffer number          |
| ^                             | Echo buffer access                          |

There are two rows of a piano keyboard:

```
Current octave+1
 2 3  5 6 7  9
Q W ER T Y UI O

Current octave
 S D  G H J  L
Z X CV B N M, .
```

The keyboard shortcuts and layout can be changed in the configuration.

## Volume range
|       | Minimum | Maximum |
| ----- | ------- | ------- |
| FM    | 7F      | 00      |
| SSG   | 00      | 0F      |
| Drums | 00      | 1F      |

It is able to reverse the order of FM volume (Configuration -> General -> Reverse FM volume order).

## Effect list
| Effect | FM                                                                                      | SSG                                                                | Drums                             | ADPCM                   |
| ------ | --------------------------------------------------------------------------------------- | ------------------------------------------------------------------ | --------------------------------- | ----------------------- |
| 00xy   | Arpeggio (x[0-F]: 2nd note, y[0-F]: 3rd note)                                           | Arpeggio                                                           | -                                 | Arpeggio                |
| 01xx   | Portamento up (xx[00-FF]: depth)                                                        | Portamento up                                                      | -                                 | Portamento up           |
| 02xx   | Portamento down (xx[00-FF]: depth)                                                      | Portamento down                                                    | -                                 | Portamento down         |
| 03xx   | Tone portamento (xx[00-FF]: depth)                                                      | Tone portamento                                                    | -                                 | Tone portamento         |
| 04xy   | Vibrato (x[0-F]: period, y[0-F]: depth)                                                 | Vibrato                                                            | -                                 | Vibrato                 |
| 07xy   | Tremolo (x[0-F]: period, y[0-F]: depth)                                                 | Tremolo                                                            | -                                 | Tremolo                 |
| 08xx   | Pan (xx: 00=No sound, 01=Right, 02=Left, 03=Center)                                     | -                                                                  | Pan                               | Pan                     |
| 0A0x   | Volume slide down (x[0-F]: depth)                                                       | Volume slide down                                                  | -                                 | Volume slide down       |
| 0Ax0   | Volume slide up (x[0-F]: depth)                                                         | Volume slide up                                                    | -                                 | Volume slide up         |
| 0Bxx   | Position jump (xx: order)                                                               | Position jump                                                      | Position jump                     | Position jump           |
| 0Cxx   | Song end (xx: any number)                                                               | Song end                                                           | Song end                          | Song end                |
| 0Dxx   | Pattern break (xx: start step on the next order)                                        | Pattern break                                                      | Pattern break                     | Pattern break           |
| 0Fxx   | Speed/Tempo change (xx[00-1F]: speed, [20-FF]: tempo)                                   | Speed/Tempo change                                                 | Speed/Tempo change                | Speed/Tempo change      |
| 0Gxx   | Note delay (xx[00-FF]: count)                                                           | Note delay                                                         | Note delay                        | Note delay              |
| 0Hxy   | -                                                                                       | Auto envelope (x[0-F]: shift amount (x-8), y[0-F]: shape)          | -                                 | -                       |
| 0Ixx   | -                                                                                       | Hardware envelope period 1 (xx[00-FF]: high byte)                  | -                                 | -                       |
| 0Jxx   | -                                                                                       | Hardware envelope period 2 (xx[00-FF]: low byte)                   | -                                 | -                       |
| 0Oxx   | Groove (xx[00-FF]: number)                                                              | Groove                                                             | Groove                            | Groove                  |
| 0Pxx   | Detune (xx[00-FF]: pitch (xx-80))                                                       | Detune                                                             | -                                 | Detune                  |
| 0Qxy   | Note slide up (x[0-F]: count, y[0-F]: seminote)                                         | Note slide up                                                      | -                                 | Note slide up           |
| 0Rxy   | Note slide down (x[0-F]: count, y[0-F]: seminote)                                       | Note slide down                                                    | -                                 | Note slide down         |
| 0Sxx   | Note cut (xx[01-FF]: count)                                                             | Note cut                                                           | Note cut                          | Note cut                |
| 0Txy   | Transpose delay (x[1-7]: upwards count, [9-F]: downwards count (x-8), y[0-F]: seminote) | Transpose delay                                                    | -                                 | Transpose delay         |
| 0Vxx   | -                                                                                       | Tone/Noise mix (xx: 00=No sound, 01=Tone, 02=Noise, 03=Tone&Noise) | Master volume (xx[00-3F]: volume) | -                       |
| 0Wxx   | -                                                                                       | Noise pitch (xx[00-1F]: pitch)                                     | -                                 | -                       |
| 0Xxx   | Register address bank 0 (xx[00-B6]: address)                                            | Register address bank 0                                            | Registe address bank 0            | Register address bank 0 |
| 0Yxx   | Register address bank 1 (xx[00-B6]: address)                                            | Register address bank 1                                            | Registe address bank 1            | Register address bank 1 |
| 0Zxx   | Regisetr value set (xx[00-FF]: value)                                                   | Register value set                                                 | Register value set                | Register value set      |
| Axyy   | AR control (x[1-4]: operator, y[00-1F]: attack rate)                                    | -                                                                  | -                                 | -                       |
| B0xx   | Brightness (xx[01-FF]: relative value (xx-80))                                          | -                                                                  | -                                 | -                       |
| Dxyy   | DR control (x[1-4]: operator, y[00-1F]: decay rate)                                     | -                                                                  | -                                 | -                       |
| FBxx   | FB control (xx[00-07]: feedback value)                                                  | -                                                                  | -                                 | -                       |
| Mxyy   | Volume delay (x[1-F]: count, yy[00-FF]: volume)                                         | Volume delay                                                       | Volume delay                      | Volume delay            |
| MLxy   | ML control (x[1-4]: operator, y[0-F]: multiple)                                         | -                                                                  | -                                 | -                       |
| RRxy   | RR control (x[1-4]: operator, y[0-F]: release rate)                                     | -                                                                  | -                                 | -                       |
| Txyy   | TL control (x[1-4]: operator, yy[00-7F]: total level)                                   | -                                                                  | -                                 | -                       |

## File I/O
### Module
The tracker enables to open and save to .btm (BambooTracker module file).

### Instrument
The tracker can load instrument from the following files.

- .bti (BambooTracker instrument file)
- .dmp (DefleMask preset file)
- .tfi (TFM Music Maker instrument file)
- .vgi (VGM Music Maker instrument file)
- .opni (WOPN instrument file)
- .y12 (Gens KMod dump file)
- .ins (MVSTracker instrument file)

It also supports loading plain text of FM envelope.  
ADPCM waveform editor supports .wav import (16-bit mono 2k-16kHz).

A instrument saves as .bti file.

### Bank
The tracker can load bank from the following files.

- .btb (BambooTracker bank file)
- .wopn (WOPN bank file)
- .ppc (PMD PPC file)
- .pvi (FMP PVI file)

A bank saves as .btb file.

### Export
The tracker can export a song to the following files.

- .wav (WAVE file)
- .vgm (VGM file)
- .s98 (S98 file)

## Language
BambooTracker supports following languages:

- English (default)
- French
- Japanese

## Build on Linux / BSD

### Debian / Ubuntu
`apt install bambootracker`

### FreeBSD
`pkg install bambootracker`

### Manual
#### Dependencies
To build BambooTracker, you'll need the following required dependencies:
- A C++ compiler (GCC/Clang/...)
- make 
- Qt5 Base
- Qt5 Multimedia
- Qt5 Multimedia plugins
- Qt5 Tools (qmake, lrelease etc)
- ALSA libraries

There are some optional dependencies:
- PulseAudio libraries
  for PulseAudio support
- JACK libraries
  for JACK support

The way of acquiring all these will depend on your distribution.

##### Debian / Ubuntu:
```bash
apt install \
  build-essential \
  qt5-default qtmultimedia5-dev libqt5multimedia5-plugins qttools5-dev-tools \
  libasound2-dev \
  libpulse-dev libjack-dev
```

#### Compilation
```bash
git clone https://github.com/rerrahkr/BambooTracker
cd BambooTracker
qmake CONFIG+=release
make
# For optionally installing into the system:
make install
```

If you want to build with PulseAudio or JACK support, append the following options to `qmake`:

- PulseAudio: `DEFINES+=__LINUX_PULSE__`
- JACK: `DEFINES+=__UNIX_JACK__`

##### FreeBSD
```bash
cd /usr/ports/audio/bambootracker
make install clean
```

## Changelog
*See [CHANGELOG.md](./CHANGELOG.md).*

## License
This program and its source code are licensed under the GNU General License Version 2.

*See [LICENSE](./LICENSE) and [list.md](./licenses/list.md) of libraries.*

## Credits
I would like to thank the following people for making it:

- Qt team for Qt framework
- MAME team, and Valley Bell for the codes of YM2608 chip emuration
- Mark James for the tool bar icons
- VGMRips team for the VGM file format documentation
- Some tracker creators, especially HertzDevil of 0CC-FamiTracker, Leonardo Demartino (delek) of Deflemask Tracker and Lasse Öörni (Cadaver) of GoatTracker for UI and routines of the tracker
- Decidetto for the application icon
- Vitaly Novichkov (Wohlstand) for WOPN instrument format files
- maak, SuperJet Spade, Dippy, RigidatoMS and ImATrackMan for sample modules
- papiezak for sample instruments
- Ru^3 for S98 file format documentation
- がし３ (gasshi) for SCCI libraries
- honet for C86CTL libraries
- Gary P. Scavone and others for RtAudio and RtMidi libraries
- Thanks Alexey Khokholov (Nuke.YKT) and Jean Pierre Cimalando for Nuked OPN-Mod emulation
- Jean Pierre Cimalando and Olivier Humbert for French translation
- Ian Karlsson for ADPCM encoder/decoder
- And everyone who helps this project!
