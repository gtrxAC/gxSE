# gxSE
Sprite Editor made with raylib (in early development)

![](assets/screenshot.png)


## Usage
* Currently there are three tools, 0 (brush), 1 (eraser), and 2 (fill bucket). The current tool can be changed by pressing left/right.
* At any point you can type commands into the text field on top. Press ENTER to execute the command and ESC to clear it.
* Left click to paint with color A, right click for color B.
* Middle click to pan.
* Scroll wheel to zoom.


## Commands
Each command consists of the command name and sometimes arguments, all separated by a space. Here is a command reference:

### `a` (1-4 arguments)
Sets the left click paint color (color A). The behavior depends on the amount of arguments:
- 1 argument: hex color value with alpha, for example `0xFF8000FF`.
- 3 arguments: RGB color values, for example `255 128 0`. Alpha is set to 255.
- 4 arguments: RGBA color values, for example `255 128 0 255`.

### `b` (1-4 arguments)
Same as `a`, but for the right click paint color (color B).

### `n` (1-2 arguments)
Creates a new image with the specified width and height. If only one argument is given, the width and height are the same value.

### `s` (1 argument)
Saves the file with the specified file name.

### `qn`
Quits without saving.

### `qs` (1 argument)
Saves the file just like the `s` command, and quits.


## Building
1. If you're on Windows, download [w64devkit](https://github.com/skeeto/w64devkit/releases). Make sure you get a release zip, not the source code. Extract the archive somewhere and run `w64devkit.exe`. On Linux, just open a terminal.
2. Follow the below instructions for the platform you want to build for.

### Desktop
1. Run `./setup.sh` to set up the project.
2. Run `./build.sh` to compile the project.

### Web
1. Run `TARGET=Web ./setup.sh` to set up the project. You will need about 1 GB of free space.
2. Run `TARGET=Web ./build.sh` to compile the project.

### Compiling for Windows from Linux
1. Install `mingw-w64` using your package manager.
2. Run `TARGET=Windows_NT ./setup.sh` to set up the project.
3. Run `TARGET=Windows_NT ./build.sh` to compile the project.