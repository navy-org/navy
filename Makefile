MKCWD = @mkdir -p $(@D)
LIMINE_GEN = ./meta/scripts/limine-gen.sh

SYSROOT = ./.image/
CACHE = ./.cache/

LOADER := $(SYSROOT)/efi/boot/bootx64.efi
BIN_FOLDER := $(SYSROOT)/bin
KERNEL := $(SYSROOT)/kernel.elf
FIRMWARE := $(CACHE)/OVMF.fd
ISO_FILE := $(CACHE)/navy.iso

LIMINE := $(CACHE)/limine
LIMINE_VER = v8.x-binary

SRC_KRNL = $(shell find ./src/kernel/ -name '*.zig')
SRC_APP = $(shell find ./src/apps/ -name '*.zig')

$(LOADER):
	@$(MKCWD)
	@curl -L https://github.com/limine-bootloader/limine/raw/refs/heads/$(LIMINE_VER)/BOOTX64.EFI -o $@

$(FIRMWARE):
	@$(MKCWD)
	@curl -L https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-x86_64.fd -o $@

$(CACHE)/limine.c:
	@$(MKCWD)
	@curl -L https://raw.githubusercontent.com/limine-bootloader/limine/refs/heads/$(LIMINE_VER)/limine.c -o $@

$(CACHE)/limine.h:
	@$(MKCWD)
	@curl -L https://raw.githubusercontent.com/limine-bootloader/limine/refs/heads/$(LIMINE_VER)/limine.h -o $@

$(CACHE)/limine-bios-hdd.h:
	@$(MKCWD)
	@curl -L https://raw.githubusercontent.com/limine-bootloader/limine/refs/heads/$(LIMINE_VER)/limine-bios-hdd.h -o $@

$(LIMINE): $(CACHE)/limine.c $(CACHE)/limine.h $(CACHE)/limine-bios-hdd.h
	@$(MKCWD)
	cc -g -O2 -pipe -Wall -Wextra -std=c99 -o $@ $<

$(SYSROOT)/limine-bios-cd.bin:
	@$(MKCWD)
	@curl -L https://github.com/limine-bootloader/limine/raw/refs/heads/$(LIMINE_VER)/limine-bios-cd.bin -o $@

$(SYSROOT)/limine-uefi-cd.bin:
	@$(MKCWD)
	@curl -L https://github.com/limine-bootloader/limine/raw/refs/heads/$(LIMINE_VER)/limine-uefi-cd.bin -o $@

$(SYSROOT)/limine-bios.sys:
	@$(MKCWD)
	@curl -L https://github.com/limine-bootloader/limine/raw/refs/heads/$(LIMINE_VER)/limine-bios.sys -o $@

$(BIN_FOLDER): $(SRC_APP)
	@mkdir -p $(BIN_FOLDER)
	@mv ./zig-out/bin/* $(BIN_FOLDER)

$(KERNEL): $(SRC_KRNL)
	@$(MKCWD)
	@zig build
	@mv ./zig-out/bin/navy $@


$(ISO_FILE): build $(BIN_FOLDER) $(KERNEL) $(LIMINE) $(SYSROOT)/limine-bios-cd.bin $(SYSROOT)/limine-uefi-cd.bin $(SYSROOT)/limine-bios.sys
	@$(MKCWD)
	@bash $(LIMINE_GEN) $(SYSROOT)

	@xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(SYSROOT) -o $@

	$(LIMINE) bios-install $@

.PHONY: iso
iso: $(ISO_FILE)

.PHONY: build
build:
	@zig build

.PHONY: qemu
qemu: build $(BIN_FOLDER) $(LOADER) $(KERNEL) $(FIRMWARE)
	@bash $(LIMINE_GEN) $(SYSROOT)
	@qemu-system-x86_64 \
		--no-reboot \
		--no-shutdown \
		-smp 4 \
		-serial mon:stdio \
		-display none \
		-drive format=raw,file=fat:rw:$(SYSROOT) \
		-bios $(FIRMWARE)

.PHONY: qemu-int
qemu-int: build $(BIN_FOLDER) $(LOADER) $(KERNEL) $(FIRMWARE)
	@bash $(LIMINE_GEN) $(SYSROOT)
	@qemu-system-x86_64 \
		--no-reboot \
		--no-shutdown \
		-smp 4 \
		-serial mon:stdio \
		-d int \
		-display none \
		-drive format=raw,file=fat:rw:$(SYSROOT) \
		-bios $(FIRMWARE)

.PHONY: clean
clean:
	@rm -rf ./.zig-cache/ ./zig-out/

.PHONY: nuke
nuke: clean
	@rm -rf $(SYSROOT)

.PHONY: path
path: build
	@echo $(KERNEL)

.PHONY: addr2line
addr2line: build
	@bash ./meta/scripts/addr2line.sh $(KERNEL) $(A)

.PHONY: bochs
bochs: $(ISO_FILE)
	bochs -f ./.bochsrc -q
