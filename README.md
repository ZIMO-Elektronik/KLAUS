# KLAUS (KLUG Lite Audioload and Update Software)

A small cross-platform UI application to use in conjunction with the ZIMO MXULF and KLUG.

## About

This tool was developed to allow users of Linux desktop to update their decoders without the need of a VM. In theory, this tool should also compile for
MacOS, but no such toolchain was implemented to date (partially because of limited test hardware...).

## Prerequisites

In order to build and pack this tool, a vew tools are needed

### Build

* **[cmake](https://cmake.org/download/)** (3.25 or newer)
* A C++ compiler that supports C++ 23

> [!NOTE]
> I think some slint dependency must also be installed, something like libopengl-dev or simmilar?

### Pack

The tool can be packaged into an installable pack to allow for easier deployment. The prerequisites depend on the target platform. Since the tool is as of now intended to be build on linux, this list is only for linux users.

* NSIS

All of the above can be installed with

```sh
sudo apt install nsis
```

## Usage

To only build the tool it is enough to execute the target presets

```sh
### For linux
cmake --preset release_amd64
cmake --build --preset release_amd64

### Or for windows
cmake --preset release_amd64_windows
cmake --build --preset release_amd64_windows
```

If a package is required, this can either be done by executing the additional pack preset

```sh
### For linux
cpack --preset release_amd64

### Or for windows
cpack --preset release_amd64_windows
```

Or as an AIO workflow

```sh
### Linux (.deb and .tar.gz)
cmake --workflow --preset build-and-pack-linux

### Windows (.exe and .zip)
cmake --workflow --preset build-and-pack-windows

```

## Translation

In theory, this could be done using GNU-GetText, since this is natively supported by slint. But since it is a (humongous) hassle to
cross-compile with this tool, we will probably need a different approach.
