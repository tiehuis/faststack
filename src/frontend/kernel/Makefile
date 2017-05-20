CFLAGS := -ffreestanding -Wall -Wextra -O2 -I. -I../../engine
DEFINES := -DFS_DISABLE_OPTION -DFS_DISABLE_REPLAY -DFS_DISABLE_LOG -DFS_DISABLE_HISCORE

# TODO: Will have proper caching when moving all this into meson.
ENGINE_SRCS := $(wildcard ../../engine/*.c)
ENGINE_OBJS := $(addprefix build/,$(notdir $(ENGINE_SRCS:.c=.o)))

KERNEL_SRCS := $(wildcard kernel/*.c)
KERNEL_OBJS := $(addprefix build/,$(notdir $(KERNEL_SRCS:.c=.o)))

KERNEL_ASMS := $(wildcard kernel/*.s)
KERNEL_OBJS := $(KERNEL_OBJS) $(addprefix build/,$(notdir $(KERNEL_ASMS:.s=.o)))

FRONTEND_SRCS := $(wildcard *.c)
FRONTEND_OBJS := $(addprefix build/frontend/,$(notdir $(FRONTEND_SRCS:.c=.o)))

all: stackos.bin

iso: stackos.iso

run: stackos.bin
	qemu-system-i386 -serial file:stackos.log -kernel stackos.bin

stackos.bin: linker.ld $(KERNEL_OBJS) $(ENGINE_OBJS) $(FRONTEND_SRCS)
	i686-elf-gcc -T linker.ld -o stackos.bin $(CFLAGS) $(DEFINES) -nostdlib \
		$(KERNEL_OBJS) $(ENGINE_OBJS) $(FRONTEND_SRCS) -lgcc

stackos.iso: stackos.bin grub.cfg | mkdirs_iso
	cp stackos.bin build/iso/boot/stackos.bin
	cp grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o stackos.iso build/iso

build/%.o: kernel/%.c | mkdirs
	i686-elf-gcc -std=gnu99 $(CFLAGS) -c $< -o $@

build/%.o: kernel/%.s | mkdirs
	nasm -felf32 $< -o $@

build/frontend/%.o: %.c | mkdirs
	i686-elf-gcc -std=gnu99 $(CFLAGS) -c $< -o $@

build/%.o: ../../engine/%.c ini.h | mkdirs
	i686-elf-gcc -imacros ini.h $(DEFINES) -nostdlib $(CFLAGS) -c -o $@ $<

mkdirs:
	@mkdir -p build

mkdirs_iso:
	@mkdir -p build/iso build/iso/boot build/iso/boot/grub

clean:
	@rm -rf build stackos.log stackos.bin stackos.iso

.PHONY: clean
