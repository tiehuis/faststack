CFLAGS := -ffreestanding -Wall -Wextra -O2
DEFINES := -DFS_DISABLE_OPTION -DFS_DISABLE_REPLAY -DFS_DISABLE_LOG -DFS_DISABLE_HISCORE

# TODO: Will have proper caching when moving all this into meson.
SRCS := $(wildcard ../../engine/*.c)
OBJS := $(addprefix build/,$(notdir $(SRCS:.c=.o)))

stackos.bin: linker.ld build/boot.o build/kernel.o $(OBJS)
	i686-elf-gcc -T linker.ld -o stackos.bin $(CFLAGS) -nostdlib \
		build/boot.o build/kernel.o $(OBJS) -lgcc

build/kernel.o: kernel.c | mkdirs
	i686-elf-gcc -c kernel.c -o build/kernel.o -std=gnu99 $(CFLAGS)

build/boot.o: boot.s | mkdirs
	i686-elf-as boot.s -o build/boot.o

run: stackos.bin
	qemu-system-i386 -kernel stackos.bin

build/%.o: ../../engine/%.c | mkdirs
	i686-elf-gcc $(DEFINES) -nostdlib $(CFLAGS) -c -o $@ $<

mkdirs:
	@mkdir -p build

clean:
	@rm -rf build

.PHONY: clean
