# Changelog

## Unreleased

### Fixed

- [#480] - Fix GitHub Actions flow ([#477], [#479]; thanks [@cxong], [@Zexxerd])
- [#478] - Suppress noise caused by overflow in blip_buf resampling (thanks [@Drillimation], TotO)

[#477]: https://github.com/BambooTracker/BambooTracker/issues/477
[#479]: https://github.com/BambooTracker/BambooTracker/issues/479
[#480]: https://github.com/BambooTracker/BambooTracker/pull/480
[#478]: https://github.com/BambooTracker/BambooTracker/issues/478

## v0.6.0 (2023-01-22)

### Added

- Add toolbar items of importing/exporting a bank file to the instrument list
- [#262] - Add ymfm emulation (thanks [@OPNA2608], [@freq-mod])
- Add more sample rate choices
- Add wait-write mode
- [#433] - Add key cut note (thanks [@wildmatsu])
- [#471] - Support FM Towns .pmb file import (thanks [@OPNA2608])
- [#472] - Add SSG mixing level configuration for VGM export ([#436]; thanks [@ValleyBell], [@nyanpasu64])
- [#476] - Add partial repeat settings for ADPCM samples ([#465]; thanks [@pedipanol])

### Changed

- [#464] - Raise Qt6 version on GitHub Actions to 6.4.0
- [#386] - Restore note pitch at the end of "Fixed" arpeggio sequence (thanks [@djmaximum17])
- [#347] - Change resampling method (thanks [@freq-mod])
- Remove redundant volume change
- [#433], [#434] - Rename 2 effects (thanks [@wildmatsu])
  - `0Sxx`: Note release
  - `ESxx`: Note cut
- [#433] - Enable to use `ESxx` in SSG, RSS, ADPCM channels (thanks [@wildmatsu])
- [#473] - Use Ubuntu 20.04 GHA runner image

### Fixed

- Fix misspelled words (thanks [@wildmatsu])
- Fix action to create a new module without saving
- Fix incorrect behaviour of `0sxx` effect in RSS channels (thanks [@Mugenri])
- Fix visibility of FM 3,4ch when song mode is changed from expanded to standard (thanks TotO)
- Run tick process when echo buffer is empty
- Reset `0Qxy`, `0Rxy` note slide in the next note on (thanks Getsuka-P)
- [#470] - Fix sliders not to move during editing unrelated slider ([#468], [#469]; thanks [@OPNA2608], [@cxong], [@Zexxerd])

[#464]: https://github.com/BambooTracker/BambooTracker/pull/464
[#386]: https://github.com/BambooTracker/BambooTracker/issues/386
[#262]: https://github.com/BambooTracker/BambooTracker/issues/262
[#347]: https://github.com/BambooTracker/BambooTracker/issues/347
[#433]: https://github.com/BambooTracker/BambooTracker/issues/433
[#434]: https://github.com/BambooTracker/BambooTracker/issues/434
[#471]: https://github.com/BambooTracker/BambooTracker/pull/471
[#470]: https://github.com/BambooTracker/BambooTracker/pull/470
[#468]: https://github.com/BambooTracker/BambooTracker/issues/468
[#469]: https://github.com/BambooTracker/BambooTracker/issues/469
[#472]: https://github.com/BambooTracker/BambooTracker/pull/472
[#436]: https://github.com/BambooTracker/BambooTracker/issues/436
[#473]: https://github.com/BambooTracker/BambooTracker/pull/473
[#465]: https://github.com/BambooTracker/BambooTracker/issues/465
[#476]: https://github.com/BambooTracker/BambooTracker/pull/476

## v0.5.3 (2022-09-18)

### Changed

- [#461] - Enhance font settings (thanks [@NEO-SPECTRUMAN])
- [#463] - Raise macOS version of GitHub Actions workflow to 11

### Fixed

- [#460] - Fix the maximum value of progress bar in WAV export dialog (thanks [@Drillimation])

[@NEO-SPECTRUMAN]: https://github.com/NEO-SPECTRUMAN

[#460]: https://github.com/BambooTracker/BambooTracker/issues/460
[#461]: https://github.com/BambooTracker/BambooTracker/issues/461
[#463]: https://github.com/BambooTracker/BambooTracker/pull/463

## v0.5.2 (2022-08-24)

### Fixed

- [#459] - Fix volume effects in FM 3ch OP2,3,4 to work (thanks [@Zexxerd])

[#459]: https://github.com/BambooTracker/BambooTracker/issues/459

## v0.5.1 (2022-07-23)

### Added

- [#407] - Implement pattern/order editor DPI scaling (thanks [@nyanpasu64])
- [#438] - Allow Neo Geo system type in DMP import (thanks [@OPNA2608])
- [#397] - Add CMake build option ([#405]; thanks [@OPNA2608])
- [#449] - Add source tarballs to releases page ([#450]; thanks [@ehaupt])
- [#414] - .spb support as ADPCM sample (thanks [@OPNA2608])
- 1 new pattern effect `EAxy`: extended volume slide

### Changed

- [#408] - Bumped Qt version of Win7 development builds to 5.15.2
- [#412] - Keep instrument editors on top of main window, spawn centered (thanks [@nyanpasu64])
- [#427] - Bumped Ubuntu version on build tests to 18.04
- [#429] - Improved keyboard key formatting in documentation (thanks [@cxong])
- [#426] - Update RtAudio submodule to upstream 5.2.0 (thanks [@nyanpasu64])
- Bumped maximum wave view framerate to 144Hz
- Update emu2149
- [#448] - Update Appveyor's macOS image to Catalina (thanks [@OPNA2608])
- [#420] - Free memory after instrument editor is closed (thanks [@nyanpasu64])
- [#442] - Don't focus main window spinboxes on mouse scroll (thanks [@nyanpasu64])
- Prepare 1 new instrument in initializing a new module
- [#458] - Update Nixpkgs version to 22.05 (thanks [@OPNA2608])
- Update translation: ja

### Fixed

- [#398] - Fix MSVC support ([#216]; thanks [@djtuBIG-MaliceX], [@OPNA2608])
- [#399] - Fix `0Gxx` note delay effect to be validated against the actual ticks per step (thanks [@Zexxerd])
- [#401] - Fix the volume spinbox to be displayed in uppercase (thanks [@wildmatsu])
- [#404] - Fix bug where pattern header was sometimes not redrawn (thanks [@nyanpasu64])
- [#400] - Reworded some text to sound more natural in English (thanks [@wildmatsu])
- [#410] - Fixed mislabeled piano keys in note entry layout ([#411]; thanks [@OPNA2608])
- [#402] - Fix icon pixellation on Qt6 ([#407]; thanks [@nyanpasu64])
- [#413] - Restore the release rate of FM instrument after using `ESxx` (thanks [@Zexxerd])
- [#406] - Fix the hovered cursor position in the order list and the pattern editor (thanks [@nyanpasu64])
- [#421] - Fixed a bug that caused the last tick processing of the current step to be skipped if there was a note delay event in the next step (thanks [@nyanpasu64])
- [#417] - Write the absolute file path to the file history (thanks [@nyanpasu64])
- [#424] - Fix instrument editor envelope graphs on Breeze master (thanks [@nyanpasu64])
- [#435] - Fix VGM files being written with incorrect headers (thanks [@nyanpasu64])
- [#418] - Fix SliderStyle memory leak ([#440], thanks [@nyanpasu64])
- [#437] - Keep SSG silent when using `0Hxy` in a muted track (thanks [@Zexxerd])
- [#443] - Fix song length estimation to handle tempo/speed change correctly (thanks [@Drillimation])
- [#444] - Fix unexpected envelope reset of FM3-OP4 (thanks [@Zexxerd])
- [#447] - Check if the read/write position of the binary container is valid (thanks [@OPNA2608])
- [#425] - Fix improper sequence property execution of `0Gxx` that skips the first sequence data (thanks [@Zexxerd])
- [#452] - Fix icon image corruptions in About and Welcome dialogs ([#451]; thanks [@OPNA2608])
- [#454] - Reduce misalignment of text in the status bar ([#415]; thanks [@OPNA2608])
- [#422] - Fix crash when redoing pattern effect entry `0Bxx`, `0Cxx` and `0Dxx` ([#441]; thanks [@nyanpasu64])
- Fix crash when playing a non-existent instrument on ADPCM channel

[@cxong]: https://github.com/cxong

[#398]: https://github.com/BambooTracker/BambooTracker/pull/398
[#399]: https://github.com/BambooTracker/BambooTracker/issues/399
[#401]: https://github.com/BambooTracker/BambooTracker/issues/401
[#404]: https://github.com/BambooTracker/BambooTracker/pull/404
[#400]: https://github.com/BambooTracker/BambooTracker/pull/400
[#408]: https://github.com/BambooTracker/BambooTracker/pull/408
[#410]: https://github.com/BambooTracker/BambooTracker/issues/410
[#411]: https://github.com/BambooTracker/BambooTracker/pull/411
[#407]: https://github.com/BambooTracker/BambooTracker/pull/407
[#402]: https://github.com/BambooTracker/BambooTracker/issues/402
[#413]: https://github.com/BambooTracker/BambooTracker/issues/413
[#412]: https://github.com/BambooTracker/BambooTracker/pull/412
[#406]: https://github.com/BambooTracker/BambooTracker/issues/406
[#421]: https://github.com/BambooTracker/BambooTracker/issues/421
[#417]: https://github.com/BambooTracker/BambooTracker/issues/417
[#424]: https://github.com/BambooTracker/BambooTracker/pull/424
[#427]: https://github.com/BambooTracker/BambooTracker/pull/427
[#429]: https://github.com/BambooTracker/BambooTracker/pull/429
[#435]: https://github.com/BambooTracker/BambooTracker/pull/435
[#426]: https://github.com/BambooTracker/BambooTracker/pull/426
[#438]: https://github.com/BambooTracker/BambooTracker/pull/438
[#418]: https://github.com/BambooTracker/BambooTracker/issues/418
[#440]: https://github.com/BambooTracker/BambooTracker/pull/440
[#397]: https://github.com/BambooTracker/BambooTracker/issues/397
[#405]: https://github.com/BambooTracker/BambooTracker/pull/405
[#448]: https://github.com/BambooTracker/BambooTracker/pull/448
[#437]: https://github.com/BambooTracker/BambooTracker/issues/437
[#443]: https://github.com/BambooTracker/BambooTracker/issues/443
[#444]: https://github.com/BambooTracker/BambooTracker/issues/444
[#447]: https://github.com/BambooTracker/BambooTracker/issues/447
[#425]: https://github.com/BambooTracker/BambooTracker/issues/425
[#449]: https://github.com/BambooTracker/BambooTracker/issues/449
[#450]: https://github.com/BambooTracker/BambooTracker/pull/450
[#451]: https://github.com/BambooTracker/BambooTracker/issues/451
[#452]: https://github.com/BambooTracker/BambooTracker/pull/452
[#420]: https://github.com/BambooTracker/BambooTracker/issues/420
[#414]: https://github.com/BambooTracker/BambooTracker/issues/414
[#454]: https://github.com/BambooTracker/BambooTracker/pull/454
[#415]: https://github.com/BambooTracker/BambooTracker/issues/415
[#442]: https://github.com/BambooTracker/BambooTracker/pull/442
[#422]: https://github.com/BambooTracker/BambooTracker/issues/422
[#441]: https://github.com/BambooTracker/BambooTracker/issues/441
[#458]: https://github.com/BambooTracker/BambooTracker/pull/458

## v0.5.0 (2021-08-01)

### Added

- 2 new pattern effect
  - `0Kxy`: Retrigger ([#249]; thanks [@Speedvicio], [@Ravancloak])
  - `ESxx`: FM envelope reset ([#301]; thanks [@Zexxerd])
- [#354] - VGM export for YM2610B ([#335]; thanks [@freq-mod], [@ToughkidDev])
- [#314] - FM / ADPCM panning sequence (thanks [@Toonlink8101])
- Panning settings in the drumkit editor
- [#345], [#372] - Key signature settings (thanks [@wildmatsu], [@OPNA2608])
- [#365] - WAV export per channel (thanks [@Toonlink8101])
- [#345] - German notation system (thanks [@OPNA2608], [@wildmatsu], [@freq-mod])
- [#332], [#373] - Qt6 support ([#298]; thanks [@OPNA2608])
- Welcome dialog (thanks [@OPNA2608])
- [#381] - Support .pzi with `PZI0` file magic ([#379]; thanks [@OPNA2608], [@freq-mod])
- 2 new sample modules (thanks SuperJet Spade, TastySnax12)
- [#387] - .btm -> BT filetype association on Linux ([#388]; thanks [@nyanpasu64], [@OPNA2608])

### Changed

- [#333] - Allow single pattern clone at hovered or current position in the order list (thanks [@Mugenri])
- [#348] - Update RtAudio & RtMidi library (thanks [@OPNA2608])
- [#342] - Use Qt's loading / saving file dialog on Linux and BSD ([#338]; thanks [@freq-mod], [@EnfauKerus], [@OPNA2608])
- Save track visibility for each song
- Change delay effects to be executed even on the 0th count
- [#366] - Load multiple instrument files at a time (thanks [@OPNA2608])
- [#367] - Offer wildcard and "all supported formats" extensions in file dialogues (thanks [@OPNA2608])
- Change item widths in the status bar
- [#374] - Exclude C86CTL and SCCI headers in the compilation on macOS and Linux ([#357]; thanks [@fpesari], [@OPNA2608], [@freq-mod])
- [#371] - Change the number of buffers in RtAudio ([#363]; thanks [@nyanpasu64], [@freq-mod], [@OPNA2608])
- Modify several dialog modalities
- [#382] - Update MAME emulation (thanks [@OPNA2608])
- [#382] - Change the project license to GPL v2+
- Move the repository to GitHub Organization
- [#389] - Update CI's Nixpkgs channel to 21.05 (thanks [@OPNA2608])
- Update translation: pl, ja (thanks [@freq-mod])

### Fixed

- [#328] - Fix progress bar length in the wav export dialog (thanks [@Drillimation], [@OPNA2608])
- [#336] - Fix noise distortion for Nuked OPN-Mod ([#255]; thanks [@OPNA2608], [@jpcima], [@freq-mod], [@nukeykt])
- [#339] - Fix the order of loading FM operators in .vgi and .tfi (thanks [@OPNA2608])
- [#340] - Fix ADPCM note off when the instrument release is not set (thanks [@Zexxerd])
- [#341] - Fix undo behaviour of instrument replacement (thanks [@Mugenri])
- [#353] - Fix definition of qmake variables ([#352]; thanks [@OPNA2608])
- [#358] - Fix AR3 parsing from text (thanks [@freq-mod])
- Update octave status display when changing octave spinbox
- [#360], [#361] - Cancel renaming an instrument before removing it (thanks [@OPNA2608], [@RaijinXBlade])
- Restore the sample rate when the export process is canceled
- Fix incorrect speed/tempo retrieving in the ADPCM track
- [#368] - Move lang files to fix lupdate errors (Thanks [@OPNA2608])
- [#376] - Skip `0Fxx` tempo change effect that have the same value as the previously set one (thanks [@nyanpasu64])
- [#377] - Fixed error in loading .bti containing ADPCM samples larger than 65458B (thanks [@freq-mod])
- [#385] - Fix the tick counter to change `0Fxx` tempo and step size ([#376], [#383]; thanks [@nyanpasu64], [@OPNA2608])
- [#334] - Fix CI (thanks [@OPNA2608])
- [#344] - Fix document (thanks [@freq-mod])
- [#390] - Fix lupdate errors/warnings ([#391], [#392]; thanks [@OPNA2608])
- [#393] - Fix an error of copying and pasting pattern cells when hiding any tracks (thanks [@Toonlink8101], [@OPNA2608])

[@Mugenri]: https://github.com/Mugenri
[@Speedvicio]: https://github.com/Speedvicio
[@RaijinXBlade]: https://github.com/RaijinXBlade
[@ToughkidDev]: https://github.com/ToughkidDev
[@wildmatsu]: https://github.com/wildmatsu
[@fpesari]: https://github.com/fpesari

[#328]: https://github.com/BambooTracker/BambooTracker/issues/328
[#255]: https://github.com/BambooTracker/BambooTracker/issues/255
[#336]: https://github.com/BambooTracker/BambooTracker/pull/336
[#339]: https://github.com/BambooTracker/BambooTracker/issues/339
[#340]: https://github.com/BambooTracker/BambooTracker/issues/340
[#341]: https://github.com/BambooTracker/BambooTracker/issues/341
[#333]: https://github.com/BambooTracker/BambooTracker/issues/333
[#344]: https://github.com/BambooTracker/BambooTracker/pull/344
[#332]: https://github.com/BambooTracker/BambooTracker/pull/332
[#298]: https://github.com/BambooTracker/BambooTracker/issues/298
[#348]: https://github.com/BambooTracker/BambooTracker/pull/348
[#342]: https://github.com/BambooTracker/BambooTracker/pull/342
[#338]: https://github.com/BambooTracker/BambooTracker/issues/338
[#353]: https://github.com/BambooTracker/BambooTracker/pull/353
[#352]: https://github.com/BambooTracker/BambooTracker/issues/352
[#358]: https://github.com/BambooTracker/BambooTracker/issues/358
[#301]: https://github.com/BambooTracker/BambooTracker/issues/301
[#249]: https://github.com/BambooTracker/BambooTracker/issues/249
[#334]: https://github.com/BambooTracker/BambooTracker/pull/334
[#360]: https://github.com/BambooTracker/BambooTracker/issues/360
[#361]: https://github.com/BambooTracker/BambooTracker/issues/361
[#354]: https://github.com/BambooTracker/BambooTracker/pull/354
[#335]: https://github.com/BambooTracker/BambooTracker/issues/335
[#366]: https://github.com/BambooTracker/BambooTracker/issues/366
[#368]: https://github.com/BambooTracker/BambooTracker/pull/368
[#367]: https://github.com/BambooTracker/BambooTracker/issues/367
[#314]: https://github.com/BambooTracker/BambooTracker/issues/314
[#345]: https://github.com/BambooTracker/BambooTracker/issues/345
[#373]: https://github.com/BambooTracker/BambooTracker/pull/373
[#365]: https://github.com/BambooTracker/BambooTracker/issues/365
[#357]: https://github.com/BambooTracker/BambooTracker/issues/357
[#374]: https://github.com/BambooTracker/BambooTracker/pull/374
[#376]: https://github.com/BambooTracker/BambooTracker/issues/376
[#371]: https://github.com/BambooTracker/BambooTracker/pull/371
[#363]: https://github.com/BambooTracker/BambooTracker/issues/363
[#372]: https://github.com/BambooTracker/BambooTracker/pull/372
[#377]: https://github.com/BambooTracker/BambooTracker/issues/377
[#381]: https://github.com/BambooTracker/BambooTracker/pull/381
[#379]: https://github.com/BambooTracker/BambooTracker/issues/379
[#382]: https://github.com/BambooTracker/BambooTracker/pull/382
[#387]: https://github.com/BambooTracker/BambooTracker/issues/387
[#388]: https://github.com/BambooTracker/BambooTracker/pull/388
[#389]: https://github.com/BambooTracker/BambooTracker/pull/389
[#390]: https://github.com/BambooTracker/BambooTracker/issues/390
[#391]: https://github.com/BambooTracker/BambooTracker/pulls/391
[#392]: https://github.com/BambooTracker/BambooTracker/pulls/392
[#383]: https://github.com/BambooTracker/BambooTracker/issues/383
[#385]: https://github.com/BambooTracker/BambooTracker/pull/385
[#393]: https://github.com/BambooTracker/BambooTracker/issues/393

## v0.4.6 (2021-02-11)

### Added

- [#285] - Linux build (thanks [@OPNA2608])
- [#294] - Build option for linking against system RtAudio and RtMidi ([#263], [#288]; thanks [@OPNA2608])
- [#308] - Support .pvi import which has a file identifier as `PVI1` (thanks [@freq-mod], [@OPNA2608])
- [#313] - .p86, .pzi and .pps support as ADPCM samples (thanks [@OPNA2608], [@freq-mod])
- [#323] - DRAM memory warning (thanks [@freq-mod])
- [#286] - Polish support (thanks [@freq-mod])
- 1 new sample module (thanks [@Zexxerd])

### Changed

- [#285] - Change CI service from Travis CI to GitHub Actions (thanks [@OPNA2608])
- Use current channel in jam mode when editing an instrument (thanks [@Ravancloak])
- Keep playing state after changing the configuration (thanks [@OPNA2608])
- [#289] - Change MIDI input initialization ([#288]; thanks [@OPNA2608])
- [#296] - Make the release build for Windows XP in appveyor (thanks [@OPNA2608])
- Load ADPCM deltaN setting and sample count in .pvi file ([#308], [#316]; thanks [@OPNA2608], [@freq-mod])
- [#306] - Clean README ([#297]; thanks [@freq-mod], [@OPNA2608])

### Fixed

- [#284] - Fix the bug locking paint events after opening non-existent module (thanks [@OPNA2608])
- [#291] - Fix icon install paths (thanks [@OPNA2608])
- Fix .tfi import to load the operator 2 and 3 correctly ([#290]; thanks [@OPNA2608])
- [#299] - Fix a glitch in instrument macro editors when scrolling (thanks [@freq-mod])
- [#303] - Reduce futile memory copy in WAV export to prevent the memory error (thanks [@Drillimation])
- [#307] - Fix GitHub Actions workflow for macOS (thanks [@OPNA2608])
- [#304] - Fix the iteration bug of instrument sequence (thanks [@freq-mod])
- Fix several text drawing not to overlap (thanks [@EnfauKerus], [@OPNA2608])
- [#320] - Accept digit input with numpad (thanks [@OPNA2608])
- Force execution of SSG mixer effect which is set upon key on (thanks [@EnfauKerus])
- Clean some code ([#295], [#311], [#312], [#324]; thanks [@freq-mod], [@jpcima], [@OPNA2608], [@EnfauKerus])

[@EnfauKerus]: https://github.com/EnfauKerus

[#285]: https://github.com/BambooTracker/BambooTracker/pull/285
[#284]: https://github.com/BambooTracker/BambooTracker/issues/284
[#288]: https://github.com/BambooTracker/BambooTracker/pull/288
[#289]: https://github.com/BambooTracker/BambooTracker/pull/289
[#290]: https://github.com/BambooTracker/BambooTracker/pull/290
[#291]: https://github.com/BambooTracker/BambooTracker/pull/291
[#286]: https://github.com/BambooTracker/BambooTracker/pull/286
[#295]: https://github.com/BambooTracker/BambooTracker/issues/295
[#299]: https://github.com/BambooTracker/BambooTracker/issues/299
[#296]: https://github.com/BambooTracker/BambooTracker/issues/296
[#263]: https://github.com/BambooTracker/BambooTracker/issues/263
[#294]: https://github.com/BambooTracker/BambooTracker/pull/294
[#303]: https://github.com/BambooTracker/BambooTracker/issues/303
[#307]: https://github.com/BambooTracker/BambooTracker/pull/307
[#308]: https://github.com/BambooTracker/BambooTracker/issues/308
[#311]: https://github.com/BambooTracker/BambooTracker/issues/311
[#312]: https://github.com/BambooTracker/BambooTracker/pull/312
[#316]: https://github.com/BambooTracker/BambooTracker/issues/316
[#304]: https://github.com/BambooTracker/BambooTracker/issues/304
[#313]: https://github.com/BambooTracker/BambooTracker/issues/313
[#320]: https://github.com/BambooTracker/BambooTracker/issues/320
[#323]: https://github.com/BambooTracker/BambooTracker/issues/323
[#306]: https://github.com/BambooTracker/BambooTracker/pull/306
[#297]: https://github.com/BambooTracker/BambooTracker/issues/297
[#324]: https://github.com/BambooTracker/BambooTracker/pull/324

## v0.4.5 (2020-11-05)

### Added

- [#238] - MIDI API selection ([#230]; thanks [@OPNA2608], [@ehaupt])
- Frame rate setting in the wave view
- 1 new pattern effect `FPxx`: fine detune
- 1 new sample module (thanks [@Zexxerd])
- 1 new color schemes (thanks [@Yuzu4K])

### Changed

- [#266] - Set a license to a part of the source code (thanks [@superctr])
- [#274] - Try to initialize audio stream by multiple APIs on the first launch ([#270]; thanks [@N-SPC700], [@OPNA2608])
- [#268], [#273] - Changed issue template policy (thanks [@OPNA2608])
- [#278] - Display error descriptions in File I/O and MIDI input (thanks [@OPNA2608])
- [#278] - Set Dummy API to MIDI input during the initialization (thanks [@OPNA2608])
- [#281] - Separate data and code in the repository (thanks [@OPNA2608])
- Update translation: ja

### Fixed

- [#256] - Fix a crash bug on launch when maximized (thanks [@nyanpasu64])
- [#259] - Fix copying and pasting position error when hiding any tracks (thanks [@YoshiLightStar])
- [#258] - Fix an error of wav/vgm/s98 export (thanks [@Zexxerd])
- [#260] - Update license of Silk icons (thanks [@alexmyczko])
- [#261] - Fix the update timing of color scheme when color is changed (thanks [@Genatari])
- Add the missing descriptions to the effect list dialog
- Fix `0Xxx`, `0Yxx` and `0Zxx` to work
- [#267] - Fix a crash when changing the current song type (thanks [@Zexxerd])
- [#269] - Fix order insert when changing the pattern size to less than 64 (thanks [@Toonlink8101])
- [#272] - Fix FM3ch expanded labeling and muting bug when changing the song type (thanks [@Toonlink8101])
- [#277] - Wait pattern drawing until initializing internal data ([#276]; thanks [@OPNA2608])
- [#280] - Synchronize sample rate between the emulation and the audio stream ([#279]; thanks [@OPNA2608])
- [#283] - Fix the module open error when launching the tracker firstly by macOS finder ([#282]; thanks [@N-SPC700], [@OPNA2608])

[@Yuzu4K]: https://twitter.com/Yuzu4K
[@Zexxerd]: https://github.com/Zexxerd
[@Genatari]: https://github.com/Genatari
[@Toonlink8101]: https://github.com/Toonlink8101

[#256]: https://github.com/BambooTracker/BambooTracker/issues/256
[#259]: https://github.com/BambooTracker/BambooTracker/issues/259
[#258]: https://github.com/BambooTracker/BambooTracker/issues/258
[#260]: https://github.com/BambooTracker/BambooTracker/issues/260
[#261]: https://github.com/BambooTracker/BambooTracker/issues/261
[#238]: https://github.com/BambooTracker/BambooTracker/pull/238
[#266]: https://github.com/BambooTracker/BambooTracker/issues/266
[#267]: https://github.com/BambooTracker/BambooTracker/issues/267
[#269]: https://github.com/BambooTracker/BambooTracker/issues/269
[#270]: https://github.com/BambooTracker/BambooTracker/issues/270
[#272]: https://github.com/BambooTracker/BambooTracker/issues/272
[#274]: https://github.com/BambooTracker/BambooTracker/pull/274
[#268]: https://github.com/BambooTracker/BambooTracker/pull/268
[#273]: https://github.com/BambooTracker/BambooTracker/pull/273
[#276]: https://github.com/BambooTracker/BambooTracker/issues/276
[#277]: https://github.com/BambooTracker/BambooTracker/pull/277
[#278]: https://github.com/BambooTracker/BambooTracker/issues/278
[#279]: https://github.com/BambooTracker/BambooTracker/issues/279
[#280]: https://github.com/BambooTracker/BambooTracker/pull/280
[#281]: https://github.com/BambooTracker/BambooTracker/pull/281
[#282]: https://github.com/BambooTracker/BambooTracker/issues/282
[#283]: https://github.com/BambooTracker/BambooTracker/pull/283

## v0.4.4 (2020-08-22)

### Added

- [#242] - Estimate Song Length (thanks [@Drillimation])
- Settings of header color and editor shadow
- [#136] - Support MUCOM88 voice file import (thanks [@freq-mod])
- [#237] - Supported by Nixpkgs (thanks [@OPNA2608])
- 1 new sample module (thanks SuperJet Spade)
- 5 new color schemes

### Changed

- [#234] - Change maximum import rate of ADPCM sample to 55.5kHz (thanks [@superctr], [@OPNA2608])
- [#239] - Improve compiler & version detection (thanks [@OPNA2608])
- [#238] - Update RtMidi library
- Change painting order of pattern and order border
- [#250] - Improve misc file installation ([#248]; thanks [@OPNA2608])
- [#253] - Reduce build package size ([#251])
- [#252] - Support bank import from .FF including SSG-EG params (thanks [@freq-mod])
- Update translation: ja

### Fixed

- [#229] - Fix detection of effect `0Bxx`, `0Cxx` and `0Dxx` in WAV/VGM/S98 export (thanks [@YuzuMSX])
- Enable play/stop shortcuts in the instrument list (thanks [@nyanpasu64])
- [#228] - Fix incorrect instrument swapping (thanks [@YuzuMSX])
- [#233] - Fix corruption of audio thread on macOS ([#231]; thanks [@OPNA2608])
- [#232] - Ignore `0Bxx`, `0Cxx` and `0Dxx` when looping a pattern (thanks [@OPNA2608])
- [#230], [#236], [#241] - Fix compilation error on BSD ([#235]; thanks [@ehaupt], [@OPNA2608])
- Fix to enable border color setting of the pattern editor and the order list
- Fix .vgi import to load the operator 2 and 3 correctly
- [#243] - Fix several typos (thanks [@alexmyczko])

[@superctr]: https://github.com/superctr
[@Drillimation]: https://github.com/Drillimation

[#229]: https://github.com/BambooTracker/BambooTracker/issues/229
[#228]: https://github.com/BambooTracker/BambooTracker/issues/228
[#233]: https://github.com/BambooTracker/BambooTracker/pull/233
[#231]: https://github.com/BambooTracker/BambooTracker/issues/231
[#234]: https://github.com/BambooTracker/BambooTracker/issues/234
[#232]: https://github.com/BambooTracker/BambooTracker/issues/232
[#230]: https://github.com/BambooTracker/BambooTracker/pull/230
[#235]: https://github.com/BambooTracker/BambooTracker/issues/235
[#236]: https://github.com/BambooTracker/BambooTracker/pull/236
[#239]: https://github.com/BambooTracker/BambooTracker/pull/239
[#241]: https://github.com/BambooTracker/BambooTracker/pull/241
[#242]: https://github.com/BambooTracker/BambooTracker/issues/242
[#243]: https://github.com/BambooTracker/BambooTracker/pull/243
[#237]: https://github.com/BambooTracker/BambooTracker/pull/237
[#248]: https://github.com/BambooTracker/BambooTracker/issues/248
[#250]: https://github.com/BambooTracker/BambooTracker/pull/250/files
[#251]: https://github.com/BambooTracker/BambooTracker/issues/251
[#253]: https://github.com/BambooTracker/BambooTracker/pull/253
[#252]: https://github.com/BambooTracker/BambooTracker/issues/252

## 0.4.3 (2020-06-28)

### Added

- [#2], [#213], [#214] - ADPCM sample editor (thanks [@freq-mod])
- [#98] - Track visibility control (thanks [@pedipanol], [@freq-mod])
- Slope notation in text edit of instrument sequence
- [#98] - Selection of FM "None" target in vgm/s98 export (thanks [@freq-mod])
- [#136] - Support PMD .FF file import (thanks [@OPNA2608])

### Changed

- [#219] - Enable SSG Tone/Noise editor to turn off tone and noise (thanks [@ImATrackMan])
- Enable song type change without creating a new song
- [#200] - Change build methods on CI (thanks [@OPNA2608])
- Update translation: ja

### Fixed

- Resize width of the order list after closing configuration dialog
- Fix corruption while loading module
- [#217] - Remove redundant call (thanks [@nyanpasu64])
- [#216], [#218], [#221], [#227] - Fix some compilation warings (thanks [@nyanpasu64], [@OPNA2608])
- [#210] - Finish editing an instrument name when leaving focus (thanks [@OPNA2608])
- [#220] - Restore setting of FM volume reverse on launch (thanks [@nyanpasu64], [@OPNA2608])
- Keep the main window open when canceling save dialog in closing
- [#225] - Modify some code (thanks [@nyanpasu64])
- Modify appearance slightly

[#213]: https://github.com/BambooTracker/BambooTracker/issues/213
[#214]: https://github.com/BambooTracker/BambooTracker/issues/214
[#217]: https://github.com/BambooTracker/BambooTracker/pull/217
[#216]: https://github.com/BambooTracker/BambooTracker/issues/216
[#218]: https://github.com/BambooTracker/BambooTracker/issues/218
[#210]: https://github.com/BambooTracker/BambooTracker/issues/210
[#220]: https://github.com/BambooTracker/BambooTracker/issues/220
[#221]: https://github.com/BambooTracker/BambooTracker/pull/221
[#219]: https://github.com/BambooTracker/BambooTracker/issues/219
[#225]: https://github.com/BambooTracker/BambooTracker/issues/225
[#200]: https://github.com/BambooTracker/BambooTracker/pull/200
[#227]: https://github.com/BambooTracker/BambooTracker/pull/227

## v0.4.2 (2020-05-10)

### Added

- [#6], [#211] - ADPCM drumkit (thanks [@ImATrackMan], [@pedipanol] and others)
- Color settings of pattern marker
- [#206] - Enable to swap instruments (thanks [@Mikejmoffitt])
- Song transposition (thanks [@Mijinkono02])
- Track swapping
- Paste and insert in pattern
- [#144] - Jamming volume settings (thanks [@marysiamzawka])
- Paste mode selection: Cursor/Selection/Fill
- 2 new sample modules (thanks [@TastySnax12], [@HeavyViper])

### Changed

- Change comment dialog to modeless
- Rename ADPCM Waveform to Sample
- Move wave view settings to View menu
- Update translation: ja

### Fixed

- [#209] - Fix crashing effect list dialog on opening (thanks [@Ikalou])
- [#208] - Fix speed/tempo states retrieving on starting playback (thanks [@freq-mod])
- [#207] - Fix pattern editor resolution after module open error on launch (thanks [@freq-mod])
- Fix bank loading crash if properties are not changed
- Fix to load "Write only used samples" settings on launch

[@TastySnax12]: https://twitter.com/TastySnax12
[@HeavyViper]: https://twitter.com/HeavyViper
[@Mijinkono02]: https://twitter.com/Mijinkono02

[#209]: https://github.com/BambooTracker/BambooTracker/issues/209
[#208]: https://github.com/BambooTracker/BambooTracker/issues/208
[#207]: https://github.com/BambooTracker/BambooTracker/issues/207
[#211]: https://github.com/BambooTracker/BambooTracker/issues/211
[#144]: https://github.com/BambooTracker/BambooTracker/issues/144

## v0.4.1 (2020-04-20)

### Added

- Shortcut settings
- 6 new shortcuts
  - Increase/Decrease pattern size
  - Increase/Decrease edit step
  - Previous/Next song
- 255 new sample instruments (thanks [Takeshi Abo])

### Changed

- [#198] - Raise the maximum of tick rate to 511Hz (thanks [@djmaximum17])
- [#205] - Modify README ([#203]; thanks [@OPNA2608], [@Mikejmoffitt])
- [#206] - Enable sound preview in bank import (thanks [@Mikejmoffitt])
- Update translation: ja

### Fixed

- [#197] - Fix the bug that a book mark name cannot be changed (thanks [@Zeinok], [@OPNA2608])
- [#199] - Fix ADPCM sample sometimes has wrong DC offset in synthesis (thanks [@galap-1])
- Reset internal start/stop address point of ADPCM sample after loading samples
- [#201] - Redraw order list after resizing (thanks [@OPNA2608])
- Fix `Dxyy` crash when starting playing from the cursor (thanks [@maakmusic])
- [#204] - Fix missing install target for translations (thanks [@OPNA2608])

[@Zeinok]: https://github.com/Zeinok
[@djmaximum17]: https://github.com/djmaximum17
[@Mikejmoffitt]: https://github.com/Mikejmoffitt
[Takeshi Abo]: https://twitter.com/valsound

[#197]: https://github.com/BambooTracker/BambooTracker/pull/197
[#199]: https://github.com/BambooTracker/BambooTracker/issues/199
[#198]: https://github.com/BambooTracker/BambooTracker/issues/198
[#201]: https://github.com/BambooTracker/BambooTracker/issues/201
[#204]: https://github.com/BambooTracker/BambooTracker/pull/204
[#205]: https://github.com/BambooTracker/BambooTracker/pull/205
[#203]: https://github.com/BambooTracker/BambooTracker/issues/203
[#206]: https://github.com/BambooTracker/BambooTracker/issues/206

## v0.4.0 (2020-03-27)

### Added

- [#6], [#184], [#185], [#186], [#187] - Support ADPCM (thanks [@ScoopJohn] and others)
- Shortcuts instead of insert key
- Note transpose shortcuts using mouse wheel
- Bookmark manager
- [#189] - Increase/Decrease selected values in a pattern by 1 or 16 (thanks [@nyanpasu64])
- Volume mask
- Row marker and play from marker
- [#193] - Shortcuts moving to previous/next order (thanks [@Xyz39808])
- Cursor jump (Go To)
- [#193] - Shortcut to select previous/next instrument (thanks [@Xyz39808])
- Visible selection for toolbars and status bar
- [#193] - Play only current step (thanks [@Xyz39808])
- 2 new sample modules (thanks [@RigidatoMS] and [@ImATrackMan])

### Changed

- [#182] - Enable the first note-on when tone portamento is set before the playing start position (thanks [@freq-mod])
- [#183] - Implement ADPCM part in Nuked OPN-Mod (thanks [@jpcima])
- [#186] - Selectable to overwrite instrument properties depending on whether or not to be edited (thanks [@OPNA2608])
- Make an instrument editor form on opening
- Change "Autoset instrument" to instrument mask
- [#194] - Allow to load vgi/tfi instrument set DT as 0x07 (thanks [@YoshiLightStar])
- Restore toolbar position on launch
- Update translation: ja

### Fixed

- Close all windows when the main window closed
- Fix paint glitches of order list and pattern editor
- Fix edit step crashing on change
- [#190] - Fix the error of FM envelope/operator paste when "Show FM detune as signed" enabled (thanks [@nyanpasu64])
- [#191] - Enable copy and paste in the order list without pattern edit mode (thanks [@nyanpasu64])
- [#177] - Fix the bug that could not create a new instrument (thanks [@Pigu-A], [@nyanpasu64], [@pedipanol])
- [#192] - Fix that any shortcuts set in the configuration is ignored (thanks [@SMB7])

### Removed

- Expand/Shrink pattern selection from menubar

[@freq-mod]: https://github.com/freq-mod
[@RigidatoMS]: https://twitter.com/RigidatoMS
[@nyanpasu64]: https://github.com/nyanpasu64
[@YoshiLightStar]: https://github.com/YoshiLightStar

[#182]: https://github.com/BambooTracker/BambooTracker/issues/182
[#183]: https://github.com/BambooTracker/BambooTracker/pull/183
[#184]: https://github.com/BambooTracker/BambooTracker/issues/184
[#185]: https://github.com/BambooTracker/BambooTracker/issues/185
[#186]: https://github.com/BambooTracker/BambooTracker/issues/186
[#187]: https://github.com/BambooTracker/BambooTracker/issues/187
[#189]: https://github.com/BambooTracker/BambooTracker/issues/189
[#190]: https://github.com/BambooTracker/BambooTracker/issues/190
[#191]: https://github.com/BambooTracker/BambooTracker/issues/191
[#177]: https://github.com/BambooTracker/BambooTracker/issues/177
[#192]: https://github.com/BambooTracker/BambooTracker/issues/192
[#193]: https://github.com/BambooTracker/BambooTracker/issues/193
[#194]: https://github.com/BambooTracker/BambooTracker/issues/194

## v0.3.5 (2020-02-15)

### Added

- Support C86CTL

### Changed

- Update SCCI library

### Fixed

- Fix AutoEnv and square mask pitch calculation
- [#181] - Fix crash when changing length of instrument sequence during jamming (thanks [@papiezak])
- Fix SCCI crash on launching in Windows XP (thanks [@ponzu0147])
- Fix SSG arpeggio effect crashing on setting `0000` without a instrument

[@ponzu0147]: https://twitter.com/ponzu0147

[#181]: https://github.com/BambooTracker/BambooTracker/issues/181

## v0.3.4 (2020-01-29)

### Added

- [#179] - Jump to previous/next order or pattern by forward/backward mouse button (thanks [@Midi-In])
- [#178] - View-based horizontal scroll in the order list and the pattern editor (thanks [@Midi-In])
- 1 new sample instrument bank

### Changed

- Change layout of the main window
- Modify some words in the tracker
- [#175] - Sound B7 when do key-on higher keys by keyboard (thanks [@papiezak])
- [#176] - Enable to show pattern step numbers in decimal (thanks [@Midi-In])
- Sound editing instrument in jamming
- Keep step position of the pattern cursor in changing the current order
- Unlock FM AR parameter when using SSG-EG
- Change default emulation core to Nuked OPN-Mod
- Update translation: ja

### Fixed

- [#172], [#173] - Check MIDI in api (thanks [@OPNA2608], [@jpcima], [@papiezak])
- Fix drawing of pattern editor and order list
- Fix jamming ignorance in FM & SSG editor using key layouts except QWERTY
- [#180] - Fix crash at startup (thanks [@papiezak], [@Ikalou])

[@Midi-In]: https://github.com/Midi-In
[@Ikalou]: https://github.com/Ikalou

[#172]: https://github.com/BambooTracker/BambooTracker/issues/172
[#173]: https://github.com/BambooTracker/BambooTracker/pull/173
[#175]: https://github.com/BambooTracker/BambooTracker/issues/175
[#176]: https://github.com/BambooTracker/BambooTracker/issues/176
[#179]: https://github.com/BambooTracker/BambooTracker/issues/179
[#178]: https://github.com/BambooTracker/BambooTracker/issues/178
[#180]: https://github.com/BambooTracker/BambooTracker/issues/180

## v0.3.3 (2019-12-17)

### Fixed

- [#170] - Fix selected instrument indexing in bank export (thanks [@Ravancloak], [@ImATrackMan])
- Update backup if it already exists
- [#170] - Copy command sequence type in deep clone instrument (thanks [@Ravancloak], [@ImATrackMan], [@galap-1])
- [#171] - Fix drawing error of the pattern editor when minimizing and showing the window during playback (thanks [@bryc])

[@Ravancloak]: https://github.com/Ravancloak

[#170]: https://github.com/BambooTracker/BambooTracker/issues/170
[#171]: https://github.com/BambooTracker/BambooTracker/issues/171

## v0.3.2 (2019-12-13)

### Changed

- Change drawing method of FM envelope in FM instrument editor
- Use Qt for file I/O
- Modify FM envelope graph in FM instrument editor

### Fixed

- Fix incorrect text painting in the pattern editor
- [#168] - Fix pattern rendering glitches (thanks [@OPNA2608], [@YuzuMSX])
- [#169] - Fix the order list corruption after changing font (thanks [@jimbo1qaz])
- Fix pattern jump calculation
- Change colors of the instrument list when appearance settings are changed
- Fix text position in row of the order list and the pattern editor

[@YuzuMSX]: https://github.com/YuzuMSX

[#168]: https://github.com/BambooTracker/BambooTracker/issues/168
[#169]: https://github.com/BambooTracker/BambooTracker/issues/169

## v0.3.1 (2019-11-30)

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

[#161]: https://github.com/BambooTracker/BambooTracker/issues/161
[#162]: https://github.com/BambooTracker/BambooTracker/issues/162
[#163]: https://github.com/BambooTracker/BambooTracker/issues/163
[#150]: https://github.com/BambooTracker/BambooTracker/issues/150
[#118]: https://github.com/BambooTracker/BambooTracker/issues/118
[#165]: https://github.com/BambooTracker/BambooTracker/issues/165
[#166]: https://github.com/BambooTracker/BambooTracker/issues/166
[#167]: https://github.com/BambooTracker/BambooTracker/issues/167

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

[#152]: https://github.com/BambooTracker/BambooTracker/pull/152
[#153]: https://github.com/BambooTracker/BambooTracker/issues/153
[#154]: https://github.com/BambooTracker/BambooTracker/issues/154
[#156]: https://github.com/BambooTracker/BambooTracker/issues/156
[#132]: https://github.com/BambooTracker/BambooTracker/issues/132
[#72]: https://github.com/BambooTracker/BambooTracker/issues/72
[#158]: https://github.com/BambooTracker/BambooTracker/issues/158
[#159]: https://github.com/BambooTracker/BambooTracker/issues/159
[#145]: https://github.com/BambooTracker/BambooTracker/issues/145

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

[#147]: https://github.com/BambooTracker/BambooTracker/pull/147
[#148]: https://github.com/BambooTracker/BambooTracker/pull/148
[#149]: https://github.com/BambooTracker/BambooTracker/issues/149
[#151]: https://github.com/BambooTracker/BambooTracker/issues/151

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

[#143]: https://github.com/BambooTracker/BambooTracker/issues/143
[#136]: https://github.com/BambooTracker/BambooTracker/issues/136
[#146]: https://github.com/BambooTracker/BambooTracker/issues/146

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

[#137]: https://github.com/BambooTracker/BambooTracker/issues/137
[#140]: https://github.com/BambooTracker/BambooTracker/issues/140
[#139]: https://github.com/BambooTracker/BambooTracker/pull/139
[#141]: https://github.com/BambooTracker/BambooTracker/pull/141
[#138]: https://github.com/BambooTracker/BambooTracker/issues/138
[#142]: https://github.com/BambooTracker/BambooTracker/pull/142

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

[#122]: https://github.com/BambooTracker/BambooTracker/issues/122
[#123]: https://github.com/BambooTracker/BambooTracker/pull/123
[#125]: https://github.com/BambooTracker/BambooTracker/pull/125
[#124]: https://github.com/BambooTracker/BambooTracker/pull/124
[#96]: https://github.com/BambooTracker/BambooTracker/issues/96
[#126]: https://github.com/BambooTracker/BambooTracker/issues/126
[#127]: https://github.com/BambooTracker/BambooTracker/issues/127
[#128]: https://github.com/BambooTracker/BambooTracker/issues/128
[#129]: https://github.com/BambooTracker/BambooTracker/issues/129
[#130]: https://github.com/BambooTracker/BambooTracker/issues/130
[#133]: https://github.com/BambooTracker/BambooTracker/pull/133
[#135]: https://github.com/BambooTracker/BambooTracker/issues/135
[#131]: https://github.com/BambooTracker/BambooTracker/issues/131
[#134]: https://github.com/BambooTracker/BambooTracker/issues/134

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

[#80]: https://github.com/BambooTracker/BambooTracker/pull/80
[#81]: https://github.com/BambooTracker/BambooTracker/issues/81
[#76]: https://github.com/BambooTracker/BambooTracker/pull/76
[#32]: https://github.com/BambooTracker/BambooTracker/issues/32
[#79]: https://github.com/BambooTracker/BambooTracker/issues/79
[#62]: https://github.com/BambooTracker/BambooTracker/issues/62
[#82]: https://github.com/BambooTracker/BambooTracker/issues/82
[#3]: https://github.com/BambooTracker/BambooTracker/issues/3
[#84]: https://github.com/BambooTracker/BambooTracker/issues/84
[#85]: https://github.com/BambooTracker/BambooTracker/pull/85
[#83]: https://github.com/BambooTracker/BambooTracker/issues/83
[#86]: https://github.com/BambooTracker/BambooTracker/issues/86
[#88]: https://github.com/BambooTracker/BambooTracker/issues/88
[#87]: https://github.com/BambooTracker/BambooTracker/issues/87
[#91]: https://github.com/BambooTracker/BambooTracker/issues/91
[#92]: https://github.com/BambooTracker/BambooTracker/issues/92
[#89]: https://github.com/BambooTracker/BambooTracker/pull/89
[#95]: https://github.com/BambooTracker/BambooTracker/pull/95
[#90]: https://github.com/BambooTracker/BambooTracker/pull/90
[#94]: https://github.com/BambooTracker/BambooTracker/pull/94
[#93]: https://github.com/BambooTracker/BambooTracker/issues/93
[#97]: https://github.com/BambooTracker/BambooTracker/pull/97
[#101]: https://github.com/BambooTracker/BambooTracker/issues/101
[#100]: https://github.com/BambooTracker/BambooTracker/issues/100
[#103]: https://github.com/BambooTracker/BambooTracker/pull/103
[#104]: https://github.com/BambooTracker/BambooTracker/pull/104
[#105]: https://github.com/BambooTracker/BambooTracker/pull/105
[#99]: https://github.com/BambooTracker/BambooTracker/pull/99
[#98]: https://github.com/BambooTracker/BambooTracker/issues/98
[#106]: https://github.com/BambooTracker/BambooTracker/pull/106
[#107]: https://github.com/BambooTracker/BambooTracker/pull/107
[#108]: https://github.com/BambooTracker/BambooTracker/pull/108
[#109]: https://github.com/BambooTracker/BambooTracker/pull/109
[#111]: https://github.com/BambooTracker/BambooTracker/pull/111
[#110]: https://github.com/BambooTracker/BambooTracker/pull/110
[#114]: https://github.com/BambooTracker/BambooTracker/pull/114
[#113]: https://github.com/BambooTracker/BambooTracker/issues/113
[#115]: https://github.com/BambooTracker/BambooTracker/issues/115
[#116]: https://github.com/BambooTracker/BambooTracker/pull/116
[#117]: https://github.com/BambooTracker/BambooTracker/pull/117
[#119]: https://github.com/BambooTracker/BambooTracker/issues/119
[#112]: https://github.com/BambooTracker/BambooTracker/issues/112
[#121]: https://github.com/BambooTracker/BambooTracker/pull/121

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

[#61]: https://github.com/BambooTracker/BambooTracker/pull/61
[#63]: https://github.com/BambooTracker/BambooTracker/pull/63
[#64]: https://github.com/BambooTracker/BambooTracker/pull/64
[#66]: https://github.com/BambooTracker/BambooTracker/issues/66
[#67]: https://github.com/BambooTracker/BambooTracker/issues/67
[#68]: https://github.com/BambooTracker/BambooTracker/issues/68
[#69]: https://github.com/BambooTracker/BambooTracker/issues/69
[#65]: https://github.com/BambooTracker/BambooTracker/issues/65
[#71]: https://github.com/BambooTracker/BambooTracker/issues/71
[#73]: https://github.com/BambooTracker/BambooTracker/issues/73
[#70]: https://github.com/BambooTracker/BambooTracker/issues/70
[#11]: https://github.com/BambooTracker/BambooTracker/issues/11
[#75]: https://github.com/BambooTracker/BambooTracker/issues/75
[#78]: https://github.com/BambooTracker/BambooTracker/pull/78
[#53]: https://github.com/BambooTracker/BambooTracker/issues/53

[@ImATrackMan]: https://github.com/ImATrackMan
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

[#48]: https://github.com/BambooTracker/BambooTracker/issues/48
[#51]: https://github.com/BambooTracker/BambooTracker/issues/51
[#52]: https://github.com/BambooTracker/BambooTracker/pull/52
[#54]: https://github.com/BambooTracker/BambooTracker/pull/54
[#55]: https://github.com/BambooTracker/BambooTracker/issues/55
[#56]: https://github.com/BambooTracker/BambooTracker/pull/56
[#57]: https://github.com/BambooTracker/BambooTracker/issues/57
[#44]: https://github.com/BambooTracker/BambooTracker/issues/44
[#59]: https://github.com/BambooTracker/BambooTracker/issues/59
[#58]: https://github.com/BambooTracker/BambooTracker/issues/58
[#60]: https://github.com/BambooTracker/BambooTracker/issues/60

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

[#27]: https://github.com/BambooTracker/BambooTracker/issues/27
[#28]: https://github.com/BambooTracker/BambooTracker/issues/28
[#29]: https://github.com/BambooTracker/BambooTracker/pull/29
[#30]: https://github.com/BambooTracker/BambooTracker/pull/30
[#33]: https://github.com/BambooTracker/BambooTracker/pull/33
[#36]: https://github.com/BambooTracker/BambooTracker/pull/36
[#31]: https://github.com/BambooTracker/BambooTracker/pull/31
[#17]: https://github.com/BambooTracker/BambooTracker/issues/17
[#37]: https://github.com/BambooTracker/BambooTracker/pull/37
[#38]: https://github.com/BambooTracker/BambooTracker/pull/38
[#39]: https://github.com/BambooTracker/BambooTracker/pull/39
[#41]: https://github.com/BambooTracker/BambooTracker/pull/41
[#13]: https://github.com/BambooTracker/BambooTracker/issues/13
[#40]: https://github.com/BambooTracker/BambooTracker/issues/40
[#45]: https://github.com/BambooTracker/BambooTracker/pull/45
[#46]: https://github.com/BambooTracker/BambooTracker/issues/46
[#47]: https://github.com/BambooTracker/BambooTracker/pull/47

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

[#25]: https://github.com/BambooTracker/BambooTracker/issues/25
[#14]: https://github.com/BambooTracker/BambooTracker/issues/14
[#18]: https://github.com/BambooTracker/BambooTracker/issues/18
[#24]: https://github.com/BambooTracker/BambooTracker/issues/24
[#26]: https://github.com/BambooTracker/BambooTracker/issues/26

[@papiezak]: https://github.com/papiezak

## v0.1.2 (2018-12-02)

### Added

- [#2], [#6] - Add selection whether the order of FM volume is reversed (thanks [@pedipanol], [@ScoopJohn])
- [#22] - Add .dmp, .tfi and .vgi instrument file importing support ([#2], [#6]; thanks [@Pigu-A])
- [#12] - Add order/pattern focus shortcut (thanks [@jimbo1qaz])

### Fixed

- Fix module load error by missing pattern size initialization (thanks [@maakmusic])
- [#10] - Rearrange tab stops for better keyboard navigation (thanks [@Pigu-A])
- [#16] - Change envelope reset behavior (thanks [@Pigu-A])
- Fix instrument property used/unused inheritance by cloning
- [#20] - Split FM envelope copy/paste to multiple lines (thanks [@jimbo1qaz])
- [#21] - Fix selection range in pattern editor (thanks [@jimbo1qaz])
- [#1] - Make compatible with WinXP (thanks [@JonKaruzu])
- [#2] - Fix the order of input for columns (thanks [@pedipanol])
- [#23] - Fix tick event to be loaded from playing step (thanks [@pedipanol])
- Fix play/stop song when pressing return key
- [#12] - Fix insert/clone order selection when clicking above or below orders (thanks [@jimbo1qaz])
- Fix clone order to assign first free patterns

[#10]: https://github.com/BambooTracker/BambooTracker/pull/10
[#16]: https://github.com/BambooTracker/BambooTracker/pull/16
[#20]: https://github.com/BambooTracker/BambooTracker/pull/20
[#21]: https://github.com/BambooTracker/BambooTracker/issues/21
[#1]: https://github.com/BambooTracker/BambooTracker/issues/1
[#2]: https://github.com/BambooTracker/BambooTracker/issues/2
[#6]: https://github.com/BambooTracker/BambooTracker/issues/6
[#23]: https://github.com/BambooTracker/BambooTracker/issues/23
[#22]: https://github.com/BambooTracker/BambooTracker/pull/22
[#12]: https://github.com/BambooTracker/BambooTracker/issues/12

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

[#5]: https://github.com/BambooTracker/BambooTracker/issues/5
[#4]: https://github.com/BambooTracker/BambooTracker/issues/4
[#9]: https://github.com/BambooTracker/BambooTracker/pull/9
[#7]: https://github.com/BambooTracker/BambooTracker/issues/7
[#8]: https://github.com/BambooTracker/BambooTracker/issues/8

[@Xyz39808]: https://github.com/Xyz39808
[@9001]: https://github.com/9001
[@OPNA2608]: https://github.com/OPNA2608
[@marysiamzawka]: https://github.com/marysiamzawka
[@nabetaqz]: https://twitter.com/nabetaqz

## v0.1.0 (2018-11-25)

### Added

- First release
