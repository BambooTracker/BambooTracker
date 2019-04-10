# Significant changes
The following changes affect module playback critically.
In some cases, you need to modify modules.

- Since BambooTracker v0.2.0, a bug is fixed that increases tone by octave when changing from SMTri (previous: Tri w) to Saw in SSG waveform sequence.
- Since BambooTracker v0.1.5, a bug to set wrong value of FM envelope in instrument event is fixed. You would need to change TL3 value if you use AL = 0-4 to play module made by before v0.1.4 correctly.
- Since BambooTracker v0.1.4, FM octave range lower. However the tracker automatically correct song octaves whose module version is before 1.0.2, old octave 7 cannot be played.
- Since BambooTracker v0.1.3, FM synthesis frequency is changed from 110933Hz to 55466Hz.
