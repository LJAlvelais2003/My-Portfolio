"""Plot sampled and continuous Taylor amplitudes from the supplied MATLAB equations.

This is an analytical calculation, not an HFSS result or a measurement.
The existing MATLAB files and HFSS model are not modified.
"""
from pathlib import Path
import argparse
import json

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from scipy.special import j0, jn_zeros

from generate_aperture_preview import calculate, configure_style


def continuous_weights(x, y):
    radius, nbar, sidelobe_db = 5.0, 4, 20.0
    gamma = jn_zeros(1, nbar) / np.pi
    a = np.arccosh(10 ** (sidelobe_db / 20)) / np.pi
    n = np.arange(1, nbar)
    un = gamma[nbar - 1] * np.sqrt(
        (a * a + (n - 0.5) ** 2) / (a * a + (nbar - 0.5) ** 2)
    )
    p = np.pi * np.hypot(x, y) / radius
    field = np.full_like(p, 0.5)
    for i in range(nbar - 1):
        numerator = np.prod(1 - (gamma[i] / un) ** 2)
        denominator = np.prod([
            1 - (gamma[i] / gamma[m]) ** 2
            for m in range(nbar - 1) if m != i
        ])
        coefficient = -0.5 * j0(np.pi * gamma[i]) * numerator / denominator
        field += coefficient / j0(np.pi * gamma[i]) ** 2 * j0(gamma[i] * p)
    return 2 / np.pi ** 2 * field


def render(font_file=None):
    configure_style(font_file)
    centers, weights, inside, _, _ = calculate()
    assert int(inside.sum()) == 276
    assert np.count_nonzero(weights[~inside]) == 0
    x, y = np.meshgrid(centers, centers)
    sampled_check = continuous_weights(x, y)
    sampled_check[~inside] = 0
    sampled_check /= sampled_check.max()
    assert np.allclose(weights, sampled_check, atol=1e-12)

    dense = np.linspace(-5, 5, 401)
    xc, yc = np.meshgrid(dense, dense)
    # Include mathematical boundary points despite floating-point roundoff.
    continuous_inside = np.hypot(xc, yc) <= 5.0 + 1e-12
    continuous = continuous_weights(xc, yc)
    continuous[~continuous_inside] = 0
    continuous /= continuous.max()
    assert np.allclose(continuous, continuous[::-1, :], atol=1e-12)
    assert np.allclose(continuous, continuous[:, ::-1], atol=1e-12)

    cmap = plt.get_cmap("viridis").copy()
    cmap.set_bad("white")
    fig, axes = plt.subplots(1, 2, figsize=(7.4, 3.4))
    edges = np.linspace(-5, 5, 21)
    left = axes[0].pcolormesh(
        edges, edges, np.ma.masked_where(~inside, weights),
        cmap=cmap, vmin=0, vmax=1, shading="flat",
        edgecolors="face", linewidth=0.1,
    )
    right = axes[1].imshow(
        np.ma.masked_where(~continuous_inside, continuous),
        origin="lower", extent=(-5, 5, -5, 5), cmap=cmap,
        vmin=0, vmax=1, interpolation="nearest",
    )
    axes[0].set_title("Discrete Taylor Aperture Distribution")
    axes[1].set_title("Continuous Taylor Aperture Distribution")
    for ax, image in zip(axes, [left, right]):
        ax.set_aspect("equal")
        ax.set(xlim=(-5, 5), ylim=(-5, 5), xlabel="x/\u03bb", ylabel="y/\u03bb")
        ax.set_xticks([-4, -2, 0, 2, 4])
        ax.set_yticks([-4, -2, 0, 2, 4])
        fig.colorbar(image, ax=ax, fraction=0.046, pad=0.035, ticks=[0, 0.5, 1])
    fig.subplots_adjust(left=0.065, right=0.955, bottom=0.16, top=0.90, wspace=0.40)
    output = Path(__file__).resolve().parents[1] / "figures/taylor-amplitude-distribution.png"
    fig.savefig(output, dpi=360, facecolor="white")
    plt.close(fig)
    print(json.dumps({"active_elements": int(inside.sum()), "outside_weights": 0,
                      "sampled_formula_check": "passed", "symmetry_checks": "passed",
                      "output": str(output)}))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--font-file", type=Path)
    args = parser.parse_args()
    render(args.font_file)
