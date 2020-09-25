CC = arm-linux-gnueabi-gcc
CFLAGS = -static -O2 -nostdlib -fno-builtin -fno-stack-protector -fno-PIC -march=armv7-a -mfloat-abi=soft -std=c99 -Wall

SYS_OBJS = $(addprefix build/sys/, boot.o start.o printf.o vm.o buddy.o math.o mem_test.o proc.o program.o)
INIT_OBJS = $(addprefix build/usr/, start.o)

.PHONY: all clean qemu

all: build/boot

build/boot: boot_dir $(SYS_OBJS) src/sys/boot.ld
	arm-linux-gnueabi-ld -static -T src/sys/boot.ld -o $@ $(SYS_OBJS)

boot_dir:
	mkdir -p build/sys

build/sys/start.o: src/sys/start.S
	$(CC) $(CFLAGS) -c -o $@ $<

build/sys/boot.o: src/sys/boot.c src/sys/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

build/sys/math.o: src/sys/math.c src/sys/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

build/sys/printf.o: src/sys/printf.c src/sys/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

build/sys/vm.o: src/sys/vm.c src/sys/*.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
build/sys/buddy.o: src/sys/buddy.c src/sys/*.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
build/sys/mem_test.o: src/sys/mem_test.c src/sys/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

build/sys/proc.o: src/sys/proc.c src/sys/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

build/sys/program.o: gen/program.c
	$(CC) $(CFLAGS) -c -o $@ $<

gen/program.c: build/bin2c build/init
	build/bin2c build/init gen/program.c

build/bin2c: src/tools/bin2c.c
	gcc -O2 -o build/bin2c src/tools/bin2c.c

build/init: init_dir $(INIT_OBJS) src/libc/user.ld
	arm-linux-gnueabi-ld -static -T src/libc/user.ld -o $@ $(INIT_OBJS)

init_dir:
	mkdir -p build/usr

build/usr/start.o: src/libc/start.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f boot init bin2c
	rm -f build/*

qemu:
	qemu-system-arm -cpu cortex-a7 -M raspi2 -m 512 -serial mon:stdio -nographic -kernel build/boot
