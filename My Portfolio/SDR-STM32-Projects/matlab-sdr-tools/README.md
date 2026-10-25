# MATLAB SDR Tools

MATLAB was used to implement carrier and timing synchronization based on *Software-Defined Radio for Engineers*.
Hardware testing was done with ADALM-Pluto.

## Carrier and Timing Recovery

The carrier synchronization script uses an M-th-power FFT estimate for coarse frequency correction. A Costas-style
loop is then used for fine correction. The included example uses 4-QAM.

The timing script uses fractional-delay interpolation, a zero-crossing timing-error detector, and a feedback loop to
adjust the sample timing. Its example uses 16-QAM.

The included scripts generate synthetic signals. Pluto acquisition code and recorded datasets are not included.

## Files

- [Coarse and fine carrier synchronization](matlab/coarse_and_fine_carrier_synchronization.m)
- [Symbol timing synchronization](matlab/symbol_timing_synchronization.m)

The scripts use Communications Toolbox, DSP System Toolbox, and Signal Processing Toolbox. The timing script
sweeps many loop-bandwidth settings. The unused `FineFreqSyncFaster` helper still needs cleanup; the example calls
`FineFreqSync`.

## Reference

T. F. Collins, R. Getz, D. Pu, and A. M. Wyglinski, *Software-Defined Radio for Engineers*, Artech House, 2018.
[Book resource](https://www.analog.com/en/resources/technical-books/software-defined-radio-for-engineers.html).

[SDR and STM32 projects](../README.md) | [Portfolio](../../README.md)
