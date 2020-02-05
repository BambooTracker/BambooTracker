# Significant changes
The following changes affect module playback critically.
In some cases, you need to modify modules.

- In BambooTracker v0.3.5, pitch calculation of AutoEnv and square mask is fixed. When you used hardware envelope and square mask in a module created in an earlier version, you may need to modify its frequency.
- In BambooTracker v0.3.1, the bug of groove start position is fixed that happened when setting groove by groove set pattern effect.
- Before BambooTracker v0.3.0, square-masked saw and square-masked inversed saw were not actually square-masked.
- In BambooTracker v0.2.1, the start position of effect sequence when declaring sequence type effect after key on was changed from 2nd to 1st.
- In BambooTracker v0.2.1, execution routine of pattern effects is revised. Some effects (especially delay effects) may behave differently than before.
- Since BambooTracker v0.2.0, a bug is fixed that increases tone by octave when changing from SMTri (previous: Tri w) to Saw in SSG waveform sequence.
- Since BambooTracker v0.1.5, a bug to set wrong value of FM envelope in instrument event is fixed. You would need to change TL3 value if you use AL = 0-4 to play module made by before v0.1.4 correctly.
- Since BambooTracker v0.1.4, FM octave range lower. However the tracker automatically correct song octaves whose module version is before 1.0.2, old octave 7 cannot be played.
- Since BambooTracker v0.1.3, FM synthesis frequency is changed from 110933Hz to 55466Hz.

And the found critical bugs about file I/O.

- Files made by BambooTracker before v0.3.3 (Module: &lt;v1.3.2, Instrument: &lt;v1.2.3, Bank: &lt;v1.0.2), deep-cloned instrument sequence type of FM/SSG arpeggio and pitch might be saved as an incorrect value. The later version tracker loads these files by replacing an incorrect value with "Absolute" type, so you may need to modify appropriate sequence types after loading old files.
- In BambooTracker v0.2.0, there is the bug that unit subdata of FM operator sequence is not omitted when saving a module and an instrument. To read them made by v0.2.0 and used operator sequence, please use the fixed version tracker.
