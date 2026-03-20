# Geometrise

Geometrise is a geometric image approximation tool that reconstructs images using procedurally generated and evolved shapes. It uses a genetic algorithm to iteratively place shapes that best approximate a target image, starting with large coarse shapes and progressively refining with smaller detailed ones.

![Preview Image](images/preview.png)
*Add preview image here*

---

## How It Works

Geometrise uses an evolutionary approach to image reconstruction:

1. A generation of random shapes is created
2. Each shape is scored by how much it improves the canvas towards the target image
3. The best shape is kept and projected onto the canvas
4. Subsequent generations mutate the best shape to find further improvements
5. This repeats for a specified number of shapes, progressively refining the image

The algorithm uses a coarse-to-fine strategy — early shapes are found at a lower resolution for speed, with resolution increasing as more detail is needed.

![Algorithm Progression](images/progression.png)
*Add progression image here*

---

## Features

- Geometric image approximation using evolutionary algorithms
- Progressive resolution scaling for fast convergence
- Real time canvas preview via ImGui
- Support for custom shape masks
- Adjustable parameters via UI sliders
- Multi-threaded shape generation with OpenMP

---

## UI

![UI Screenshot](images/ui.png)
*Add UI screenshot here*

The UI provides:
- Image selection via file dialog
- Adjustable parameters (number of shapes, generation size, generations)
- Real time canvas preview
- Generation progress indicator

---

## Dependencies

- [OpenCV 4.x](https://opencv.org/) — image loading and processing
- [Dear ImGui](https://github.com/ocornut/imgui) — UI rendering (included in `imgui/`)
- DirectX 11 — rendering backend (included with Windows SDK)
- OpenMP — parallel shape generation (included with MSVC)

---

## Setup

### Prerequisites
- Visual Studio 2019 or later
- OpenCV 4.x installed
- Windows 10 or later

### Building

1. Clone the repository:
```bash
git clone https://github.com/yourusername/Geometrise.git
```

2. Open `Geometrise.sln` in Visual Studio

3. Set up OpenCV paths — create `opencv.props` from the example:
```
opencv.props.example → opencv.props
```
Update the paths in `opencv.props` to match your OpenCV installation:
```
Include: C:\opencv\build\include
Lib:     C:\opencv\build\x64\vc16\lib
```

4. Build and run in Debug or Release configuration

---

## Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `NUM_SHAPES` | Total number of shapes to place | 300 |
| `GENERATION_SHAPES` | Shapes evaluated per generation | 50 |
| `NUM_GENERATIONS` | Generations per shape | 2 |

---

## Project Structure
```
Geometrise/
    src/
        main.cpp          — entry point, ImGui render loop
        pipeline.cpp      — draw_shapes, find_best_shape
        generation.cpp    — createGeneration, createShape, scoring
        operations.cpp    — shape transforms, mask creation
        draw.cpp          — canvas visualisation
    include/
        arena.h           — Colour, shape_t, ShapePoint structs
        params.h          — global parameters
        pipeline.h        — pipeline function declarations
        generation.h      — generation function declarations
        operations.h      — operations function declarations
        draw.h            — Frame class, visualise_canvas
    imgui/                — Dear ImGui source files
    shapes/               — shape mask images
    targets/              — target images
```

---

## License

MIT License — see `LICENSE` for details.

---

## Acknowledgements

Inspired by [primitive](https://github.com/fogleman/primitive) by Michael Fogleman.