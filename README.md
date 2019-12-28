# retrofw_helloworld
A simple program which runs on Linux and RetroFW.


## Building

Prerequisites:
  - Build tools (Ubuntu,Debian: `sudo apt install build-essential`)
  - Native sdl1.2 development libraries (Ubuntu,Debian: `sudo apt install libsdl1.2-dev`)
  - RetroFW buildroot
  - curl

Build for RetroFW: `make -f Makefile.retrofw`
Build for Linux: `make -f Makefile.linux`


## Development

Override build options like the buildroot location in `Makefile.retrofw.config`. For example:
```
_BUILDROOT = /home/user/buildroot-rfw2

SYSROOT = $(_BUILDROOT)/output/host/mipsel-buildroot-linux-uclibc/sysroot
CROSS_COMPILE = $(_BUILDROOT)/output/host/usr/bin/mipsel-linux-
```

### Uploading code

  1. Plug in the RetroFW device and select "Charger" mode on the alert that appears.
    - The RetroFW device will act like a USB network card and host telnet and FTP services.
  2. Build and upload to RetroFW device: `make -f Makefile.retrofw upload-bin`
    - This will create the directory `devel/` in the device's internal SD card and upload the program there.

