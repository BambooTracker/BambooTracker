# Significant Changes

## Related with Module Playback

The following changes affect module playback critically.
In some cases, you need to modify modules.

### v0.6.0

- When the instrument sequence type of an arpeggio is "Fixed", the original pitch of the note on is restored at the tick immediately after the last data in the sequence is executed. Data created before v0.5.3 is automatically modified when the file is loaded to maintain compatibility.
- A bug is fixed that sequence macro not running when echo buffer is empty and note is on. Also, redundant volume changes in the volume column and envelope macros have been removed.

### v0.5.1

- A bug has been fixed that caused sequence properties such as envelope sequence to skip the first count when a note is turned on by the note delay effect. The behavior of sequence properties for modules created in the past may change.

### v0.5.0

- A bug for tempo and step size control is fixed that only a row which set the step size change 0Fxx had the step size added by one. To play modules made by previous version at the same speed as before, it is necessary to replace 0Fxx with 0Fxx+1, and set 0Fxx in the next row.
- MAME FM, SSG, and ADPCM emulations are updated which make several differences in the synthesis accuracy from previous versions.
- The number of buffer in RtAudio is changed to 2. This may change the latency.

### v0.3.6

- Hard envelope settings of SSG is fixed when note off.
- Effects in columns are executed from left to right. In previous versions there was a read order ambiguity.

### v0.3.5

- Pitch calculation of AutoEnv and square mask is fixed. When you used hardware envelope and square mask in a module created in an earlier version, you may need to modify its frequency.

### v0.3.1

- A bug of groove start position is fixed that happened when setting groove by groove set pattern effect.

### v0.3.0

- Before this version, square-masked saw and square-masked inversed saw were not actually square-masked.

### v0.2.1

- The start position of effect sequence when declaring sequence type effect after key on was changed from 2nd to 1st.
- The execution routine of pattern effects is revised. Some effects (especially delay effects) may behave differently than before.

### v0.2.0

- A bug is fixed that increases tone by octave when changing from SMTri (previous: Tri w) to Saw in SSG waveform sequence.

### v0.1.5

- A bug to set wrong value of FM envelope in instrument event is fixed. You would need to change TL3 value if you use AL = 0-4 to play module made by before v0.1.4 correctly.

### v0.1.4

- FM octave range becomes lower. However the tracker automatically correct song octaves whose module version is before 0.1.2, old octave 7 cannot be played.

### v0.1.3

- FM synthesis frequency is changed from 110933Hz to 55466Hz.

## Related with File I/O

And the found critical bugs about file I/O.

### v0.3.3

- Files made by BambooTracker before v0.3.3 (Module: &lt;v1.3.2, Instrument: &lt;v1.2.3, Bank: &lt;v1.0.2), deep-cloned instrument sequence type of FM/SSG arpeggio and pitch might be saved as an incorrect value. The later version tracker loads these files by replacing an incorrect value with "Absolute" type, so you may need to modify appropriate sequence types after loading old files.

### v0.2.1

- In BambooTracker v0.2.0, there is the bug that unit subdata of FM operator sequence is not omitted when saving a module and an instrument. To read them made by v0.2.0 and used operator sequence, please use the fixed version tracker.
