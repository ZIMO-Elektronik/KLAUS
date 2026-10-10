# KLAUS (KLUG Lite Audioload and Update Software)

A small cross-platform UI application to use in conjunction with the ZIMO MXULF and KLUG. Prebuilt binaries are available on the [releases](https://github.com/ZIMO-Elektronik/KLAUS/releases/latest) page. 

## About

This tool was developed to allow users of Linux desktop to update their decoders without the need of a VM. In theory, this tool should also compile for
MacOS, but no such toolchain was implemented to date (partially because of limited test hardware...).

Furthermore, KLAUS is a proof-of-concept work for [libULF](https://github.com/ZIMO-Elektronik/libULF). 

## Prerequisites

In order to build and package KLAUS, a vew tools are needed

### Build

* **[cmake](https://cmake.org/download/)** (3.25 or newer)
* **[vcpkg](https://vcpkg.io)**
* A C++ compiler that supports C++ 23
* Rust

### Pack

The tool can be packaged into an installable pack to allow for easier deployment. The prerequisites depend on the target platform. Since the tool is as of now intended to be build on linux, this list is only for linux users.

* For windows
  * [NSIS](https://github.com/NSIS-Dev/nsis)
* For linux
  * [appimagetool](https://github.com/AppImage/appimagetool)
  * [patchelf](https://github.com/nixos/patchelf)

## Usage

To only build the tool it is enough to execute the target presets

```sh
cmake --preset release && cmake --build --preset release
```

If a package is required, this can either be done by executing the additional pack preset

```sh
### Of course packages are platform dependent, so we need platform specific presets
## For linux (AppImage and Archive)
cpack --preset release_linux

## For windows (Installer and Archive)
cpack --preset release_windows
```

Or as an AIO workflow

```sh
### Linux (.AppImage and .tar.gz)
cmake --workflow --preset build-and-pack-linux

### Windows (.exe and .zip)
cmake --workflow --preset build-and-pack-windows
```

> !NOTE
> For some reason, on windows VCPKG defaults to MSVC even if the cmake install detects MinGW as compiler. This will lead to ABI issues. Resulting, for a toolchain not dependent on MSVC, the VCPKG triplet needs to be selected explicitly. An example would be the 'release_amd64_windows_ci' preset chain (for the same reason) 

## Translation

In theory, this could be done using GNU-GetText, since this is natively supported by slint. But since it is a (humongous) hassle to
cross-compile with this tool, we will probably need a different approach.
