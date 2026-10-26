#!/usr/bin/env python3
# Draw the Wall Clock application icon and write icons/wall_clock.ico
# and a 256 x 256 icons/wall_clock.png for the README.
#
# The icon is drawn here from simple shapes, so it is original artwork
# with no third-party image. Needs Pillow (python3 -m pip install pillow).
#
# Usage: python3 tools/make_icon.py [output.ico [output.png]]
#
# License: GNU General Public License version 3 or later, see LICENSE.

import math
import os
import sys

from PIL import Image, ImageDraw

SIZES = [16, 20, 24, 32, 40, 48, 64, 96, 128, 256]
SCALE = 4  # draw large and shrink, for smooth edges

RIM = (32, 40, 56, 255)
FACE = (250, 250, 247, 255)
TICK = (32, 40, 56, 255)
HAND = (32, 40, 56, 255)
SECOND = (220, 30, 30, 255)


def line(draw, cx, cy, angle, r0, r1, width, color):
    # angle in degrees, clockwise from 12 o'clock
    a = math.radians(angle)
    x0, y0 = cx + r0 * math.sin(a), cy - r0 * math.cos(a)
    x1, y1 = cx + r1 * math.sin(a), cy - r1 * math.cos(a)
    draw.line((x0, y0, x1, y1), fill=color, width=max(1, round(width)))
    # round the ends
    for x, y in ((x0, y0), (x1, y1)):
        h = width / 2
        draw.ellipse((x - h, y - h, x + h, y + h), fill=color)


def draw_clock(size):
    n = size * SCALE
    img = Image.new("RGBA", (n, n), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    c = n / 2
    r = n * 0.47
    small = size <= 24

    # rim and face
    d.ellipse((c - r, c - r, c + r, c + r), fill=RIM)
    rf = r * (0.84 if small else 0.88)
    d.ellipse((c - rf, c - rf, c + rf, c + rf), fill=FACE)

    # hour ticks, only the four main ones on the smallest sizes
    for i in range(12):
        major = i % 3 == 0
        if small and not major:
            continue
        length = rf * (0.20 if major else 0.11)
        width = n * (0.050 if major else 0.025)
        line(d, c, c, i * 30, rf * 0.90 - length, rf * 0.90, width, TICK)

    # hands at 10:10, the classic clock pose
    line(d, c, c, 10 * 30 + 10 * 0.5, 0, rf * 0.52, n * 0.075, HAND)  # hour
    line(d, c, c, 10 * 6, 0, rf * 0.78, n * 0.055, HAND)  # minute
    if size >= 32:
        line(d, c, c, 37 * 6, -rf * 0.18, rf * 0.80, n * 0.022, SECOND)
    hub = n * (0.06 if size >= 32 else 0.045)
    d.ellipse((c - hub, c - hub, c + hub, c + hub),
              fill=SECOND if size >= 32 else HAND)

    return img.resize((size, size), Image.LANCZOS)


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    icons = os.path.join(here, "..", "icons")
    out = sys.argv[1] if len(sys.argv) > 1 else \
        os.path.join(icons, "wall_clock.ico")
    png = sys.argv[2] if len(sys.argv) > 2 else \
        os.path.join(icons, "wall_clock.png")
    images = [draw_clock(s) for s in SIZES]
    images[-1].save(out, format="ICO", sizes=[(s, s) for s in SIZES],
                    append_images=images[:-1])
    print("Wrote", out)
    images[-1].save(png)
    print("Wrote", png)


if __name__ == "__main__":
    main()
