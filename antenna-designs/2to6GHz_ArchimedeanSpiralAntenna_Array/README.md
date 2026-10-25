# Archimedean Spiral Array

Two 3 x 3 Archimedean spiral arrays were studied using vertically integrated Klopfenstein and exponential tapered
baluns. The work compares broadband matching, element coupling, and the effect of feed geometry on center and
corner element patterns.

**Status:** Paper and HFSS archive available. The paper documents fabricated arrays and measured input matching.

![Fabricated Klopfenstein-fed spiral array](figures/array-prototype.png)

## Design

Each two-arm spiral is fed through a vertically mounted tapered balun. The baluns transform a 50 ohm unbalanced
input toward the approximately 150 ohm balanced spiral impedance. The elements use Rogers RO4350B, a square
lattice with 54.6 mm spacing, and a backing conductor. Full-wave optimization was used to refine the feed geometry.

![HFSS model of the Klopfenstein-fed array](figures/array-model.png)

## Reported Results

| Quantity | Klopfenstein feed | Exponential feed | Evidence |
| --- | --- | --- | --- |
| Frequency coverage meeting S11 at or below -10 dB | Approximately 2.1-5.0 GHz | Approximately 2.0-2.89 GHz and 3.02-5.0 GHz | Measured input matching reported in the paper |
| Average center-to-edge isolation over 2-5 GHz | 18.9 dB | 21.6 dB | Simulated coupling |
| Average center-to-corner isolation over 2-5 GHz | 22.4 dB | 23.6 dB | Simulated coupling |

The feeds produced similar simulated center-element patterns, while corner-element behavior showed the influence
of the feed and finite-array edge effects. The exponential feed provided higher average isolation in the reported
comparison, while the Klopfenstein feed provided more continuous measured impedance coverage.

![Simulated and measured input matching for both balun profiles](figures/input-matching.png)

## Files and Use

- [Project paper](docs/spiral-array-paper.pdf)
- [Ansys project archive](models/ArchimedeanSpiralArrayExport_Fall2026.aedtz)
- [Simulated element coupling](figures/simulated-coupling.png)
- [Center-element pattern](figures/center-element-pattern.png)
- [Corner-element pattern](figures/corner-element-pattern.png)

Open or restore the `.aedtz` archive using Ansys Electronics Desktop / HFSS and select a local working directory.
The compressed archive is retained as supplied; its contained `.aedt` file is much larger. The paper and included
figures provide results without requiring an HFSS installation. No new simulations or measurements were run to
prepare this portfolio.

## Credits

The supplied paper, *Archimedean Spiral Array With Vertically Integrated Klopfenstein and Exponential Baluns*,
lists Leo Alvelais, Paul Obrecht, Payam Nayeri, and Steve Dunton. Results and figures above are taken from that
paper. Publication venue and publication status are not specified here.

[All antenna designs](../README.md) | [Portfolio](../../README.md)
