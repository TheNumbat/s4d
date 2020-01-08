# s4d

![screenshot](https://i.imgur.com/EmkPbuC.png)

## Comments

The windows assimp debug static libraries are too big to push to github. They are in ``deps/win/assimp/libs.zip``. Extract it.

## Build

```
meson build -Dbuildtype=debugoptimized
ninja -C build
```

### Windows Dependencies

Tools
- [Visual Studio Community 2019](https://visualstudio.microsoft.com/downloads/) with C++ features
- [Meson](https://mesonbuild.com/Getting-meson.html)
- [Ninja](https://github.com/ninja-build/ninja/releases)

Remember to unzip ``deps/win/assimp/libs.zip``.

### Linux/MacOS Dependencies

Tools
- gcc or clang
- meson
- ninja

Libraries
- SDL2
- gtk+3.0
- assimp

#### Apt
```
sudo apt install libsdl2-dev libgtk-3-dev python3-pip ninja-build libassimp-dev
sudo pip3 install meson
```

#### Brew
```
brew install meson sdl2 gtk+3 pkgconfig assimp
```
