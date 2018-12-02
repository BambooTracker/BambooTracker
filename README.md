# BambooTracker
[![GitHub release](https://img.shields.io/badge/release-v0.1.2-orange.svg)](https://github.com/rerrahkr/BambooTracker/releases)
![Platform: win-32](https://img.shields.io/badge/platform-win--32-lightgrey.svg)
[![GitHub issues](https://img.shields.io/github/issues/rerrahkr/BambooTracker.svg)](https://github.com/rerrahkr/BambooTracker/issues)
[![GitHub](https://img.shields.io/github/license/rerrahkr/BambooTracker.svg)](./LICENSE)

BambooTracker is a tracker for YM2608 (OPNA) which was used in NEC PC-8801/9801 series computers.

## Key commands
### General
| Key    | Command                        |
| ------ | ------------------------------ |
| Ctrl+N | Create new module              |
| Ctrl+O | Open module                    |
| Ctrl+S | Save module                    |
| Ctrl+P | Open module property dialog    |
| Ctrl+I | Open current instrument editor |
| Return | Play/stop song                 |
| F5     | Play from start                |
| F6     | Play pattern                   |
| F7     | Play from current position     |
| F8     | Stop song                      |
| Space  | Toggle jam/edit mode           |
| Alt+O  | Focus on order list            |
| Alt+P  | Focus on pattern editor        |
| F12    | Kill sound                     |

### Order list
| Key      | Command             |
| -------- | ------------------- |
| Ctrl+C   | Copy                |
| Ctrl+V   | Paste               |
| Ctrl+A   | Select track/all    |
| Ctrl+D   | Duplicate order     |
| Alt+D    | Clone order         |
| Home     | Jump to first order |
| End      | Jump to last order  |
| PageUp   | Jump to upper oder  |
| PageDown | Jump to lower oder  |
| Insert   | Insert order below  |
| Delete   | Delete order        |
| Escape   | Deselect            |

### Pattern editor
| Key       | Command                            |
| --------- | ---------------------------------- |
| Ctrl+C    | Copy                               |
| Ctrl+X    | Cut                                |
| Ctrl+V    | Paste                              |
| Ctrl+M    | Paste and mix                      |
| Ctrl+A    | Select track/all                   |
| Ctrl+G    | Interpolate                        |
| Ctrl+R    | Reverse                            |
| Ctrl+F1   | Decrease note                      |
| Ctrl+F2   | Increase note                      |
| Ctrl+F3   | Decrease octave                    |
| Ctrl+F4   | Increase octave                    |
| Alt+F9    | Mute track                         |
| Alt+F10   | Solo track                         |
| Alt+S     | Replace instrument                 |
| Tab       | Jump to right track                |
| BackTab   | Jump to left track                 |
| Home      | Jump to first step                 |
| End       | Jump to last step                  |
| PageUp    | Jump to upper step                 |
| PageDown  | Jump to lower step                 |
| Insert    | Insert step                        |
| BackSpace | Delete the step above              |
| Delete    | Delete commands                    |
| Escape    | Deselection                        |
| -         | Key off                            |
| \*        | Increase octave/echo buffer number |
| /         | Decrease octave/echo buffer number |
| ^         | Echo buffer access                 |

There are two rows of a piano keyboard:

```
Current octave+1
 2 3  5 6 7  9
Q W ER T Y UI O

Current octave
 S D  G H J  L
Z X CV B N M, .
```

## Volume range
|      | Minimum | Maximum |
| ---- | ------- | ------- |
| FM   | 7F      | 00      |
| SSG  | 00      | 0F      |
| Drum | 00      | 1F      |

It is able to reverse the order of FM volume (Configuration -> General -> Reverse FM volume order).

## Effect list
| Effect | FM                                                                                      | PSG                | Drum                              |
| ------ | --------------------------------------------------------------------------------------- | ------------------ | --------------------------------- |
| 00xy   | Arpeggio (x[0-f]: 2nd note, y[0-f]: 3rd note)                                           | Arpeggio           | -                                 |
| 01xx   | Portamento up (xx[00-ff]: depth)                                                        | Portamento up      | -                                 |
| 02xx   | Portamento down (xx[00-ff]: depth)                                                      | Portamento down    | -                                 |
| 03xx   | Tone portamento (xx[00-ff]: depth)                                                      | Tone portamento    | -                                 |
| 04xy   | Vibrato (x[0-f]: period, y[0-f]: depth)                                                 | Vibrato            | -                                 |
| 07xy   | Tremolo (x[0-f]: period, y[0-f]: depth)                                                 | Tremolo            | -                                 |
| 08xx   | Pan (xx: 00=No sound, 01=Right, 02=Left, 03=Center)                                     | -                  | Pan                               |
| 0A0x   | Volume slide down (x[0-f]: depth)                                                       | Volume slide down  | -                                 |
| 0Ax0   | Volume slide up (x[0-f]: depth)                                                         | Volume slide up    | -                                 |
| 0Bxx   | Position jump (xx: order)                                                               | Position jump      | Position jump                     |
| 0Cxx   | Track end (xx: any number)                                                              | Track end          | Track end                         |
| 0Dxx   | Pattern break (xx: start step on the next order)                                        | Pattern break      | Pattern break                     |
| 0Fxx   | Speed/tempo change (xx[00-1f]: speed, [20-ff]: tempo)                                   | Speed/tempo change | Speed/tempo change                |
| 0Gxx   | Note delay (xx[00-ff]: count)                                                           | Note delay         | Note delay                        |
| 0Oxx   | Groove (xx[00-ff]: number)                                                              | Groove             | Groove                            |
| 0Pxx   | Detune (xx[00-ff]: pitch (xx-80))                                                       | Detune             | -                                 |
| 0Qxy   | Note slide up (x[0-f]: speed, y[0-f]: seminote)                                         | Note slide up      | -                                 |
| 0Rxy   | Note slide down (x[0-f]: speed, y[0-f]: seminote)                                       | Note slide down    | -                                 |
| 0Sxx   | Note cut (xx[01-ff]: count)                                                             | Note cut           | Note cut                          |
| 0Txy   | Transpose delay (x[1-7]: upwards count, [9-f]: downwards count (x-8), y[0-f]: seminote) | Transpose delay    | -                                 |
| 0Vxx   | -                                                                                       | -                  | Master volume (xx[00-3f]: volume) |
| Mxyy   | Volume delay (x[1-f]: count, yy[00-ff]: volume)                                         | Volume delay       | Volume delay                      |

## Build on Linux
On Ubuntu 18.04:

### Dependencies
> make  
> Qt5 (including qmake)  
> Qt5 Multimedia  
> Qt5 Multimedia plugins

```bash
sudo apt-get install \
  build-essential \
  qt5-default qtmultimedia5-dev libqt5multimedia5-plugins
```

### Compilation
```bash
cd BambooTracker
qmake
make
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
- VGMRips team for the VGM file format documentation.
- Some tracker creators, especially HertzDevil of 0CC-FamiTracker, Leonardo Demartino (delek) of Deflemask Tracker and Lasse Öörni (Cadaver) of GoatTracker for UI and routines of the tracker.
