CC = clang
LD = ld.lld
AS = nasm
AR = ar

include config.mk

MKCWD=mkdir -p $(@D)
BUILD = ./build

CFLAGS =							\
	-ffreestanding					\
    -mno-80387						\
    -mno-mmx						\
    -mno-3dnow						\
    -mno-red-zone					\
    -mno-sse						\
    -mno-sse2						\
    -fno-stack-protector			\
    -fno-builtin					\
    -ggdb							\
    -O0								\
    -Wall							\
    -Wextra							\
    -Werror							\
    -std=gnu2x						\
	-Isrc							\
	-Isrc/libc						\
	-D__$(ARCH)__					\
	-D__$(LOADER)__					\
	-D__$(SCHED)__					\
	-D__freestanding__

ASFLAGS =							\
	-F dwarf						\
	-g

LDFLAGS = 							\
	-static							\
	-nostdlib						\

ARFLAGS = rcs

QEMU_FLAGS =												\
	-no-shutdown											\
	-no-reboot												\
	-serial mon:stdio										\
	-m 256M													\
	-smp 4													\
	-bios $(BIOS)											\
	-drive file=fat:rw:sysroot,media=disk,format=raw		\
	-enable-kvm												\

include src/$(ARCH)/.build.mk
include src/$(LOADER)/.build.mk
include src/$(SCHED)/.build.mk
include src/handover/.build.mk
include src/libc/.build.mk
include src/copland/.build.mk
include src/liballoc/.build.mk
include src/kernel/.build.mk
include src/hello-world/.build.mk

all: $(KERNEL) sysroot hello-world

clean:
	rm -r build/ sysroot/

sysroot: $(KERNEL) $(LOADER_FILE)
	mkdir -p $(SYSROOT)/boot $(SYSROOT)/bin
	cp $(KERNEL) $(SYSROOT)/boot

run: all $(SYSROOT)
	$(QEMU) $(QEMU_FLAGS)

.PHONY: all
.DEFAULT_GOAL := all