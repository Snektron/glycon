# Glycon

## Dependencies

* meson
* ninja or samurai
* avr-binutils
* avr-gcc
* avr-libc

## Compiling

### Coprocessor

```
$ mkdir -p build/glyco
$ cd build/glyco
$ meson ../.. --cross-file ../../cross/avr-atmega2560-cross.ini
$ ninja
```

To use the provided `flash` target to flash the coprocessor, set the coprocessor port the arduino is connected with by passing `-Dport=/dev/<port>` to meson (usually /dev/ttyUSB0) and add yourself to to appropriate groups (typically `dialout` or `uucp`).
