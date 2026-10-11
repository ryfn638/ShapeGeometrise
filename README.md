# Geometrise

Geometrise rebuilds an image out of shapes. It throws a bunch of random shapes at the canvas, keeps whichever one gets it closest to the target image, mutates that one a bit to see if it can do better, then draws it and moves on to the next. Do that a few hundred times and you get a pretty decent approximation of the picture.

This is the second time I've done this project. The first one was in Python and was objectively worse than this one. This version has custom shapes, better optimisation, better accuracy, better everything honestly. It also grades shapes on the GPU now with CUDA, which made it a lot faster.

Custom shapes still aren't hooked up in the GUI, but if you want to try them you can add your own mask in `igWindowUI.cpp` (look for `AddMask`).

<img width="271" height="182" alt="image" src="https://github.com/user-attachments/assets/8ec093bc-feb5-4e90-a627-055813eb8e5f" />

## How it works

For every shape that gets placed:

1. Make a generation of random shapes, each coloured with the average colour of the target underneath it
2. Score each one by how much closer it would get the canvas to the target (colour distance is done in LAB, so it's closer to how your eyes see it)
3. Take the best one and mutate it (scale, move, rotate, opacity) to make the next generation
4. After the last generation, draw the winner onto the canvas

It also does coarse to fine. The first 75% of shapes are found at quarter resolution because the big shapes don't need detail, then it steps up through half and three quarters to full resolution for the last 10%.

![2026-03-20 19-22-44](https://github.com/user-attachments/assets/fb0f242b-5d47-45ac-bc6c-995a146e83b7)

## CPU vs GPU

Scoring shapes was about 90% of the run time, so that's what got moved to CUDA. Each generation gets graded in one kernel launch, with one row of blocks per candidate shape, and each thread handles some of that shape's pixels. The average colour for each candidate is worked out on the GPU too, right before grading.

The CPU version is still there and you can switch between them with the "Use GPU (CUDA)" checkbox. On my 5070, colouring and grading 100 candidates at 780x520 took around 950ms on the CPU and under 1ms on the GPU. The scores aren't bit for bit identical (the GPU rounds `powf` slightly differently), but they're within a tiny fraction of a percent and pick the same shapes.

If you tick "Diagnostics" you get a window with graphs of how long each shape took, split into grading, creating candidates and everything else, plus which one it ran on. On the GPU the colour averaging counts as grading time, since it happens in the same step.

## UI

<img width="1255" height="771" alt="image" align="center" src="https://github.com/user-attachments/assets/8f7a204b-9d9e-4cd2-a1fb-37ba6f2fa456" />

Pick an image, set the sliders, hit Generate. You can stop it partway through if you've seen enough.

| Slider | What it does | Default |
|--------|--------------|---------|
| Shapes | How many shapes get drawn in total | 300 |
| Gen Shapes | Shapes tried per generation | 50 |
| Generations | Generations per shape | 2 |

## Building

You'll need:

- Windows and Visual Studio 2026 (2022 works too, see below)
- [premake5](https://premake.github.io/download) on your PATH
- The [CUDA toolkit](https://developer.nvidia.com/cuda-downloads) and an NVIDIA GPU. It won't build without the toolkit at the moment. Without a GPU it'll still run, it just grades on the CPU.

Then run:

```bash
createproject.bat
```

That makes the solution in `build/`. The first time, it also downloads OpenCV into `packages/` (around 200MB), so give it a minute. Use `createproject.bat open` to open the solution straight after, or `createproject.bat vs2022` for Visual Studio 2022. If you add or remove source files, run it again.

If you already have OpenCV installed you can point `OPENCV_DIR` at it and it'll skip the download.

The build setup is two parts. `premake5.lua` and `createproject.bat` are generic and I copy them between repos. The Geometrise specific stuff lives in `workspace.lua` (packages like OpenCV, DirectX and CUDA) and `ImageGeometrise/project.lua`.

## Project structure

```
ImageGeometrise/
    include/, src/
        igGeneration       runs the whole thing on its own thread: makes candidates, picks the best, draws it
        igCanvasGrader     scores a shape against the canvas (CPU), and works out its average colour
        igCanvas           the canvas/target image, projects shapes onto itself
        igShape            a mask plus position, size, rotation and colour
        igMask             the shape bitmap, loaded from an image in shapes/
        igCommon           colours (including the LAB conversion) and vectors
        CUDA/
            igGenerationBatch    GPU version of the grader, plus the kernels
        UI/
            igWindow             Win32 window
            igCanvasRenderer     DirectX 11, owns the canvas you see on screen
            igWindowUI           the ImGui panels
    3rdParty/imgui/        Dear ImGui
shapes/                    shape masks
targets/                   some test images
```

## Dependencies

- [OpenCV](https://opencv.org/) for loading and resizing images (downloaded for you)
- [Dear ImGui](https://github.com/ocornut/imgui) for the UI (in `3rdParty/imgui`)
- DirectX 11 for drawing (comes with the Windows SDK)
- CUDA for GPU grading
- OpenMP (comes with MSVC)

## License

MIT, see `LICENSE`.

## Acknowledgements

Inspired by [primitive](https://github.com/fogleman/primitive) by Michael Fogleman.
