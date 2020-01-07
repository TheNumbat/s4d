# s4d

![screenshot](https://i.imgur.com/EmkPbuC.png)

## Comments

The windows assimp debug static libraries are too big to push to github. They are in a zip file in deps/win/assimp. Extract it.

Currently mac/linux builds are broken, need to add assimp dependency.

## Build

```
meson build -Dbuildtype=debugoptimized
ninja -C build
```

### Windows Dependencies

- [Visual Studio Community 2019](https://visualstudio.microsoft.com/downloads/) with C++ features
- [Meson](https://mesonbuild.com/Getting-meson.html)
- [Ninja](https://github.com/ninja-build/ninja/releases)

Remember to unzip ``deps/win/assimp/libs.zip``.

### Linux/MacOS Dependencies

- gcc or clang
- SDL2
- gtk+3.0
- meson
- ninja

#### Apt
```
sudo apt install clang libsdl2-dev libgtk-3-dev python3-pip ninja-build
sudo pip3 install meson
```

#### Brew
```
brew install meson sdl2 gtk+3 pkgconfig
```
