Delaunay Viewer
===============

## Features

...

## GUI

### Dependencies

* [Dear ImGui](https://github.com/ocornut/imgui)
* [GLFW3](http://glfw.sf.net)
* [Portable File Dialogs](https://github.com/samhocevar/portable-file-dialogs)
* [simple-svg]()
* [bx](https://github.com/bkaradzic/bx) (A depenency of simple-svg)

Delaunay triangulation libs:

* [poly2tri](https://github.com/jhasse/poly2tri)
* [CDT](https://github.com/artem-ogre/CDT)

### Build

With [CMake](https://cmake.org/download/). For example on Windows:

```
mkdir ./build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

### Install

Install in some dir:

```
cmake --install . --config Release --prefix <some_dir>
```

Or, package the library:

```
cpack -G ZIP -C Release
```

### Usage

## Contributions

This project does not accept pull requests at the moment.

Please [submit an issue](https://github.com/pierre-dejoue/delaunay-viewer/issues/new) for a feature request, a bug report or any question.

## License

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](./LICENSE)
