# BambooTracker Instrument File (.bti) Format Specification

v1.5.1 - 2022-11-06

This is a version revision to switch the loading process for files from previous versions due to changes in instrument sequence behaviour. The file specification is exactly the same as [v1.5.0](./archives/inst_specs_v1.5.0.md).

---

## History

| Version | Date       | Detail                                                                                          |
| ------- | ---------- | ----------------------------------------------------------------------------------------------- |
| 1.5.1   | 2022-11-06 | Revised to change the behaviour of instrument sequence.                                         |
| 1.5.0   | 2021-06-13 | Added FM/ADPCM panning sequence, drumkit panning, and removed unused subdata of ADPCM envelope. |
| 1.4.0   | 2020-04-28 | Added ADPCM drumkit instrument.                                                                 |
| 1.3.0   | 2020-02-25 | Added ADPCM instrument.                                                                         |
| 1.2.3   | 2019-12-16 | Revised to fix the deep copy of instrument sequence types.                                      |
| 1.2.2   | 2019-11-09 | Reversed SSG noise pitch order.                                                                 |
| 1.2.1   | 2019-06-07 | Revised to fix unit data skipping bug of FM operator sequence.                                  |
| 1.2.0   | 2019-04-10 | Added and changed for SSG tone/hard or square-mask ratio settings.                              |
| 1.1.0   | 2019-03-24 | Added fields for FM3ch expanded mode.                                                           |
| 1.0.1   | 2018-12-10 | Added instrument sequence type.                                                                 |
| 1.0.0   | 2018-11-23 | Initial release.                                                                                |
