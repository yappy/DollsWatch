# DollsWatch
Multiple applications suite for M5Stack esp32 wrist watch.

## How to use
* Left, Center, Right button: app-specific features
* Long-press Center + Left or Right: Move app

### Watch
Initially 1970/01/01 00:00:00 after reset.
It will be automatically set by NTP when wifi is connected to an access point.

### Wifi
Can work as a station or as an access point.

To connect to an access point as a station, it is needed to write
SSID/PASS in `/conf/wifi.txt` on the SD card.
To modify file, see `Files setup` section below.
Obtained **IP address** will be displayed on the screen.

If you become an access point, your **IP address** will be `192.168.4.1`.

### Web server
Access `http://<ipaddr>/recovery` from your browser.
Once you put main page files on SD card, `http://<ipaddr>/` will be available.

### Files setup
* Prepair micro SD card. (<=16GiB)
* Write files under `sdcard/` to the SD card.
  * If you don't have SD writer, you can use recovery web page
    `http://<ipaddr>/recovery` with starting wifi access point feature!
  * Recovery page now supports REST API and python script is available.
    For example, the script can upload all files under a directory so quickly.

```
# python 3.5 required

# Show help
$ python3 tools/sd.py

# Show all files in SD card
$ python3 tools/sd.py list <ipaddr>

# Upload all files under "sdcard/" to "/sd/"
$ python3 tools/sd.py upload <ipaddr> sdcard/

# Delete all files (warning!)
$ python3 tools/sd.py initialize
```

----

## How to build
### Check the required ESP-IDF version
See `IDF_VER_REQUIRED` in Makefile or `IDF_VER` in .travis.yml.
It is represented as <IDF_VER> in the following sections.

### Install ESP-IDF
See ESP-IDF Programming Guide "Get Started" page.
* https://docs.espressif.com/projects/esp-idf/en/<IDF_VER>/
* https://docs.espressif.com/projects/esp-idf/en/<IDF_VER>/get-started/index.html

Steps:
* Download and extract toolchain.
  * Set `$PATH` to the toolchain dir.
* git clone the esp-idf.
  * `git clone https://github.com/espressif/esp-idf.git`
  * Set env `$IDF_PATH` to the esp-idf dir.
  * `git checkout <IDF_VER>`
  * `git submodule update --init --recursive`
* Install dependencies.
  * flex, bison, python, etc. See the guide.
* Install python dependencies.
  * `python -m pip install --user -r $IDF_PATH/requirements.txt`

### Get this project
* git clone this project and update submodules.
  * `git submodule update --init --recursive`

### Default config
* Delete old file if exists
  * `rm sdkconfig`
* Debug build
  * `make defconfig`
* Release build
  * `make SDKCONFIG_DEFAULTS=sdkconfig.release defconfig`

### Config (if needed)
* `make menuconfig`

### Make
* `make -j16`
  * Determine N value in `-jN` with your logical CPU core number.
* `make -j` (no limit)

### Install UART driver
Download "CP210X Driver"

https://m5stack.com/pages/download

### Write to flash
* Set `$ESPPORT` to "/dev/ttyS?"
* `make flash`
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

### Build error in AzureIoT release build
Apply AzureIoT.patch. (see .travis.yml)
