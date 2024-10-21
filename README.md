[![Build Windows](https://github.com/P0ulpy/raycasting-engine/actions/workflows/cmake-build-windows.yml/badge.svg)](https://github.com/P0ulpy/raycasting-engine/actions/workflows/cmake-build-windows.yml)
[![Build MacOS](https://github.com/P0ulpy/raycasting-engine/actions/workflows/cmake-build-macos.yml/badge.svg)](https://github.com/P0ulpy/raycasting-engine/actions/workflows/cmake-build-macos.yml)
[![Build Ubuntu](https://github.com/P0ulpy/raycasting-engine/actions/workflows/cmake-build-ubuntu.yml/badge.svg)](https://github.com/P0ulpy/raycasting-engine/actions/workflows/cmake-build-ubuntu.yml)

# Raycasting Engine - A Modern C++ Implementation of a DOOM Style Renderer

Welcome to the Raycasting Engine project, an implementation of a DOOM Style renderer using modern C++ and technologies like raylib and imgui.

This project is developed in a **recreational programming** context, it does not aim to be *production ready* at some point. It serves as a learning experience to explore new concepts and technologies while creating a project around them.

## Goals
The primary goal of this project is to implement a DOOM Style Renderer with a simple Engine around it and a Graphical Editor for prototyping Maps, rendering, gameplay etc... I'm using modern C++ and technologies like raylib and imgui it achieve this goal. At the end the engine should be able to build and ship a standalone executable without the editor.

## Showcase

![Overall Showcase](.github/videos/overall_showcase.mp4)

`Editor 3D View`
![Editor 3D View](.github/images/editor_3d_view.png)

`Map Editor View`
![Map Editor View](.github/images/map_editor_view.png)

## Features checklist

For now the project in a prototyping phase, still mainly focused in making an efficient Renderer.

*Renderer*
- [x] DOOM Style rendering including :
  - [x] Sectors and top/bottom Elevation
  - [x] Neighbouring sectors sides
  - [ ] Textures for Wall & Borders
- [x] Pitch and Yaw for camera movement
- [ ] Ceiling and floor casting
- [ ] Vignette for sprites rendered over walls
- [ ] Entities sprites within the 3D space
- [ ] Basic Physics including : 
  - [x] Elevation related to sectors 
  - [ ] Simple gravity
  - [ ] 3D space Raycast
- [x] Simple shading using far plane distance
- [ ] Dynamic lighting for light sources

*Editor*
- [x] 3D View
- [x] Simple inspector for Global world objects (Physics, Camera, etc...)
- [x] Rendering Debugger including :
  - [x] Step by step rendering
  - [x] Rendering steps visalizer
 - [ ] Map Editor including :
  - [x] Controls (zoom, move, ...)
  - [ ] [IN_PROGRESS] Sector Editor
    - [x] Elevation editor
    - [x] Proper sectors rendering
    - [ ] Sector creation tool
    - [ ] Wall creation tool
    - [ ] Portal creation/visualisation tool
  - [ ] Vignet editor
  - [ ] Sprite editor
 - [ ] Texture Browser
*System*
- [ ] Project instance cration : being able to create a project witch is using the engine and editor in one click
- [ ] Simple assets serealization / deserialization system
  - [ ] standards containers (std::map, std::vector, std::string)
  - [ ] Map / Entities
  - [ ] Enable user to implement serialization for his own Asset types 
- [ ] Project shiping including :
  - [ ] Release Build
  - [ ] Assets bundle

*Debug / Enhancement*
- [ ] Fix Sector elevation computation

## Building the project from sources

This repo toolchain as been created using my [cmake and vcpkg template](https://github.com/P0ulpy/cmake-vcpkg-template).

### Bootstrap workspace

**Windows :**

Make sure you installed [Visual Studio](https://visualstudio.microsoft.com/) with [C and C++ support](https://learn.microsoft.com/cpp/build/vscpp-step-0-installation?view=msvc-170#step-4---choose-workloads) and [C++ linux development with cmake](https://learn.microsoft.com/cpp/linux/download-install-and-setup-the-linux-development-workload?view=msvc-170#visual-studio-setup).
Make sure you installed [CMake](https://cmake.org/download) as well.

**GNU/Linux (apt) :**

Install necessary build tools and a C/C++ compiler
```sh
sudo apt-get update
sudo apt-get install cmake build-essential tar curl zip unzip autoconf libtool g++ gcc
```

Then run the bootstrap script
```sh
# Unix
./scripts/bootstrap-workspace.sh
# Windows
.\scripts\bootstrap-workspace.bat
```

**Generate the cmake project :**

```sh
# For debug build
# Unix
./scripts/generate-cmake-debug.sh
# Windows
.\scripts\generate-cmake-debug.bat

# For release build
# Unix
./scripts/generate-cmake-release.sh
# Windows
.\scripts\generate-cmake-release.bat
```

**Compile :**

```bash
# For debug build
# Unix
./scripts/build-debug.sh
# Windows
.\scripts\build-debug.bat

# For release build
# Unix
./scripts/build-release.sh
# Windows
.\scripts\build-release.bat
```

**Run the program**

You can now run the compiled program by looking into `out/Debug` or `out/Release`.
The out directory hierarchy will be different depending on your generator.

For exemple with Make generator in Debug mode run your program like that
```bash
# Unix
./out/Debug/raycasting-engine
# Windows
.\out\Debug\raycasting-engine.exe
```

**Using Visual Studio Code workspace**

If you are using visual studio code you can directly use the project embeded workspace `.vscode/raycasting-engine.code-workspace`.
You can press `F5` to build and run with debugger gdb or vsdbg (visual studio debugger).

## Licence

This project is open-source and available under the [MIT Licence](LICENSE).
