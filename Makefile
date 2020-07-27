.PHONY: all clean qemu

all: boot

boot: obj/boot.o obj/start.o src/boot.ld
	arm-linux-gnueabi-ld -static -T src/boot.ld -o boot obj/start.o obj/boot.o

obj/start.o: src/start.S
	arm-linux-gnueabi-gcc -static -c -O2 -nostdlib -fno-builtin -fno-stack-protector -fno-PIC -march=armv7-a -mfloat-abi=soft -std=c99 -Wall -o obj/start.o src/start.S

obj/boot.o: src/boot.c
	arm-linux-gnueabi-gcc -static -c -O2 -nostdlib -fno-builtin -fno-stack-protector -fno-PIC -march=armv7-a -mfloat-abi=soft -std=c99 -Wall -o obj/boot.o src/boot.c

clean:
	rm -f boot
	rm -f obj/*

qemu:
	qemu-system-arm -cpu cortex-a7 -M raspi2 -m 512 -serial mon:stdio -nographic -kernel boot
