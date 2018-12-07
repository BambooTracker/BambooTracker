# Changelog

## Unreleased
### Added
- [#18] - Add FM operator copy/paste (thanks [@jimbo1qaz])

### Fixed
- Fix default place of load/save instrument
- [#25] - Change FM synthesis frequency from 110933Hz to 55466Hz (thanks [@papiezak])
- [#6], [#14] - Fix to set new property when adding instrument (thanks [@ScoopJohn], [@jimbo1qaz])
- [#2] - Fix wheel step in slider (thanks [@pedipanol])
- Fix file load/save to use local encoding (thanks [@maakmusic])

[#25]: https://github.com/rerrahkr/BambooTracker/issues/25
[#14]: https://github.com/rerrahkr/BambooTracker/issues/14
[#18]: https://github.com/rerrahkr/BambooTracker/issues/18

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
- First release.
