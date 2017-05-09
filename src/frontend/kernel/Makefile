stackos.bin: boot.o kernel.o linker.ld
	i686-elf-gcc -T linker.ld -o stackos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

kernel.o: kernel.c
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

boot.o: boot.s
	i686-elf-as boot.s -o boot.o

run: stackos.bin
	qemu-system-i386 -kernel stackos.bin

clean:
	rm -f boot.o kernel.o

.PHONY: clean
