#!/usr/bin/env python3
"""
Generate schematic, perfboard wiring diagram, and simple waveform plots for the
V4.2.2 fan → 5 V logic PWM transistor adapter.

Run from repo:  python3 docs/laser-conversion/fan-transistor-adapter/generate_diagrams.py
Requires: cairosvg, pillow, numpy, reportlab (optional for PDF).
"""
from __future__ import annotations

import html
import math
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent


def _svg_wrap(body: str, w: int, h: int) -> str:
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" '
        f'viewBox="0 0 {w} {h}">'
        f'<rect width="{w}" height="{h}" fill="white"/>'
        f'<g font-family="DejaVu Sans,Arial,sans-serif">{body}</g></svg>'
    )


def _t(x, y, text, size=16, color="#16364a", bold=False, anchor="start"):
    w = "bold" if bold else "normal"
    return (
        f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{w}" '
        f'fill="{color}" text-anchor="{anchor}">{html.escape(text)}</text>'
    )


def _line(x1, y1, x2, y2, color="#294d65", w=2.5):
    return f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" stroke-width="{w}"/>'


def _rect(x, y, w, h, stroke="#35566b", fill="none", sw=2, rx=6):
    return (
        f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" '
        f'stroke="{stroke}" stroke-width="{sw}" fill="{fill}"/>'
    )


def _circ(cx, cy, r, fill="#294d65", stroke="none", sw=0):
    return f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>'


def build_schematic_svg() -> str:
    parts = []
    parts.append(_t(40, 42, "Creality V4.2.2 FAN (PA0) → 5 V TTL adapter (non-inverting)", 26, bold=True))
    parts.append(_t(40, 70, "Fan ON (FAN− ≈ 0 V) → OUT ≈ 5 V  |  Fan OFF (FAN− ≈ 24 V) → OUT ≈ 0 V  |  laser disconnected for bench", 15))

    # Left: board / load
    parts.append(_rect(40, 100, 280, 320, "#ac5c00", "#fff8f0", 3))
    parts.append(_t(55, 130, "K-FAN1 or K-FAN2 (shared PA0)", 15, "#ac5c00", True))
    parts.append(_t(55, 158, "FAN+  (fixed ≈ 24 V)", 14))
    parts.append(_t(55, 186, "FAN−  (switched drain)", 14))
    parts.append(_t(55, 220, "Board / PSU DC−  =  common GND", 14, "#0867a5", True))
    parts.append(_t(55, 255, "Rload 2.2 kΩ 1 W", 14, bold=True))
    parts.append(_t(55, 278, "across FAN+ → FAN−", 13))
    parts.append(_t(55, 302, "I≈10.9 mA  P≈0.26 W @ 24 V ON", 13))
    parts.append(_t(55, 335, "Optional Dfly (UF4007)", 13))
    parts.append(_t(55, 355, "cathode→FAN+ if fan remains", 12))
    parts.append(_t(55, 390, "Do NOT invent K-FAN pin order:", 12, "#b32727"))
    parts.append(_t(55, 408, "verify +/− from silk / meter", 12, "#b32727"))

    # Middle: sense + Q1
    parts.append(_rect(360, 100, 340, 320, "#35566b", "#f5f8fa", 3))
    parts.append(_t(375, 130, "Sense + invert (2N3904)", 16, bold=True))
    parts.append(_t(375, 160, "FAN− ── R1 47 kΩ ──●── R2 10 kΩ ── GND", 13))
    parts.append(_t(375, 185, "                  │", 13))
    parts.append(_t(375, 210, "                  Rb 4.7 kΩ", 13))
    parts.append(_t(375, 235, "                  │", 13))
    parts.append(_t(375, 260, "                  B of Q1 (NPN)", 13))
    parts.append(_t(375, 290, "Q1: 2N3904 / BC547", 14, bold=True))
    parts.append(_t(375, 315, "E → GND   C → OUT node", 13))
    parts.append(_t(375, 345, "When FAN−≈24 V: Q1 ON → OUT low", 13))
    parts.append(_t(375, 368, "When FAN−≈0 V:  Q1 OFF → OUT high", 13))
    parts.append(_t(375, 400, "(= fan ON → laser ON; MCU-non-inv.)", 13, "#0a7a3e", True))

    # Right: 5V rail and output
    parts.append(_rect(740, 100, 360, 320, "#0867a5", "#f5faff", 3))
    parts.append(_t(755, 130, "5 V logic rail + output", 16, "#0867a5", True))
    parts.append(_t(755, 160, "Preferred Vcc: external buck 24 V→5 V", 13, bold=True))
    parts.append(_t(755, 182, "from PSU/FAN+ and DC− (common GND)", 13))
    parts.append(_t(755, 210, "Documented on-board 5 V also exists on", 12))
    parts.append(_t(755, 228, "BL_T / probe and Display EXP — both", 12))
    parts.append(_t(755, 246, "OCCUPIED; pin order NOT asserted here.", 12, "#b32727"))
    parts.append(_t(755, 280, "Rc 1.0 kΩ: Vcc(5 V) → OUT / Q1-C", 14, bold=True))
    parts.append(_t(755, 305, "Rpd 10 kΩ: OUT → GND (fail-safe LOW)", 14, bold=True))
    parts.append(_t(755, 330, "Rs 220 Ω series to laser TTL (optional)", 13))
    parts.append(_t(755, 355, "Optional: Cbp 100 nF on Vcc–GND", 13))
    parts.append(_t(755, 385, "3.3 V option: same parts, Vcc=3.3 V", 13, "#0a7a3e", True))
    parts.append(_t(755, 405, "(Rc may stay 1 kΩ; levels ~0 / 3.3 V)", 12))

    # Bottom truth table / notes
    parts.append(_rect(40, 450, 1060, 130, "#b32727", "#fff4f1", 2))
    parts.append(_t(55, 480, "Safety / polarity", 16, "#b32727", True))
    parts.append(_t(55, 505, "Laser DISCONNECTED for resistor-load bench. Heaters DISCONNECTED. Scope ground ONLY on board/PSU DC− — never on FAN−.", 13))
    parts.append(_t(55, 528, "Unpowered board, missing 5 V, or fan OFF → OUT held LOW by Rpd (and/or Q1 ON when FAN− floats high via Rload).", 13))
    parts.append(_t(55, 551, "Common ground: 24 V DC−, adapter GND, 5 V buck GND, laser module GND. No M3 / laser fire without Michael watching.", 13))

    parts.append(_t(40, 610, "Why Rload: SCOPE8 saw ~1 kHz duty ripple stuck near 24 V — consistent with probing an always-hot node and/or an unloaded/inductive FAN− that never pulled cleanly.", 12))
    parts.append(_t(40, 630, "Rload from FAN+ to FAN− forces a defined pull-UP on the drain so OFF≈24 V and ON≈0 V at FAN− vs DC− (tests MOSFET path independent of fan electronics).", 12))
    parts.append(_t(40, 655, "Generated by generate_diagrams.py — values match README.md / BOM.csv", 11, "#666"))
    return _svg_wrap("".join(parts), 1140, 680)


