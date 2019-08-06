# DollsWatch

## How to use
* Left, Center, Right button: app-specific features
* Long-press Center + Left or Right: Move app

### Watch
Initially 1970/01/01 00:00:00 after reset.
Will be automatically set by NTP when wifi is connected to an access point.

### Wifi
Can work as an access point or as a station.

For connecting to an access point as a station, it is needed to write
SSID/PASS in `wifi.txt` on SD card.
Obtained IP address will be displayed on the screen.

If you become an access point, your IP address will be `192.168.4.1`.

### Web server
Access `http://192.168.obtained.ip/recovery` from your browser.
Once you put main pages on SD card, `http://192.168.obtained.ip/`
will be available.

## Files setup
* Prepair micro SD card. (<=16GiB)
* Write files under `sdcard/` to the SD card.
  * If you don't have SD writer, you can use recovery web page
    with starting wifi access point feature!


## How to build

### Install ESP-IDF
See ESP-IDF Programming Guide "Get Started" page.
* https://docs.espressif.com/projects/esp-idf/en/v3.2.2/
* https://docs.espressif.com/projects/esp-idf/en/v3.2.2/get-started/index.html

----

* Download and extract toolchain.
  * Set $PATH to the toolchain dir.
* git clone the esp-idf.
  * Set `$IDF_PATH` to the esp-idf dir.
  * Checkout the stable version tag. See `IDF_VER_REQUIRED` in Makefile.
  * `git submodule update --init --recursive`
* Install dependencies.
  * flex, bison, python, etc. See the guide.
* Install python dependencies.
  * `python -m pip install --user -r $IDF_PATH/requirements.txt`

### Get this project
* git clone this project.
  * `git submodule update --init --recursive`

### Config (if needed)
* `make menuconfig`

### Make
* `make -j16`

### Install UART driver
Download "CP210X Driver"

https://m5stack.com/pages/download

### Write to flash
* Set `$ESPPORT` to "/dev/ttyS?"
* `make flash ESPPORT=/dev/ttyS??`
  * `make app-flash` may be faster.

### Show UART output
* `make monitor`
  * Quit: Ctrl+]
  * Command: Ctrl+T, Ctrl+?
    * Help: Ctrl+T, Ctrl+H
    * Build & flash app only (should be useful): Ctrl+T, Ctrl+A


## Troubleshooting

### Cannot write flash
Attach a capacitor (e.g. 2.2uF) to RST - GND pin.

### Build error in M5Stack build
Apply M5Stack.patch.
