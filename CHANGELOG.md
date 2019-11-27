# Changelog

## Unreleased
### Added
- [#132] - 3 new pattern effects
  - `0Xxx`, `0Yxx`: register address set
  - `0Zxx`: register value set
  - `B0xx`: FM brightness ([#118]; thanks [@jpcima])
- Add 1 sample module (thanks [@maakmusic])
- Add selection whether auto-set current instrument upon entering note
- Add color scheme settings
- [#162] - Add order edit icons to the menu bar (thanks [@pedipanol])
- [#150] - Add the optimizing option to remove duplicate instruments (thanks [@papiezak])
- Add the icon of renaming instrument in the instrument list

### Changed
- Change width of items in status bar
- [#161] - Reduce drawing cost of the pattern editor and the order list (thanks [@papiezak])
- Change background color of odd columns in the instrument sequence editor
- Improve SSG tone/noise editor
- Change MML text in FM/SSG arpeggio editor from numbers to notes
- [#162] - Allow order list edit in jam mode (thanks [@pedipanol])
- [#162] - Reverse the input order of the order numbers (thanks [@pedipanol])
- Update translation: ja

### Fixed
- [#161] - Fix drawing error after loading song (thanks [@papiezak])
- Fix groove start position by `0Oxx` (thanks [@maakmusic])
- Fix some drawing errors in pattern editor and order list
- Fix corruption after changing song type
- [#163] - Fix the bug that the item is hidden in the instrument list (thanks [@galap-1])
- [#165] - Fix directive about std::hash with enum (thanks [@jpcima])
- [#166] - Fix uninitialized variables (thanks [@OPNA2608])
- [#167] - Fix the instrument list to disable the icons when it is empty (thanks [@OPNA2608])

[#161]: https://github.com/rerrahkr/BambooTracker/issues/161
[#162]: https://github.com/rerrahkr/BambooTracker/issues/162
[#163]: https://github.com/rerrahkr/BambooTracker/issues/163
[#150]: https://github.com/rerrahkr/BambooTracker/issues/150
[#118]: https://github.com/rerrahkr/BambooTracker/issues/118
[#165]: https://github.com/rerrahkr/BambooTracker/issues/165
[#166]: https://github.com/rerrahkr/BambooTracker/issues/166
[#167]: https://github.com/rerrahkr/BambooTracker/issues/167

## v0.3.0 (2019-10-25)
### Added
- [#96] - macOS support (thanks [@OPNA2608], [@jpcima] and others)
- [#132] - 11 new pattern effects
  - `0Hxy`: SSG auto envelope ([#3]; thanks [@marysiamzawka])
  - `0Ixx`, `0Jxx`: SSG hardware envelope period
  - `0Vxx`: SSG tone/noise mix (thanks [@pedipanol])
  - `0Wxx`: SSG noise pitch (thanks [@pedipanol])
  - `Axyy`: FM AR control
  - `Dxyy`: FM DR control (thanks [@papiezak])
  - `FBxx`: FM FB control
  - `MLxy`: FM ML control
  - `RRxy`: FM RR control
  - `Txyy`: FM TL control
- [#72] - Add module mixer settings (thanks [@ImATrackMan])
- Add sample rate selection in wav export
- Add shortcuts of expanding/shrinking effect column
- [#145] - Add configuration of font family and size of the pattern editor and the order list (thanks [@jimbo1qaz], [@papiezak])

### Changed
- [#152] - Use RtAudio for sound processing ([#96]; thanks [@jpcima], [@OPNA2608] and others)
- [#96], [#152], [#158] - Reduce drawing cost of pattern editor and order list (thanks [@OPNA2608], [@papiezak])
- Disable slider operation during playback
- [#156] - Change key-on/off of multiple drum instruments to write to register at once (thanks [@ImATrackMan], [@ValleyBell])
- Increase the maximum value of the master mixer to 200%
- Change 3 shortcuts
  - F2: Focus on pattern editor
  - F3: Focus on order list
  - F4: Focus on instrument list

### Fixed
- Fix to draw the playing cursor when follow mode is turned off
- [#152] - Prevent real step size setting to 0 (thanks [@OPNA2608])
- [#153] - Fix hardware envelope to set data in sequence (thanks [@papiezak])
- [#154] - Fix position initialization when opening module (thanks [@papiezak])
- Fix corruption after `0Bxx`, `0Cxx` and `0Dxx` are set the position before current playback row
- Fix incorrect square-mask pitch in square-masked saw and inversed saw
- Reset character entry position in pattern editor and order list after undoing
- Fix effect description in status bar after entering the 2nd character
- Reset playback position when it is out of song range by editing pattern during playback
- Restore FM RR parameters in key-on without instrument set after envelope reset
- [#159] - Fix corruption happened by toggling track during playback (thanks [@papiezak])

[#152]: https://github.com/rerrahkr/BambooTracker/pull/152
[#153]: https://github.com/rerrahkr/BambooTracker/issues/153
[#154]: https://github.com/rerrahkr/BambooTracker/issues/154
[#156]: https://github.com/rerrahkr/BambooTracker/issues/156
[#132]: https://github.com/rerrahkr/BambooTracker/issues/132
[#72]: https://github.com/rerrahkr/BambooTracker/issues/72
[#158]: https://github.com/rerrahkr/BambooTracker/issues/158
[#159]: https://github.com/rerrahkr/BambooTracker/issues/159
[#145]: https://github.com/rerrahkr/BambooTracker/issues/145

[@ValleyBell]: https://github.com/ValleyBell

## v0.2.4 (2019-09-17)
### Changed
- Finish value entry in the cell of pattern editor and order list when the cursor is moved
- Update translation: ja

### Fixed
- [#147], [#148] - Fix some typos (thanks [@alexmyczko])
- Ignore instrument addition when the instrument list has filled (thanks [@maakmusic])
- [#149] - Fix incorrect key off controls before key on (thanks [@CommodoreKulor])
- Fix size of track companding button in the pattern editor
- [#137] - Fix redo behavior of effect ID entry when only 1 character is enterd (thanks [@SMB7])
- [#151] - Fix some code (thanks [@jimbo1qaz])
- Reset cursor position after loading module, song and module properties

[@CommodoreKulor]: https://github.com/CommodoreKulor

[#147]: https://github.com/rerrahkr/BambooTracker/pull/147
[#148]: https://github.com/rerrahkr/BambooTracker/pull/148
[#149]: https://github.com/rerrahkr/BambooTracker/issues/149
[#151]: https://github.com/rerrahkr/BambooTracker/issues/151

## v0.2.3 (2019-08-31)
### Added
- Add the selection of effect value initialization upon entering effect id
- [#136] - Add instrument bank (thanks [@papiezak], [@jpcima])
- Keep selected file filter in instrument and bank import
- Load instrument and bank by drag and drop
- Add sample instruments (thanks [@papiezak])

### Fixed
- [#143] - Fix the corruption of note delay effect without note on (thanks [@ImATrackMan])
- [#146] - Fix the replay corruption after song-end effect (thanks [@papiezak])

[#143]: https://github.com/rerrahkr/BambooTracker/issues/143
[#136]: https://github.com/rerrahkr/BambooTracker/issues/136
[#146]: https://github.com/rerrahkr/BambooTracker/issues/146

## v0.2.2 (2019-06-25)
### Added
- [#141] - Add bank import and icons to instrument context menu (thanks [@jpcima])
- [#142] - Add search box for instrument selection dialog (thanks [@jpcima])

### Changed
- [#139] - Update translation: fr ([#122]; thanks [@jpcima], [@trebmuh])

### Fixed
- Fix SSG hardware envelope to reset on note on
- Fix that Drum master volume effect is not available
- [#137] - Fix corruption that occurred when using volume slide in FM3ch expansion (thanks [@SMB7])
- [#140] - Fix incorrect DT importing from DMP (thanks [@papiezak], [@jpcima], [@OPNA2608])
- [#138] - Fix that panning is reversed by insufficient stream buffer initialization (thanks [@jpcima])
- Fix the sequence start position when declaring sequence-type effect after key on
- Fix note slide effect to enable to execute when speed is 0
- Fix default save and export name when the name of module file contains multiple dots

[#137]: https://github.com/rerrahkr/BambooTracker/issues/137
[#140]: https://github.com/rerrahkr/BambooTracker/issues/140
[#139]: https://github.com/rerrahkr/BambooTracker/pull/139
[#141]: https://github.com/rerrahkr/BambooTracker/pull/141
[#138]: https://github.com/rerrahkr/BambooTracker/issues/138
[#142]: https://github.com/rerrahkr/BambooTracker/pull/142

[@SMB7]: https://github.com/SMB7

## v0.2.1 (2019-06-16)
### Added
- [#127] - Add effect display width to module (thanks [@ImATrackMan])
- Add 1 sample module (thanks [Dippy])

### Changed
- [#123], [#125] - Update translation: fr ([#122]; thanks [@trebmuh], [@jpcima])
- [#129] - Set the maximum count of FM channel to 6 in jamming during FX 3ch expanded mode (thanks [@galap-1])
- [#134] - Change delay effects to affect over step except note delay (thanks [@scarletbullgon])

### Fixed
- [#124] - Automatic section size for table column ([#122]; thanks [@jpcima], [@trebmuh])
- [#96] - Clean audio buffer initially (thanks [@OPNA2608])
- [#126] - Reset scrollbar positions in order list when loading song (thanks [@N-SPC700], [@OPNA2608])
- [#127] - Fix stream to change its interrupt rate when a module open (thanks [@ImATrackMan])
- [#128] - Fix that FM envelope paste was not worked (thanks [@papiezak])
- [#129] - Fix polyphonic jam-mode to accept unison when using MIDI keyboard (thanks [@galap-1])
- [#133] - Use Unicode paths ([#130]; thanks [@jpcima] and others)
- [#135] - Delete unit data saving of FM operator sequence (thanks [@elohimf], [@jpcima], [@OPNA2608])
- [#131] - Fix incorrect bit set for FM 3ch expanded mode (thanks [@nukeykt], [@jpcima])

[#122]: https://github.com/rerrahkr/BambooTracker/issues/122
[#123]: https://github.com/rerrahkr/BambooTracker/pull/123
[#125]: https://github.com/rerrahkr/BambooTracker/pull/125
[#124]: https://github.com/rerrahkr/BambooTracker/pull/124
[#96]: https://github.com/rerrahkr/BambooTracker/issues/96
[#126]: https://github.com/rerrahkr/BambooTracker/issues/126
[#127]: https://github.com/rerrahkr/BambooTracker/issues/127
[#128]: https://github.com/rerrahkr/BambooTracker/issues/128
[#129]: https://github.com/rerrahkr/BambooTracker/issues/129
[#130]: https://github.com/rerrahkr/BambooTracker/issues/130
[#133]: https://github.com/rerrahkr/BambooTracker/pull/133
[#135]: https://github.com/rerrahkr/BambooTracker/issues/135
[#131]: https://github.com/rerrahkr/BambooTracker/issues/131
[#134]: https://github.com/rerrahkr/BambooTracker/issues/134

[@N-SPC700]: https://github.com/N-SPC700
[Dippy]: https://www.youtube.com/channel/UCw2xCNQhuwpnfnf1-wfRefQ
[@elohimf]: https://github.com/elohimf
[@nukeykt]: https://github.com/nukeykt

## v0.2.0 (2019-04-30)
### Added
- [#76], [#107] - MIDI keyboard support ([#32]; thanks [@galap-1], [@jpcima], [@OPNA2608])
- Add edit/jam button in tool bar
- Add 1 sample module (thanks [@SuperJetSpade])
- Add 2nd pattern highlight
- [#79] - Add BPM indicator in status bar (thanks [@papiezak])
- Add key repeat settings for pattern editor
- [#62], [#84] - Add FM3ch expanded mode (thanks [@jimbo1qaz], [@OPNA2608])
- [#91] - Add module and instrument specification archive (thanks [@jpcima])
- [#88] - Add FM detune display mode selection (thanks [@bryc] and others)
- [#89] - Add custom note entry layout configuration ([#70]; thanks [@OPNA2608], [@jpcima])
- [#3] - Add tone/hard frequency ratio in SSG envelope edit and tone/square-mask frequency ratio in SSG waveform edit (thanks [@marysiamzawka])
- [#94], [#106] - Add Nuked OPN-Mod emulator ([#93]; thanks [@jpcima], [@papiezak])
- [#103], [#105], [#110] - Add oscilloscope ([#79]; thanks [@jpcima], [@papiezak])
- [#99], [#111] - Add VGM and S98 chip target selection ([#98]; thanks [@jpcima], [@pedipanol])
- Add default file name to save/export dialogs
- [#108] - Add macOS test to Travis CI (thanks [@OPNA2608], [@jpcima])
- Add resolution selection to S98 export settings
- [#112] - Add effect list dialog (thanks [@attilaM68K])
- [#79] - Add plain keyboard shortcut list dialog (thanks [@papiezak])

### Changed
- [#82] - Change default main window size to 900x700 (thanks [@KamuiKazuma])
- Change to keep current instrument when loading module
- Move tick frequency settings from module settings groupbox to module properties dialog
- [#119] - Change FM instrument editor scrollable (thanks [@Delta-Psi])
- Improve groove editor
- Change some layouts of UI
- [#121] - Update translation: fr, ja (thanks [@jpcima], [@trebmuh])

### Fixed
- [#80] - Fix a typo in BambooTracker.fr.1 (thanks [@trebmuh])
- [#81] - Fix crash when toggling FM1 on/off using mouse (thanks [@papiezak])
- Fix operator mask in restarting
- [#3] - Fix SSG square mask entry (thanks [@papiezak])
- [#85] - Fix Out-of-Path build for Qt version workaround ([#83]; thanks [@OPNA2608], [@jimbo1qaz])
- [#86] - Fix crash when changing pattern size during playback (thanks [@papiezak])
- Fix spinboxes to emit value change event when finishing text editing
- [#88] - Fix incorrect DT importing from TFI/VGI (thanks [@bryc] and others)
- [#87], [#95] - Fix FM envelope set association when renaming and type combobox overlapping (thanks [@OPNA2608])
- [#92] - Fix instrument declaration check on arpeggio effect (thanks [@attilaM68K])
- [#90] - Fix module and instrument specification documents (thanks [@jpcima])
- Fix bug increasing tone by octave when changing from Tri w to Saw in SSG waveform sequence
- [#97] - Fix .y12 instrument loader (thanks [@jpcima])
- [#101] - Fix to update the current order position when deleting a order (thanks [@jpcima], [@OPNA2608])
- Fix SCCI to be launched when changing configuration
- [#100], [#104] - Accelerate vgm/s98 export to skip sample generation (thanks [@jpcima])
- [#106] - Fix 12-bit wrapping behavior in MAME YM2608 ADPCM-A (thanks [@jpcima])
- [#109] - Fix icon to desplay all windows (thanks [@jpcima])
- [#114] - Fix incorrect event interpolation ([#113]; thanks [@jpcima])
- [#115] - Fix jam manager to replace key on data with new one when the same data is exist (thanks [@papiezak])
- Fix tick timings more accurate in using SCCI
- [#116] - Fix position the cursor after setting line edit contents (thanks [@jpcima])
- [#117] - Resolve warning 'catching polymorphic type by value' (thanks [@jpcima])
- Add file save check before opening a module from recent files list
- Fix incorrect wait counts in S98 exportation
- Fix wait time precision of VGM and S98
- Fix channel state retrieve to stop instrument sequences when there is no note at the first step

### Removed
- Remove module and instrument saving to past version
- [#3] - Remove square-mask frequency selection by note+pitch
- Delete module properties dialog open button in module settings groupbox

[#80]: https://github.com/rerrahkr/BambooTracker/pull/80
[#81]: https://github.com/rerrahkr/BambooTracker/issues/81
[#76]: https://github.com/rerrahkr/BambooTracker/pull/76
[#32]: https://github.com/rerrahkr/BambooTracker/issues/32
[#79]: https://github.com/rerrahkr/BambooTracker/issues/79
[#62]: https://github.com/rerrahkr/BambooTracker/issues/62
[#82]: https://github.com/rerrahkr/BambooTracker/issues/82
[#3]: https://github.com/rerrahkr/BambooTracker/issues/3
[#84]: https://github.com/rerrahkr/BambooTracker/issues/84
[#85]: https://github.com/rerrahkr/BambooTracker/pull/85
[#83]: https://github.com/rerrahkr/BambooTracker/issues/83
[#86]: https://github.com/rerrahkr/BambooTracker/issues/86
[#88]: https://github.com/rerrahkr/BambooTracker/issues/88
[#87]: https://github.com/rerrahkr/BambooTracker/issues/87
[#91]: https://github.com/rerrahkr/BambooTracker/issues/91
[#92]: https://github.com/rerrahkr/BambooTracker/issues/92
[#89]: https://github.com/rerrahkr/BambooTracker/pull/89
[#95]: https://github.com/rerrahkr/BambooTracker/pull/95
[#90]: https://github.com/rerrahkr/BambooTracker/pull/90
[#94]: https://github.com/rerrahkr/BambooTracker/pull/94
[#93]: https://github.com/rerrahkr/BambooTracker/issues/93
[#97]: https://github.com/rerrahkr/BambooTracker/pull/97
[#101]: https://github.com/rerrahkr/BambooTracker/issues/101
[#100]: https://github.com/rerrahkr/BambooTracker/issues/100
[#103]: https://github.com/rerrahkr/BambooTracker/pull/103
[#104]: https://github.com/rerrahkr/BambooTracker/pull/104
[#105]: https://github.com/rerrahkr/BambooTracker/pull/105
[#99]: https://github.com/rerrahkr/BambooTracker/pull/99
[#98]: https://github.com/rerrahkr/BambooTracker/issues/98
[#106]: https://github.com/rerrahkr/BambooTracker/pull/106
[#107]: https://github.com/rerrahkr/BambooTracker/pull/107
[#108]: https://github.com/rerrahkr/BambooTracker/pull/108
[#109]: https://github.com/rerrahkr/BambooTracker/pull/109
[#111]: https://github.com/rerrahkr/BambooTracker/pull/111
[#110]: https://github.com/rerrahkr/BambooTracker/pull/110
[#114]: https://github.com/rerrahkr/BambooTracker/pull/114
[#113]: https://github.com/rerrahkr/BambooTracker/issues/113
[#115]: https://github.com/rerrahkr/BambooTracker/issues/115
[#116]: https://github.com/rerrahkr/BambooTracker/pull/116
[#117]: https://github.com/rerrahkr/BambooTracker/pull/117
[#119]: https://github.com/rerrahkr/BambooTracker/issues/119
[#112]: https://github.com/rerrahkr/BambooTracker/issues/112
[#121]: https://github.com/rerrahkr/BambooTracker/pull/121

[@SuperJetSpade]: https://twitter.com/SuperJetSpade
[@KamuiKazuma]: https://github.com/KamuiKazuma
[@bryc]: https://github.com/bryc
[@attilaM68K]: https://github.com/attilaM68K
[@Delta-Psi]: https://github.com/Delta-Psi

## v0.1.6 (2019-03-16)
### Added
- [#63] - L10n: French, Japanese (thanks [@jpcima], [@trebmuh])
- Add file history
- [#65] - Add repository to Appveyor and Travis CI (thanks [@papiezak], [@OPNA2608], [@jpcima])
- [#70] - Add keyboard layout selection for note entry (thanks [@OPNA2608])
- Add key shortcuts for instrument list
- [#53] - Add 2 demo modules (thanks [@ehaupt], [@maakmusic])

### Fixed
- [#69] - Fix corruption in jamming (thanks [@maakmusic], [@ImATrackMan])
- [#61] - Fix translation building for <Qt5.12 (thanks [@OPNA2608], [@jpcima])
- [#64] - Fix the envlope reset behavior (thanks [@Pigu-A])
- [#67] - Fix SSG Mixer settings not being loaded properly ([#66]; thanks [@ImATrackMan], [@OPNA2608])
- [#68] - Fix initialization of SSG note slide flag (thanks [@OPNA2608])
- Fix position status in status bar when 0C effect
- [#71] - Fix showing row numbers in hex (thanks [@OPNA2608])
- [#73] - Fix DR and SR misreading in .ins (thanks [@scarletbullgon])
- Fix FM Mxyy effect to reverse yy by revesing FM volume order
- Fix incorrect notes on undoing of note transposition
- Fix some key shortcut duplication
- Fix selected region clear on moving cursor
- Fix module initialization after failing opening module
- [#11], [#75] - Fix corruption of release point in instrument sequence (thanks [@emninem], [@jpcima])
- Fix to restore FM RR on unmuting
- [#78] - Fix unused functions and parameters warnings (thanks [@jpcima])

[#61]: https://github.com/rerrahkr/BambooTracker/pull/61
[#63]: https://github.com/rerrahkr/BambooTracker/pull/63
[#64]: https://github.com/rerrahkr/BambooTracker/pull/64
[#66]: https://github.com/rerrahkr/BambooTracker/issues/66
[#67]: https://github.com/rerrahkr/BambooTracker/issues/67
[#68]: https://github.com/rerrahkr/BambooTracker/issues/68
[#69]: https://github.com/rerrahkr/BambooTracker/issues/69
[#65]: https://github.com/rerrahkr/BambooTracker/issues/65
[#71]: https://github.com/rerrahkr/BambooTracker/issues/71
[#73]: https://github.com/rerrahkr/BambooTracker/issues/73
[#70]: https://github.com/rerrahkr/BambooTracker/issues/70
[#11]: https://github.com/rerrahkr/BambooTracker/issues/11
[#75]: https://github.com/rerrahkr/BambooTracker/issues/75
[#78]: https://github.com/rerrahkr/BambooTracker/pull/78
[#53]: https://github.com/rerrahkr/BambooTracker/issues/53

[@ImATrackMan]: https://github.com/ImATrackMan
[@scarletbullgon]: https://github.com/scarletbullgon
[@emninem]: https://github.com/emninem

## v0.1.5 (2019-02-11)
### Added
- Add mix settings
- [#56] - Add installation rules and loader for translations (thanks [@jpcima])
- Load module from command line argument
- [#57] - Add FM envelope paste from plain text like MML (thanks [@pedipanol])
- Support .y12 and .ins instrument file importing
- [#58] - Implement retrieving channel state during playback (thanks [@papiezak])
- Add effect description in status bar

### Fixed
- [#48] - Fix incorrect label when setting buffer length to 1ms (thanks [@OPNA2608])
- [#51] - Add png icon (thanks [@ehaupt])
- [#52], [#54] - Fix install instructions for FreeBSD, Debian and Ubuntu (thanks [@ehaupt], [@alexmyczko])
- [#55] - Change default octave up/down keys (thanks [@galap-1])
- Fix instrument deep clone corruption by deregistering FM envelope user (thanks [@maakmusic])
- Fix editable step to apply entering events and be "0" selectable (thanks [@maakmusic])
- Fix to unmute track on jam mode (thanks [@maakmusic])
- [#44], [#59] - Divide instrument owned check from its number (thanks [@papiezak], [@djtuBIG-MaliceX])
- Fix echo buffer size and FM envelope reset before echo buffer access
- Change default FM envelope reset state to unused
- Fix wait time of VGM
- [#60] - Fix loop point of S98 (thanks [@scarletbullgon])
- Fix to work FM TL4 sequence macro (thanks [@maakmusic])
- Fix to skip instrument redefinition
- Fix FM TL3 calculation in instrument definition event

[#48]: https://github.com/rerrahkr/BambooTracker/issues/48
[#51]: https://github.com/rerrahkr/BambooTracker/issues/51
[#52]: https://github.com/rerrahkr/BambooTracker/pull/52
[#54]: https://github.com/rerrahkr/BambooTracker/pull/54
[#55]: https://github.com/rerrahkr/BambooTracker/issues/55
[#56]: https://github.com/rerrahkr/BambooTracker/pull/56
[#57]: https://github.com/rerrahkr/BambooTracker/issues/57
[#44]: https://github.com/rerrahkr/BambooTracker/issues/44
[#59]: https://github.com/rerrahkr/BambooTracker/issues/59
[#58]: https://github.com/rerrahkr/BambooTracker/issues/58
[#60]: https://github.com/rerrahkr/BambooTracker/issues/60

[@ehaupt]: https://github.com/ehaupt
[@galap-1]: https://github.com/galap-1
[@djtuBIG-MaliceX]: https://github.com/djtuBIG-MaliceX
[@scarletbullgon]: https://github.com/scarletbullgon

## v0.1.4 (2019-01-01)
### Added
- [#27] - Add application icon (thanks [@Decidetto])
- [#29], [#36], [#41] - Create BambooTracker.desktop (thanks [@alexmyczko], [@trebmuh], [@OPNA2608])
- [#30] - Create BambooTracker.1 (thanks [@alexmyczko], [@OPNA2608])
- [#37], [#38] - Support WOPN bank and instrument format (thanks [@jpcima])
- Add S98 export
- [#13] - Add selection to move the cursor to right in the effect columns (thanks [@jimbo1qaz])
- [#13] - Add highlighted step jump shortcuts (thanks [@jimbo1qaz])
- Add general settings descriptions in configuration dialog
- [#13] - Add editable step (thanks [@jimbo1qaz])
- Support SCCI (beta) ([#46], [#47]; thanks [@papiezak])
- Add tool bar in instrument list

### Fixed
- [#28] - Fix error when compiling before Qt5.10 (thanks [@papiezak] and others)
- [#33] - Revise README (thanks [@OPNA2608])
- [#31] - Replacing Json config with QSettings ([#17]; thanks [@OPNA2608])
- [#39] - Fix invalid memory access on main window destruction (thanks [@jpcima])
- Optimize wait data of exported VGM and S98
- [#40] - Change FM octave lower (thanks [@papiezak])
- Fix to set the current instrument when entering instrument number in pattern editor
- [#45] - Update README Linux build instructions, Fix typo in config dialogue (thanks [@OPNA2608])
- Rewrite about dialog

[#27]: https://github.com/rerrahkr/BambooTracker/issues/27
[#28]: https://github.com/rerrahkr/BambooTracker/issues/28
[#29]: https://github.com/rerrahkr/BambooTracker/pull/29
[#30]: https://github.com/rerrahkr/BambooTracker/pull/30
[#33]: https://github.com/rerrahkr/BambooTracker/pull/33
[#36]: https://github.com/rerrahkr/BambooTracker/pull/36
[#31]: https://github.com/rerrahkr/BambooTracker/pull/31
[#17]: https://github.com/rerrahkr/BambooTracker/issues/17
[#37]: https://github.com/rerrahkr/BambooTracker/pull/37
[#38]: https://github.com/rerrahkr/BambooTracker/pull/38
[#39]: https://github.com/rerrahkr/BambooTracker/pull/39
[#41]: https://github.com/rerrahkr/BambooTracker/pull/41
[#13]: https://github.com/rerrahkr/BambooTracker/issues/13
[#40]: https://github.com/rerrahkr/BambooTracker/issues/40
[#45]: https://github.com/rerrahkr/BambooTracker/pull/45
[#46]: https://github.com/rerrahkr/BambooTracker/issues/46
[#47]: https://github.com/rerrahkr/BambooTracker/pull/47

[@Decidetto]: https://github.com/Decidetto
[@alexmyczko]: https://github.com/alexmyczko
[@trebmuh]: https://github.com/trebmuh
[@jpcima]: https://github.com/jpcima

## v0.1.3 (2018-12-14)
### Added
- [#18] - Add FM operator copy/paste (thanks [@jimbo1qaz])
- [#14] - Assign instrument editor close shortcut to escape key (thanks [@jimbo1qaz])
- [#26] - Add menu shortcuts (thanks [@jimbo1qaz])
- [#2] - Add simple key shortcut settings (thanks [@pedipanol])

### Fixed
- Fix default place of load/save instrument
- [#25] - Change FM synthesis frequency from 110933Hz to 55466Hz (thanks [@papiezak])
- [#6], [#14] - Fix to set new property when adding instrument (thanks [@ScoopJohn], [@jimbo1qaz])
- [#2] - Fix wheel step in slider (thanks [@pedipanol])
- Fix file load/save to use local encoding (thanks [@maakmusic])
- Fix loop point check when exporting to wav and vgm (thanks [@nabetaqz])
- Fix support mod/inst file v1.0.1
- [#24] - Fix crash when adding song (thanks [@JonKaruzu])
- Fix focus in instrument list
- [#26] - Fix mute actions in pattern editor (thanks [@jimbo1qaz])
- [#26] - Fix pattern and order context menu shortcuts (thanks [@jimbo1qaz])

[#25]: https://github.com/rerrahkr/BambooTracker/issues/25
[#14]: https://github.com/rerrahkr/BambooTracker/issues/14
[#18]: https://github.com/rerrahkr/BambooTracker/issues/18
[#24]: https://github.com/rerrahkr/BambooTracker/issues/24
[#26]: https://github.com/rerrahkr/BambooTracker/issues/26

[@papiezak]: https://github.com/papiezak

## v0.1.2 (2018-12-02)
### Added
- [#2], [#6] - Add selection whether the order of FM volume is reversed (thanks [@pedipanol], [@ScoopJohn])
- [#22] - Add .dmp, .tfi and .vgi instrument file importing support ([#2], [#6]; thanks [@Pigu-A])
- [#12] - Add order/pattern focus shortcut (thanks [@jimbo1qaz])

### Fixed
- Fix module load error by missing pattern size initialization (thanks [@maakmusic])
- [#10] - Rearrange tab stops for better keyboard navigation (thanks [@Pigu-A])
- [#11] - Change envelope reset behavior (thanks [@Pigu-A])
- Fix instrument property used/unused inheritance by cloning
- [#20] - Split FM envelope copy/paste to multiple lines (thanks [@jimbo1qaz])
- [#21] - Fix selection range in pattern editor (thanks [@jimbo1qaz])
- [#1] - Make compatible with WinXP (thanks [@JonKaruzu])
- [#2] - Fix the order of input for columns (thanks [@pedipanol])
- [#23] - Fix tick event to be loaded from playing step (thanks [@pedipanol])
- Fix play/stop song when pressing return key
- [#12] - Fix insert/clone order selection when clicking above or below orders (thanks [@jimbo1qaz])
- Fix clone order to assign first free patterns

[#10]: https://github.com/rerrahkr/BambooTracker/pull/10
[#11]: https://github.com/rerrahkr/BambooTracker/pull/16
[#20]: https://github.com/rerrahkr/BambooTracker/pull/20
[#21]: https://github.com/rerrahkr/BambooTracker/issues/21
[#1]: https://github.com/rerrahkr/BambooTracker/issues/1
[#2]: https://github.com/rerrahkr/BambooTracker/issues/2
[#6]: https://github.com/rerrahkr/BambooTracker/issues/6
[#23]: https://github.com/rerrahkr/BambooTracker/issues/23
[#22]: https://github.com/rerrahkr/BambooTracker/pull/22
[#12]: https://github.com/rerrahkr/BambooTracker/issues/12

[@maakmusic]: https://twitter.com/maakmusic
[@Pigu-A]: https://github.com/Pigu-A
[@jimbo1qaz]: https://github.com/jimbo1qaz
[@JonKaruzu]: https://github.com/JonKaruzu
[@pedipanol]: https://github.com/pedipanol
[@ScoopJohn]: https://github.com/ScoopJohn

## v0.1.1 (2018-11-27)
### Added
- [#8] - Add dependencies and compile instructions on linux to readme (thanks [@OPNA2608])

### Fixed
- [#5] - Fix scroll directions of scroll bar arrow in instrument sequence editor (thanks [@Xyz39808])
- [#4] - Fix wrong offset jump when opening file with operator sequence (thanks [@9001], [@nabetaqz])
- [#9] - Check if saved soundOutput exists ([#7]; thanks [@OPNA2608], [@marysiamzawka])

[#5]: https://github.com/rerrahkr/BambooTracker/issues/5
[#4]: https://github.com/rerrahkr/BambooTracker/issues/4
[#9]: https://github.com/rerrahkr/BambooTracker/pull/9
[#7]: https://github.com/rerrahkr/BambooTracker/issues/7
[#8]: https://github.com/rerrahkr/BambooTracker/issues/8

[@Xyz39808]: https://github.com/Xyz39808
[@9001]: https://github.com/9001
[@OPNA2608]: https://github.com/OPNA2608
[@marysiamzawka]: https://github.com/marysiamzawka
[@nabetaqz]: https://twitter.com/nabetaqz

## v0.1.0 (2018-11-25)
### Added
- First release