def build_perfboard_svg() -> str:
    parts = []
    parts.append(_t(40, 40, "Perfboard wiring (suggested stripboard / proto layout)", 24, bold=True))
    parts.append(_t(40, 68, "Top view · not to scale · verify every net with a meter before applying 24 V", 14))

    # Grid hint
    parts.append(_rect(40, 100, 700, 420, "#888", "#fafafa", 2, 4))
    for i in range(15):
        y = 120 + i * 26
        parts.append(_line(60, y, 720, y, "#ddd", 1))
    for i in range(26):
        x = 60 + i * 26
        parts.append(_line(x, 120, x, 500, "#eee", 1))

    # Blocks
    def box(x, y, w, h, title, lines, color):
        parts.append(_rect(x, y, w, h, color, "#fff", 2, 8))
        parts.append(_t(x + 10, y + 22, title, 14, color, True))
        yy = y + 44
        for ln in lines:
            parts.append(_t(x + 10, yy, ln, 12))
            yy += 18

    box(70, 130, 200, 150, "IN from K-FAN", [
        "FAN+  → Rload left",
        "FAN−  → Rload right",
        "FAN−  → R1 (47k)",
        "GND   → common rail",
    ], "#ac5c00")
    box(300, 130, 200, 150, "Divider / Q1", [
        "R1–R2 junction → Rb",
        "Rb → Q1 base",
        "Q1 E → GND rail",
        "Q1 C → OUT node",
    ], "#35566b")
    box(530, 130, 180, 150, "5 V / OUT", [
        "Buck +5 → Vcc rail",
        "Rc: Vcc → OUT",
        "Rpd: OUT → GND",
        "OUT → Rs → laser TTL",
    ], "#0867a5")

    box(70, 320, 300, 140, "Rails (recommended)", [
        "Top copper strip: Vcc (+5 V)",
        "Bottom copper strip: GND",
        "Keep 24 V FAN+ on a short stub only",
        "Keep Rload away from signal resistors (heat)",
    ], "#16364a")
    box(400, 320, 310, 140, "Optional", [
        "Dfly across FAN+/FAN− if fan stays fitted",
        "100 nF Vcc–GND near Q1",
        "3.3 V: feed Vcc from 3.3 V instead of 5 V",
        "Second CE stage NOT required for polarity",
    ], "#0a7a3e")

    parts.append(_t(40, 550, "Wire colours (suggestion only): orange=FAN+, brown=FAN−, blue=GND, red=+5, green=OUT.", 13))
    parts.append(_t(40, 575, "K-FAN and BL_T/EXP contact order is NOT drawn — identify on the physical board.", 13, "#b32727"))
    return _svg_wrap("".join(parts), 780, 600)


