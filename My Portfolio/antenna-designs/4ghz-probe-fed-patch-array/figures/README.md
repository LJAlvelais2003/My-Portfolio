# Taylor Aperture Figures

Figure 1 shows the sampled circular Taylor amplitude distribution. Figure 2 shows the corresponding ideal principal-plane array factor. The web preview places the two figures alongside each other; their numbered print exports are separate.

Both figures are analytical recalculations from the equations and parameters in [circular_taylor_array_factor.m](../matlab/circular_taylor_array_factor.m): a 20 x 20 lattice, 0.5-wavelength spacing, a 5-wavelength aperture radius, nbar = 4, and a 20 dB sidelobe design parameter. Cells are selected using their outer corners, and weights are normalized to the largest sampled value.

The calculations exclude element-pattern effects, mutual coupling, feed losses, and full-wave optimization. The dashed -20 dB line is an input design parameter, not a statement about final antenna sidelobes.

## Print Files

| Figure | PDF | EPS | TIFF |
| --- | --- | --- | --- |
| Figure 1: aperture weights | [PDF](print/FIG1.pdf) | [EPS](print/FIG1.eps) | [TIFF](print/FIG1.tif) |
| Figure 2: principal cut | [PDF](print/FIG2.pdf) | [EPS](print/FIG2.eps) | [TIFF](print/FIG2.tif) |

The plots use 8-point Arial, a color amplitude scale, 0.75-point axes, and a 1-point blue curve. The print size is 3.5 by 3 inches per figure. PDF and EPS fonts are embedded; TIFF files are RGB color at 600 dpi. Captions are outside the plotted graphics.

## Reproduce

Install [the Python dependencies](../analysis/requirements.txt) and make Arial or Times New Roman available. Run [generate_aperture_preview.py](../analysis/generate_aperture_preview.py), optionally passing `--font-file` with the path to the font file. The script checks weight symmetry and verifies the plotted cut against a direct two-dimensional sum. MATLAB, HFSS, and hardware were not run for these figures.

[Back to array project](../README.md)
