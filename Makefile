CC = arm-linux-gnueabi-gcc
CFLAGS = -static -c -O2 -nostdlib -fno-builtin -fno-stack-protector -fno-PIC -march=armv7-a -mfloat-abi=soft -std=c99 -Wall

OBJS = obj/boot.o obj/start.o obj/printf.o obj/vm.o

.PHONY: all clean qemu

all: boot

boot: $(OBJS) src/boot.ld
	arm-linux-gnueabi-ld -static -T src/boot.ld -o $@ $(OBJS)

obj/start.o: src/start.S
	$(CC) $(CFLAGS) -o $@ $<

obj/boot.o: src/boot.c src/*.h
	$(CC) $(CFLAGS) -o $@ $<

obj/printf.o: src/printf.c src/*.h
	$(CC) $(CFLAGS) -o $@ $<

obj/vm.o: src/vm.c src/*.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f boot
	rm -f obj/*

qemu:
	qemu-system-arm -cpu cortex-a7 -M raspi2 -m 512 -serial mon:stdio -nographic -kernel boot
