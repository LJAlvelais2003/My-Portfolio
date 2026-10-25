# MATLAB Software-Defined Radio Tools

MATLAB experiments implementing receiver synchronization concepts studied in *Software-Defined Radio for
Engineers*. Leo Alvelais used Analog Devices ADALM-Pluto hardware for the associated tests.

**Status:** Two MATLAB development snapshots are included. Their top-level demonstrations generate synthetic QAM
signals; live-radio acquisition code and recorded Pluto datasets are not part of this package.

## Implemented Topics

| File | Main functions and experiment |
| --- | --- |
| [coarse_and_fine_carrier_synchronization.m](matlab/coarse_and_fine_carrier_synchronization.m) | M-th-power / FFT coarse frequency estimation, complex frequency correction, and a Costas-style fine carrier recovery loop for a 4-QAM example |
| [symbol_timing_synchronization.m](matlab/symbol_timing_synchronization.m) | Fractional-delay interpolation, zero-crossing timing-error detection, loop filtering, and interpolation control in a 16-QAM example |

## Tools and Use

The scripts use MATLAB, Communications Toolbox, DSP System Toolbox, and functions such as `upfirdn` from
Signal Processing Toolbox. ADALM-Pluto is the hardware platform for the associated experiments; the included
synthetic demonstrations do not establish a live radio connection.

Open each script to inspect the signal generation, synchronization functions, and constellation displays. The timing
script contains a 10,000-iteration loop-bandwidth exploration and updates a constellation display each iteration.
Select a smaller experiment when inspecting individual loop settings.

The `FineFreqSyncFaster` helper is an unused development variant and still references filter objects that are not
defined inside that function. The top-level frequency example calls `FineFreqSync` instead. These files are shared
as the supplied experimental snapshots, rather than as a packaged receiver library.

## Learning Reference

T. F. Collins, R. Getz, D. Pu, and A. M. Wyglinski, *Software-Defined Radio for Engineers*. Artech House, 2018.
[Official book resource](https://www.analog.com/en/resources/technical-books/software-defined-radio-for-engineers.html).

The project applies concepts from the book to MATLAB experiments and ADALM-Pluto testing. The book is credited
as the learning reference; an independent implementation of every underlying algorithm is not claimed.

[Wireless systems and SDR](../README.md) | [Portfolio](../../README.md)
