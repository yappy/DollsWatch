# DollsWatch

## How to build

### Install ESP-IDF
See ESP-IDF Programming Guide "Get Started" page.
* Set $PATH to the toolchain dir.
* git clone the esp-idf.
* Set $IDF_PATH to the esp-idf dir.
  * Checkout the stable version tag. See `IDF_VER_REQUIRED` in Makefile.
  * `git submodule update --init --recursive`
* Install dependencies.
  * flex, bison, python, etc. See the document.
* Install python dependencies.
  * `python -m pip install --user -r $IDF_PATH/requirements.txt`

### Get this project
* git clone this project.
  * `git submodule update --init --recursive`

### Config
* `make menuconfig`

### Make
* `make -j16`

### Write to flash
* `make flash ESPPORT=/dev/ttyS??`
  * `make app-flash ESPPORT=/dev/ttyS??` may be faster.
