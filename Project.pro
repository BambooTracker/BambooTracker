TEMPLATE = subdirs
SUBDIRS = BambooTracker \
          RtAudio \
          RtMidi \
          data

RtAudio.file = submodules/RtAudio/RtAudio.pro
RtMidi.file = submodules/RtMidi/RtMidi.pro
BambooTracker.depends = RtAudio RtMidi
