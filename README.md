# DollsWatch

## How to build

### Install ESP-IDF
* Set $PATH to the toolchain dir.
* Set $IDF_PATH to the esp-idf dir.
  * Checkout the stable version tag. See `IDF_VER_REQUIRED` in Makefile.
  * `git submodule update --init --recursive`

### Fetch submodules
* `git submodule update --init --recursive`

### Config
* `cp sdkconfig.def sdkconfig`
* `make menuconfig`

### Make
* `make -j16`

### Write to flash
* `make flash ESPPORT=/dev/ttyS??`
