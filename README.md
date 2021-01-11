# ![icon](./img/icon.png) BambooTracker
[![GitHub release](https://img.shields.io/badge/release-v0.4.5-brightgreen.svg)](https://github.com/rerrahkr/BambooTracker/releases)
![Platforms: windows | macos | linux](https://img.shields.io/badge/platforms-windows%20|%20macos%20|%20linux-lightgrey.svg)
[![LICENSE](https://img.shields.io/github/license/rerrahkr/BambooTracker.svg)](./LICENSE)

[![Windows](https://github.com/rerrahkr/BambooTracker/workflows/Windows%20(7%20and%20up)/badge.svg)](https://github.com/rerrahkr/BambooTracker/actions?query=workflow%3A%22Windows+%287+and+up%29%22)
[![macOS](https://github.com/rerrahkr/BambooTracker/workflows/macOS/badge.svg)](https://github.com/rerrahkr/BambooTracker/actions?query=workflow%3AmacOS)
[![Ubuntu 16.04](https://github.com/rerrahkr/BambooTracker/workflows/Linux%20(Ubuntu%2016.04)/badge.svg)](https://github.com/rerrahkr/BambooTracker/actions?query=workflow%3A%22Linux+%28Ubuntu+16.04%29%22)
[![Nixpkgs 20.09](https://github.com/rerrahkr/BambooTracker/workflows/Linux%20(Nixpkgs)/badge.svg)](https://github.com/rerrahkr/BambooTracker/actions?query=workflow%3A%22Linux+%28Nixpkgs%29%22)

[![Development builds (Appveyor)](https://ci.appveyor.com/api/projects/status/jgg75iyduc1ij7ew?svg=true)](https://ci.appveyor.com/project/rerrahkr/bambootracker)

![Example](./img/overview.gif)

BambooTracker is a music tracker for the Yamaha YM2608 (OPNA) sound chip which was used in NEC PC-8801/9801 series computers.

[日本語](./README_ja.md)

## Table of Contents
1. [Downloads](#Downloads)
2. [Wiki](#Wiki)
3. [Discord](#Discord)
4. [Key commands](#Key-commands)
5. [Effect list](#Effect-list)
6. [File I/O](#File-I/O)
7. [Language](#Language)
8. [Building](#Building)
9. [Packaging status](#Packaging-status)
10. [Changelog](#Changelog)
11. [License](#License)
12. [Credits](#Credits)

## Downloads <a name="Downloads"></a>
### Windows 
- <https://github.com/rerrahkr/BambooTracker/releases>
- *Development builds*: get "artifacts" from [Appveyor](https://ci.appveyor.com/project/rerrahkr/bambootracker)

### macOS 
#### GitHub Builds
- <https://github.com/rerrahkr/BambooTracker/releases>
- *Development builds*: get "artifacts" from [Appveyor](https://ci.appveyor.com/project/rerrahkr/bambootracker)

#### Nixpkgs
Alternatively, you can install the latest BambooTracker release via the [Nix package manager](https://nixos.org/) (You might need to update your nixpkgs-unstable channel first):

`nix-env -iA nixpkgs.bambootracker`

### Linux / BSD 
#### Debian / Ubuntu
`apt install bambootracker`

#### FreeBSD
`pkg install bambootracker`

#### Cross-Distribution
##### Nixpkgs
(After updating your nixpkgs-unstable channel):

`nix-env -iA nixpkgs.bambootracker`

#### Other
- See chapter "Building"

## Wiki <a name="Wiki"></a>
Wiki explains the BambooTracker interface etc.

- [BambooTracker Wiki (GitHub Wiki)](https://github.com/rerrahkr/BambooTracker/wiki)

## Discord <a name="Discord"></a>
We can chat, help and listen tunes on Discord.

- [Official Discord Server](https://discord.gg/gBscTMF)

## Key commands <a name="Key-commands"></a>

Please refer to [this](./KEYCOMMANDS.md) list.

## Effect list <a name="Effect-list"></a>

Please refer to [this](./EFFECTLIST.md) list.

## File I/O <a name="File-I/O"></a>
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

It also supports loading FM envelopes in plain text formats.  
ADPCM waveform editor supports .wav import (16-bit mono 2k-55.5kHz).

An instrument is saved as a .bti file.

### Bank
The tracker can load bank from the following files.

- .btb (BambooTracker bank file)
- .wopn (WOPN bank file)
- .ff (PMD FF file)
- .ppc (PMD PPC file)
- .pvi (FMP PVI file)
- .dat (MUCOM88 voice file)

A bank is saved as a .btb file.

### Export
The tracker can export a song to the following files:

- .wav (WAVE file)
- .vgm (VGM file)
- .s98 (S98 file)

## Language <a name="Language"></a>
BambooTracker supports following languages:

- English (default)
- Français / French
- Polski / Polish
- 日本語 / Japanese

## Building <a name="Building"></a>
### Dependencies
To build BambooTracker, you'll need the following required dependencies:

- A C++ compiler (GCC/Clang/...)
- make
- Qt5 Base
- Qt5 Multimedia
- Qt5 Multimedia plugins
- Qt5 Tools (qmake, lrelease, ...)

The way of acquiring all these will depend on your OS & distribution.

#### Windows
**TODO**

- MinGW / MSVC
- Qt5

#### macOS
You'll most likely need to install the Xcode Command Line Tools - how to acquire these might depend on your macOS version.

For Qt5, [check out Homebrew](https://formulae.brew.sh/formula/qt).

The following optional dependency exists:

- **JACK Support**: JACK headers & libraries (for example [through Homebrew](https://formulae.brew.sh/formula/jack))

#### Linux / BSD
You should usually be able to install all required dependencies through your distribution's package manager.

To build BambooTracker, you'll additionally need:

- ALSA headers & libraries

The following optional dependencies exist:

- **PulseAudio Support**: PulseAudio headers & libraries
- **JACK Support**: JACK headers & libraries

##### Debian / Ubuntu:
```bash
apt install \
  build-essential \
  qt5-default qttools5-dev-tools \
  libasound2-dev \
  libpulse-dev \
  librtaudio-dev \ #optionally
  librtmidi-dev \ #optionally
  libjack-jackd2-dev # for JACK2, or libjack-dev for JACK1
```

##### Arch Linux/Manjaro/Artix:
```bash
pacman -S \
  qt5-tools \
  pulseaudio \
  jack \ 
  rtaudio \ #optionally
  rtmidi \ #optionally
  alsa-plugins #uncertain about this one though
```

##### FreeBSD
You may skip to "Compilation" - "Linux / BSD" - "FreeBSD" if you want to build via FreeBSD ports.

### Compilation

#### Windows
**TODO**

- if using MinGW, `mingw32-make` / `mingw64-make` instead of `make`
- if using MSVC *?*

When building on Windows, it is recommended to use Qt Creator due to ease and simplicity.

#### macOS
If you installed JACK via Homebrew, You might have to manually tell `qmake` where to find the JACK libraries & includes by appending the following:
`LIBS+=-L/usr/local/opt/jack/lib INCLUDEPATH+=/usr/local/opt/jack/include`

If you want to build with **JACK Support**, append the following option to `qmake`:
`CONFIG+=use_jack CONFIG+=jack_has_rename`  
If you know that building with an old version of JACK that lacks the `jack_port_rename` method or you encounter build problems like

> error: ‘jack_port_rename’ was not declared in this scope

, try dropping `CONFIG+=jack_has_rename`.

#### Linux / BSD
```
git clone https://github.com/rerrahkr/BambooTracker 
cd BambooTracker
mkdir build
export BT_OPTIONS="CONFIG+=release CONFIG-=debug PREFIX=$PWD/build"
```
If using vendored Rt libraries, initialise the submodules:
```
git submodule init
git submodule update
```
... else specify which Rt libraries should be looked up with pkg-config
`export BT_OPTIONS="$BT_OPTIONS CONFIG+=system_rtaudio CONFIG+=system_rtmidi"`

If you want to build with any of the optional dependencies, append the following options to `export`:

- **PulseAudio Support**: `export BT_OPTIONS="$BT_OPTIONS CONFIG+=use_pulse"`
- **JACK Support**: `export BT_OPTIONS="$BT_OPTIONS CONFIG+=use_jack"`
- **OSS4 Support**: `export BT_OPTIONS="$BT_OPTIONS CONFIG+=use_oss"`
- **Forego some Unixoid-prefered files and Linux FHS paths during installation**: `export BT_OPTIONS="$BT_OPTIONS CONFIG+=install_flat"`
- **Skip installation of non-essential files.**: `export BT_OPTIONS="$BT_OPTIONS CONFIG+=install_minimal"`

Then configure (don't cd into BambooTracker!)
```
qmake Project.pro $BT_OPTIONS
make qmake_all  
```
Finally, build & install to local prefix
`make -j4 && make install`


##### FreeBSD
BambooTracker can be built via FreeBSD ports instead:

```bash
cd /usr/ports/audio/bambootracker
make install clean
```

### Installing
The base files (executable + i18n) can be installed into your system like this:

```bash
make install
```

## Packaging status <a name="Packaging-status"></a>

[![Packaging status](https://repology.org/badge/vertical-allrepos/bambootracker.svg)](https://repology.org/project/bambootracker/versions)

For miscellaneous files like the example demos, skins, license informations etc, you'll have to copy them to the appropriate directories yourself.

## Changelog <a name="Changelog"></a>
*See [CHANGELOG.md](./CHANGELOG.md).*

## License <a name="License"></a>
This program and its source code are licensed under the GNU General License Version 2.

*See [LICENSE](./LICENSE) and [list.md](./licenses/list.md) for details.*

## Credits <a name="Credits"></a>
I would like to thank the following people for making it:

- Qt team for Qt framework
- MAME team, and Valley Bell for the codes of YM2608 chip emulation
- Decidetto for the application icon
- Mark James for Silk icons used for some other icons
- VGMRips team for the VGM file format documentation
- Some tracker creators, especially HertzDevil of 0CC-FamiTracker, Leonardo Demartino (delek) of Deflemask Tracker and Lasse Öörni (Cadaver) of GoatTracker for UI and routines of the tracker
- Vitaly Novichkov (Wohlstand) for WOPN instrument format files
- maak, SuperJet Spade, Dippy, RigidatoMS and ImATrackMan for sample modules
- papiezak and Takeshi Abo for sample instruments
- Yuzu4K for sample skin
- Ru^3 for S98 file format documentation
- がし３ (gasshi) for SCCI libraries
- honet for C86CTL libraries
- Gary P. Scavone and others for RtAudio and RtMidi libraries
- Thanks Alexey Khokholov (Nuke.YKT) and Jean Pierre Cimalando for Nuked OPN-Mod emulation
- Jean Pierre Cimalando and Olivier Humbert for French translation
- freq-mod and Midori for Polish translation
- Ian Karlsson for ADPCM encoder/decoder
- And everyone who helps this project!
