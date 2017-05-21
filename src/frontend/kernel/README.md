# stackos

<p align="center">
    <img src="https://i.imgur.com/WnTSfQi.png">
</p>

This is a simple kernel which allows the faststack engine to run atop it. It
is very similar to the terminal interface.

The kernel implements proper keyboard interrupt handling and timing and should
conserve as much power as it can.

## Installation

You will need the following programs:

 - `i686-elf-gcc`
 - `nasm`
 - `qemu-system-i386`  (optional: for testing)
 - `xorriso`, `mtools` (optional: for iso building)

```
# Build the kernel binary image
make

# Build the kernel binary image and run using i386 qemu
make run

# Build the iso from the binary image
make iso
```

This can be copied to a bootable usb as always using dd (**Double check your
drive partition numbers!**).

```
make iso
dd bs=4M if=stackos.iso of=/dev/sdX && sync
```
