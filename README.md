# Geometrise

Geometrise rebuilds an image out of shapes. It throws a bunch of random shapes at the canvas, keeps whichever one gets it closest to the target image, mutates that one a bit to see if it can do better, then draws it and moves on to the next. Do that a few hundred times and you get a pretty decent approximation of the picture.

This is the second time I've done this project. The first one was in Python and was objectively worse than this one. This version has custom shapes, better optimisation, better accuracy, better everything honestly. It also grades shapes on the GPU now with CUDA, which made it a lot faster.

Custom shapes still aren't hooked up in the GUI, but if you want to try them you can add your own mask in `igWindowUI.cpp` (look for `AddMask`).

<img width="271" height="182" alt="image" src="https://github.com/user-attachments/assets/8ec093bc-feb5-4e90-a627-055813eb8e5f" />

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
