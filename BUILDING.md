*(these instructions are heavily outdated, but the general gist of things should hopefully still be roughly the same)*

# Building

## Dependencies

To build BambooTracker, you'll need the following dependencies across all platforms:

- Qt (5.5 or higher)
- Qt Tools (qmake, lrelease, ...)
- A Qt-supported C++ compiler (GCC, Clang, MSVC, ...)
- make

How to acquire these and further required / optional dependencies are specific to your OS (distribution).

### Windows

- C++ compiler: You can use either of these options
  - MinGW  
    Should be a bundled option in the Qt installer.  
    Otherwise check the Qt docs for your Qt version's compatible GCC version.
  - Clang  
    Untested but might work.
  - MSVC  
    Either bundled with [Visual Studio](https://visualstudio.microsoft.com) or standalone (check "Build Tools for Visual Studio 2019" download).
- [Qt](https://www.qt.io/download-qt-installer):
  Older Qt versions (e.g. for an XP builds) may require you to checkout & compile Qt yourself, official prebuilt versions were removed by Qt.

### macOS

For simplicity, these instructions assume the use of [Homebrew](https://brew.sh/) & Qt5.

- C++ compiler: Xcode Command Line Tools  
  The challenge of acquiring these is left to the reader.  
  (The developer writing this doesn't use macOS, try looking on the web for a guide perhaps)
- Qt5:  

```bash
brew install qt5
```

**Optional dependencies**:

- JACK: Support for connecting to a JACK server in vendored RtAudio / RtMidi.  
  Requires JACK (version 1 or 2) headers & libraries.  

  ```bash
  brew install jack
  ```

  Optional: pkg-config, can be skipped but may make finding JACK headers & libraries harder.  

  ```bash
  brew install pkg-config
  ```

- RtAudio / RtMidi: Use a precompiled RtAudio / RtMidi installation instead of our vendored one.  
  (Obsoletes the optional dependencies above)  
  Requires RtAudio (5.1.0 or higher) and/or RtMidi (4.0.0 or higher) + pkg-config.

  ```bash
  brew install rt-audio rt-midi pkg-config
  ```

### Linux

Installing dependencies highly depends on your distribution and package manager of choice, search the subsections below for your distribution to find a tested set of install instructions.

**Additional dependencies**:

- ALSA: Required for minimal audio & MIDI support.  
  Requires ALSA headers & libraries.

**Optional dependencies**:

- PulseAudio: Support for connecting to a PulseAudio server in vendored RtAudio.  
  Requires PulseAudio headers & libraries.  
  Optional: pkg-config, can be skipped but may make finding PulseAudio headers & libraries harder.
- JACK: Support for connecting to a JACK server in vendored RtAudio / RtMidi.  
  Requires JACK (version 1 or 2) headers & libraries.  
  Optional: pkg-config, can be skipped but may make finding JACK headers & libraries harder.
- RtAudio / RtMidi: Use a precompiled RtAudio / RtMidi installation instead of our vendored one.  
  (Obsoletes the optional dependencies above)  
  Requires RtAudio (5.1.0 or higher) and/or RtMidi (4.0.0 or higher) + pkg-config.

#### Debian / Ubuntu

```bash
# Optional dependencies
OPTIONALDEPS=""

# PulseAudio
OPTIONALDEPS="$OPTIONALDEPS libpulse-dev"

# Either:
# JACK 1
OPTIONALDEPS="$OPTIONALDEPS libjack-dev"
# JACK 2
OPTIONALDEPS="$OPTIONALDEPS libjack-jackd2-dev"

# Either / Both:
# System-RtAudio
OPTIONALDEPS="$OPTIONALDEPS librtaudio-dev"
# System-RtMidi
OPTIONALDEPS="$OPTIONALDEPS librtmidi-dev"

# Install dependencies
apt install \
  build-essential \
  qt5-default qttools5-dev-tools \
  libasound2-dev \
  $OPTIONALDEPS
```

#### Arch Linux / Manjaro

```bash
# Optional dependencies
OPTIONALDEPS=""

# PulseAudio
OPTIONALDEPS="$OPTIONALDEPS libpulse"

# Either:
# JACK 1
OPTIONALDEPS="$OPTIONALDEPS jack"
# JACK 2
OPTIONALDEPS="$OPTIONALDEPS jack2"

# Either / Both:
# System-RtAudio
OPTIONALDEPS="$OPTIONALDEPS rtaudio"
# System-RtMidi
OPTIONALDEPS="$OPTIONALDEPS rtmidi"

# Install dependencies
pacman -S \
  qt5-tools \
  alsa-plugins \
  $OPTIONALDEPS
```

### BSD

Mostly like Linux:

- ALSA is optional
- OSS4 headers & libraries is required instead

If you managed to get BambooTracker manually compiled on a BSD system, feel free to open a Pull Request and add your dependency install instructions here!

## Compilation

These compilation instructions assume CLI usage, though it's possible to use an IDE like Qt Creator instead. They should generally work on all platforms, specific changes are mentioned in the subsections below.

Qmake options (flags/switches):

- PREFIX: Where to install the completed build.  
  Default:  
  - Windows: `C:\BambooTracker`
  - macOS/Linux/BSD: `/usr/local`  
  Example: `PREFIX=C:\Users\Owner\Programs\BambooTracker`
- CONFIG  
  Can be specified multiple times to add multiple options.  
  Use `+=` to add & `-=` to remove an option.
  - debug: A debug build, may be helpful for debugging crashes.
  - release: A release build, more optimised & slimmer than a debug one.
  - install_flat: Don't use Linux FHS paths under PREFIX during installation. (Default on Windows)
  - install_minimal: Skip installation of non-essential files.
  - use_alsa: Compile vendored RtAudio & RtMidi with explicit support for ALSA<sup>1</sup>. (Default on Linux)
  - use_oss: Compile with explicit support for OSS4<sup>2</sup>. (Default on BSD)
  - use_pulse: Compile vendored RtAudio with explicit support for PulseAudio<sup>1</sup>.
  - use_jack: Compile with explicit support for JACK 1/2<sup>13</sup>.
  - system_rtaudio: Skips RtAudio compilation and links against the system's RtAudio installation instead.
  - system_rtmidi: Skips RtMidi compilation and links against the system's RtMidi installation instead.
  - real_chip: Compile with support for SCCI and C86CTL interfaces to a real OPNA chip<sup>4</sup>. (Default on Windows)

<sup>1</sup>: Only works on Linux and BSD.  
<sup>2</sup>: Shipped & auto-detected(?) on FreeBSD but possibly not functional in RtAudio.  
<sup>3</sup>: API technically supported on Windows but unlikely to compile in Rt library.  
<sup>4</sup>: Builds code that doesn't use an OSI-certified license. Only supported on Windows.

Example:

```bash
# Where you want to locally keep the source code clone
cd /home/owner/devel

# Replace --recurse-submodules with --recurse-submodules=submodules/emu2149/src if CONFIG+=system_rtaudio AND CONFIG+=system_rtmidi will be used
git clone https://github.com/BambooTracker/BambooTracker --recurse-submodules
cd BambooTracker

# Configure
qmake PREFIX=/usr/local CONFIG-=debug CONFIG+=release CONFIG+=use_alsa CONFIG+=use_pulse CONFIG+=use_jack

# Recursively initialises subprojects properly, dependency tests may be flakey if run multithreaded
make qmake_all

# Compile with 4 threads
make -j4

# Installs to PREFIX
make install
```

### Windows

Enter a shell with Qt + your compiler set up before starting the above steps. (Qt installer adds scripts you can use for this)

- If you're using MinGW, use `mingw32-make` / `mingw64-make` instead of `make`.
- If you're using MSVC, **TODO**.

Qt Creator may be used for a more graphical compilation process instead.

### macOS

If you decided to use Homebrew's JACK without pkg-config, you may need to help the build process out by
passing `qmake` some additional arguments: `LIBS+=-L/usr/local/opt/jack/lib INCLUDEPATH+=/usr/local/opt/jack/include`.

### FreeBSD

BambooTracker can be built via FreeBSD ports instead:

```bash
cd /usr/ports/audio/bambootracker
make install clean
```
