# Geometrise

Geometrise is a C++ program that recreates images using geometric shapes via a genetic algorithm. Given a target image, the algorithm iteratively generates and refines shapes, selecting the best candidates each generation until the canvas closely approximates the original.

![Demo image placeholder](images/demo.png)

---

## How It Works

Geometrise uses a genetic algorithm to approximate a target image:

1. **Initialisation** — A set of random shapes is generated, each with a random position, size, colour, and rotation.
2. **Scoring** — Each shape is projected onto the canvas and scored using the Euclidean colour distance between the canvas and the target image. Lower scores are better.
3. **Selection** — The best scoring shape from the generation is kept.
4. **Mutation** — The best shape is used as a base for the next generation, with small random variations applied to its properties (scale, colour, opacity, position, rotation).
5. **Repeat** — This process repeats for a set number of generations, progressively refining the output.

![Algorithm diagram placeholder](images/algorithm.png)

---

## Dependencies

- [OpenCV](https://opencv.org/releases/) — used for image loading and pixel manipulation
- [GLFW](https://www.glfw.org/download.html) — used for window creation and OpenGL context *(planned)*
- [GLAD](https://glad.dav1d.de/) — used for loading OpenGL functions *(planned)*
- g++ with C++17 or later

---

## Installation

### 1. Clone the repository
```bash
git clone https://github.com/yourusername/geometrise.git
cd geometrise
```

### 2. Install OpenCV

**Linux (Ubuntu/Debian)**
```bash
sudo apt install libopencv-dev
```

**Mac**
```bash
brew install opencv
```

**Windows**

Download the installer from [opencv.org](https://opencv.org/releases/) and extract it to a directory like `C:\opencv`. Then add the following to your system PATH:
```
C:\opencv\build\x64\vc16\bin
```

### 3. Update the Makefile (Windows only)

Open the `Makefile` and set `OPENCV_DIR` to your OpenCV installation path:
```makefile
OPENCV_DIR = C:/opencv/build
```

---

## Building

```bash
make
```

This will compile all source files and produce the `geometrise` executable.

---

## Usage

```bash
./geometrise <target_image>
```

You can configure the following parameters in `include/params.h`:

| Parameter | Description | Default |
|---|---|---|
| `NUM_SHAPES` | Number of shapes per generation | 50 |
| `NUM_GENERATIONS` | Number of generations to run | 5 |
| `IMG_WIDTH` | Canvas width in pixels | 500 |
| `IMG_HEIGHT` | Canvas height in pixels | 500 |

---

## Example Output

![Before and after placeholder](images/example_output.png)

*Left: target image. Right: geometrise output after n generations.*

---

## License

MIT