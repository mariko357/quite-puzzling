# Build instructions

1. Build the FW
```
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```
2. Connect pico with BOOTSEL button pressed, it should show up as mass storage.

3. Copy `build/puzzle-detector.uf2` to the pico.