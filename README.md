# MathChaosEngine

A lightweight mathematical visualization engine built with VS2022 + MFC.

MathChaosEngine explores the beauty of mathematics through fractals, chaotic systems, and generative visuals.

---

## Preview

![Gameplay Demo](./demo.gif)

![Gameplay Demo2](./demo2.gif)

![Gameplay screenshot](./screenshot.png)

![Gameplay screenshot2](./screenshot2.png)

![Gameplay screenshot3](./screenshot3.png)

![Mandelbrot Explorer](./mandelbrot.png)



![Mandelbrot Explorer](D:\Code\Class\MFC\MathChaosEngine\mandelbrot2.png)



---

## Current Features

- Fractal Tree module (animated growth, random style variants)
- Dynamic gradient background (breathing effect)
- Mandelbrot Explorer (progressive rendering + interactive navigation)
- Double-buffered rendering to reduce flicker
- Modular architecture for adding more math modules

---

## Mandelbrot Explorer (New)

### Performance

- Progressive render pipeline: `Preview -> Refining -> Final`
- Time-sliced computation per frame to keep UI responsive
- Tile-based refinement for better perceived smoothness
- Adaptive iteration count based on zoom level

### Interaction

- Mouse wheel: zoom around cursor
- Left drag: pan
- `Shift + Left drag`: box zoom
- Left click: quick zoom-in
- `Backspace`: go back in history
- Context menu: reset / zoom out / back / home

### Visual Styles

- Scientific
- Neon
- Monochrome

### Quality Presets

- Fast
- Balanced
- Detail

---

## Planned Modules

- Fractal Tree (implemented)
- Mandelbrot Set (implemented, explorer phase)
- Julia Set (next step: linked with Mandelbrot)
- Lorenz Attractor
- Particle Life Simulation
- Reaction-Diffusion

---

## Tech Highlights

- MFC `CDC`-based custom rendering pipeline
- DIB blit rendering for efficient full-frame updates
- Progressive fractal computation with stage management
- Palette-based color system with smooth coloring
- Extensible module interface (`Module`, `Engine`)

---

## Build and Run

1. Open `MathChaosEngine.sln` in Visual Studio 2022.
2. Select `Debug|x64` or `Release|x64`.
3. Build and run.

If build fails with `LNK1104` on `MathChaosEngine.exe`, close any running instance of the app and rebuild.

---

## Release

Download from: [Releases](../../releases)

---

## Author

**nci1496**  
GitHub: [https://github.com/nci1496](https://github.com/nci1496)

---

## About

This project is an exploration of mathematical art and interactive visualization with MFC.

For other MFC projects, visit: [MFC-Mini-Billiards](https://github.com/nci1496/MFC-Mini-Billiards)

---

## License

For educational and demonstration purposes.