def simulate_and_plot():
    """Piecewise model: Rload sets FAN-; divider+NPN inverter → OUT."""
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not available — skipping plots")
        return []

    plots = []
    V24, Vcc = 24.0, 5.0
    R1, R2, Rb, Rc, Rpd = 47e3, 10e3, 4.7e3, 1e3, 10e3
    # Thevenin looking into divider at base node before Vbe clamp
    Rth = R1 * R2 / (R1 + R2)
    Vth_factor = R2 / (R1 + R2)

    def out_from_fan_minus(vm):
        # Vth at divider
        vth = vm * Vth_factor
        # Into base with Vbe=0.7 when conducting
        if vth <= 0.65:
            # Q1 off
            # OUT pulled up by Rc|| with Rpd
            return Vcc * Rpd / (Rc + Rpd)
        # Saturated-ish: Ib = (vth-0.7)/(Rb + something); treat as ON → OUT low via Q1
        # Approximate Vce_sat ≈ 0.15 V
        return 0.15

    for freq, name in [(1000, "sim_1kHz.png"), (10000, "sim_10kHz.png")]:
        t = np.linspace(0, 3.0 / freq, 3000)
        # 50% duty fan command: high half = MOSFET ON = FAN- ≈ 0
        period = 1.0 / freq
        fan_on = ((t % period) < 0.5 * period)
        # Add simple RC on drain (Rload || Cstray). Cstray ~ 1 nF wiring; with 2.2k tau~2 µs
        # For plot clarity use exponential edges
        tau = 2.2e3 * 1e-9
        vm = np.zeros_like(t)
        # Generate square then filter
        square = np.where(fan_on, 0.0, V24)
        # simple IIR
        a = math.exp(-(t[1] - t[0]) / tau)
        vm[0] = square[0]
        for i in range(1, len(t)):
            vm[i] = a * vm[i - 1] + (1 - a) * square[i]
        vout = np.array([out_from_fan_minus(v) for v in vm])

        fig, ax = plt.subplots(2, 1, figsize=(9, 5), sharex=True)
        ax[0].plot(t * 1e3, vm, color="#ac5c00", label="FAN− (drain)")
        ax[0].set_ylabel("Volts")
        ax[0].set_title(f"Adapter model @ {freq/1000:g} kHz, 50% (Rload edges τ≈{tau*1e6:.1f} µs)")
        ax[0].grid(True, alpha=0.3)
        ax[0].legend(loc="upper right")
        ax[0].set_ylim(-1, 26)
        ax[1].plot(t * 1e3, vout, color="#0867a5", label="OUT (5 V logic)")
        ax[1].set_xlabel("time (ms)")
        ax[1].set_ylabel("Volts")
        ax[1].set_ylim(-0.5, 5.5)
        ax[1].grid(True, alpha=0.3)
        ax[1].legend(loc="upper right")
        fig.tight_layout()
        path = ROOT / name
        fig.savefig(path, dpi=140)
        plt.close(fig)
        plots.append(path)
    return plots


def svg_to_png_pdf(svg_path: Path, png_path: Path, pdf_path: Path | None = None):
    import cairosvg
    from PIL import Image

    cairosvg.svg2png(url=str(svg_path), write_to=str(png_path))
    if pdf_path is not None:
        try:
            from reportlab.pdfgen import canvas
            from reportlab.lib.pagesizes import A4, landscape

            im = Image.open(png_path)
            c = canvas.Canvas(str(pdf_path), pagesize=landscape(A4))
            W, H = landscape(A4)
            iw, ih = im.size
            scale = min((W - 24) / iw, (H - 24) / ih)
            c.drawImage(str(png_path), 12, 12, width=iw * scale, height=ih * scale)
            c.save()
        except Exception as e:
            print("PDF skip:", e)


def main():
    sch = ROOT / "schematic.svg"
    perf = ROOT / "perfboard.svg"
    sch.write_text(build_schematic_svg())
    perf.write_text(build_perfboard_svg())
    svg_to_png_pdf(sch, ROOT / "schematic.png", ROOT / "schematic.pdf")
    svg_to_png_pdf(perf, ROOT / "perfboard.png", None)
    plots = simulate_and_plot()
    print("wrote", sch, perf, "plots", [p.name for p in plots])


if __name__ == "__main__":
    main()
