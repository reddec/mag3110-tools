# mag3110-tools

API libraries and CLI Tools for MAG3110 sensor.

[Datasheet](https://www.nxp.com/files/sensors/doc/data_sheet/MAG3110.pdf)

## Obtain

[You may use precompilled versions](https://github.com/reddec/mag3110-tools/releases) 
or build from source

### Build requirements

1. CMake 2.8
2. Make
3. C11 (gnu)  compiler
4. I2C-supported linux kernel (all that I know)
5. git (optional) for cloning

### Build

This is generic CMake process.

First, create new empty directory:

    mkdir -p /tmp/build-mag3110
    cd /tmp

Then clone Git repo

    git clone https://github.com/reddec/mag3110-tools.git

Use CMake for prepare MAKE files:

    cd /tmp/build-mag3110
    cmake -DCMAKE_BUILD_TYPE=Release /tmp/mag3110-tools

Compile and package:

    make package


### Install

After build you can use one of following method:

1. Use `mag3110-1.0.0-Linux.sh` as self-installer
2. Use `mag3110-1.0.0-Linux.tar.gz` as binary tar-ball
3. Use built files directly =)

### Linking

You may use static or shared library.
