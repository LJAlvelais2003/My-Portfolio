"""Recalculate the ideal circular Taylor aperture study from the supplied MATLAB parameters.

This figure is analytical. It is not an HFSS result or an antenna measurement.
Dependencies: numpy, scipy, matplotlib.
"""
from pathlib import Path
import json

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from scipy.special import j0, jn_zeros


def calculate():
    count = 20
    spacing = 0.5
    radius = 5.0
    sidelobe_parameter_db = 20.0
    nbar = 4
    centers = (np.arange(count) - (count - 1) / 2) * spacing
    x, y = np.meshgrid(centers, centers)
    inside = np.hypot(np.abs(x) + spacing / 2, np.abs(y) + spacing / 2) <= radius

    gamma = jn_zeros(1, nbar) / np.pi
    a = np.arccosh(10 ** (sidelobe_parameter_db / 20)) / np.pi
    n = np.arange(1, nbar)
    un = gamma[nbar - 1] * np.sqrt((a * a + (n - 0.5) ** 2) /
                                  (a * a + (nbar - 0.5) ** 2))
    p = np.pi * np.hypot(x, y) / radius
    field = np.full_like(p, 0.5)
    for i in range(nbar - 1):
        numerator = np.prod(1 - (gamma[i] / un) ** 2)
        denominator = np.prod([1 - (gamma[i] / gamma[m]) ** 2
                               for m in range(nbar - 1) if m != i])
        coefficient = -0.5 * j0(np.pi * gamma[i]) * numerator / denominator
        field += coefficient / j0(np.pi * gamma[i]) ** 2 * j0(gamma[i] * p)
    weights = 2 / np.pi ** 2 * field
    weights[~inside] = 0
    weights /= weights.max()

    theta = np.linspace(-90, 90, 3601)
    u = np.sin(np.deg2rad(theta))
    af = weights.sum(axis=0) @ np.exp(2j * np.pi * np.outer(centers, u))
    pattern = 20 * np.log10(np.maximum(np.abs(af) / np.max(np.abs(af)), 1e-6))
    assert np.allclose(weights, weights[::-1, :])
    assert np.allclose(weights, weights[:, ::-1])
    assert np.allclose(weights, weights.T)
    assert np.isclose(pattern[len(theta) // 2], 0)
    for index in range(0, len(theta), 211):
        direct = np.sum(weights * np.exp(2j * np.pi * x * u[index]))
        assert np.isclose(direct, af[index], atol=1e-10)
    return centers, weights, inside, theta, pattern



import argparse
import os
from matplotlib import font_manager
from PIL import Image


def configure_style(font_file=None):
    font_file = font_file or os.environ.get('FIGURE_FONT_PATH')
    if font_file:
        font_manager.fontManager.addfont(str(font_file))
        family = font_manager.FontProperties(fname=font_file).get_name()
    else:
        family = None
        for candidate in ['Arial', 'Times New Roman']:
            try:
                font_manager.findfont(font_manager.FontProperties(family=candidate), fallback_to_default=False)
                family = candidate
                break
            except ValueError:
                continue
        if family is None:
            raise RuntimeError('Install Arial or Times New Roman, or pass --font-file with the font file path.')
    plt.rcParams.update({
        'font.family': family, 'font.size': 8, 'font.weight': 'normal',
        'axes.labelsize': 8, 'axes.labelweight': 'normal', 'axes.titlesize': 8,
        'xtick.labelsize': 8, 'ytick.labelsize': 8, 'legend.fontsize': 8,
        'axes.linewidth': 0.75, 'axes.edgecolor': 'black', 'axes.labelcolor': 'black',
        'axes.facecolor': 'white', 'figure.facecolor': 'white', 'savefig.facecolor': 'white',
        'axes.unicode_minus': False, 'text.color': 'black',
        'xtick.color': 'black', 'ytick.color': 'black',
        'xtick.major.width': 0.5, 'ytick.major.width': 0.5,
        'xtick.major.size': 2.5, 'ytick.major.size': 2.5,
        'pdf.fonttype': 42, 'ps.fonttype': 42,
    })
    return family


def export(fig, directory, number):
    directory.mkdir(parents=True, exist_ok=True)
    stem = directory / f'FIG{number}'
    fig.savefig(stem.with_suffix('.pdf'))
    fig.savefig(stem.with_suffix('.eps'))
    png = stem.with_suffix('.png')
    fig.savefig(png, dpi=600)
    with Image.open(png) as im:
        color = im.convert('RGB')
        color.save(png, dpi=(600, 600), optimize=True)
        color.save(stem.with_suffix('.tif'), dpi=(600, 600), compression='tiff_lzw')
    plt.close(fig)
    return png


def render(output, font_file=None):
    family = configure_style(font_file)
    centers, weights, inside, theta, pattern = calculate()
    print_dir = output.parent / 'print'

    # Figure 1: color aperture. Figure caption is supplied in the README.
    fig, ax = plt.subplots(figsize=(3.5, 3.0))
    edges = np.linspace(-5, 5, 21)
    cmap = plt.get_cmap('viridis').copy()
    cmap.set_bad('white')
    image = ax.pcolormesh(edges, edges, np.ma.masked_where(~inside, weights),
                         cmap=cmap, vmin=0, vmax=1, shading='flat', edgecolors='face', linewidth=0.1)
    ax.set_aspect('equal')
    ax.set(xlim=(-5, 5), ylim=(-5, 5), xlabel='x, wavelengths', ylabel='y, wavelengths')
    ax.set_xticks([-4, -2, 0, 2, 4])
    ax.set_yticks([-4, -2, 0, 2, 4])
    bar = fig.colorbar(image, ax=ax, fraction=0.045, pad=0.04, ticks=[0, 0.5, 1])
    bar.set_label('Normalized amplitude')
    bar.outline.set_linewidth(0.75)
    bar.solids.set_rasterized(False)
    bar.solids.set_edgecolor('face')
    fig.subplots_adjust(left=0.16, right=0.82, bottom=0.16, top=0.97)
    first = export(fig, print_dir, 1)

    # Figure 2: blue line; the dashed reference is explained in the caption.
    fig, ax = plt.subplots(figsize=(3.5, 3.0))
    ax.plot(theta, pattern, color='#0072BD', linewidth=1.0)
    ax.axhline(-20, color='0.4', linestyle='--', linewidth=0.75)
    ax.set(xlim=(-90, 90), ylim=(-60, 0), xlabel='Broadside angle, deg',
           ylabel='Normalized array factor, dB')
    ax.set_xticks(np.arange(-90, 91, 30))
    ax.set_yticks(np.arange(-60, 1, 10))
    ax.grid(color='0.82', linewidth=0.5, linestyle=':')
    ax.set_axisbelow(True)
    fig.subplots_adjust(left=0.18, right=0.97, bottom=0.16, top=0.97)
    second = export(fig, print_dir, 2)

    # Web preview joins the two independent print figures without adding headings.
    with Image.open(first) as one, Image.open(second) as two:
        gap = 48
        preview = Image.new('RGB', (one.width + two.width + gap, max(one.height, two.height)), 'white')
        preview.paste(one, (0, 0))
        preview.paste(two, (one.width + gap, 0))
        output.parent.mkdir(parents=True, exist_ok=True)
        preview.save(output, dpi=(600, 600), optimize=True)
    print(json.dumps({'font': family, 'font_points': 8, 'active_elements': int(inside.sum()),
                      'print_figures': 2, 'print_formats': ['pdf', 'eps', 'tif', 'png'],
                      'print_size_inches': [3.5, 3.0], 'raster_dpi': 600, 'output': str(output)}))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--font-file', type=Path)
    args = parser.parse_args()
    render(Path(__file__).resolve().parents[1] / 'figures' / 'taylor-aperture-study.png', args.font_file)
