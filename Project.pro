TEMPLATE = subdirs
SUBDIRS = BambooTracker \
          emu2149 \
          RtAudio \
          RtMidi \
          data

emu2149.file = submodules/emu2149/emu2149.pro
RtAudio.file = submodules/RtAudio/RtAudio.pro
RtMidi.file = submodules/RtMidi/RtMidi.pro
BambooTracker.depends = emu2149 RtAudio RtMidi
