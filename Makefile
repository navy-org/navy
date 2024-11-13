MKCWD = @mkdir -p $(@D)
LIMINE_GEN = ./meta/scripts/limine-gen.sh

SYSROOT = ./.image/
CACHE = ./.cache/

LOADER = $(SYSROOT)/efi/boot/bootx64.efi
KERNEL = $(SYSROOT)/kernel.elf
FIRMWARE = $(CACHE)/OVMF.fd

SRC = $(shell find . -name '*.zig') $(shell find . -name "*.s")

$(LOADER): $(SRC)
	@$(MKCWD)
	@zig build
	@cp ./zig-out/bin/bootx64.efi $@

$(KERNEL): $(SRC)
	@$(MKCWD)
	@zig build
	@cp ./zig-out/bin/kernel.elf $@

$(FIRMWARE):
	@$(MKCWD)
	@curl -L https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-x86_64.fd -o $@

.PHONY: build
build: $(KERNEL)

.PHONY: gdb
gdb: $(LOADER) $(KERNEL) $(FIRMWARE)
	qemu-system-x86_64 \
		--no-reboot \
		--no-shutdown \
		-s \
		-S \
		-smp 4 \
		-serial \
		mon:stdio \
		-display none \
		-drive format=raw,file=fat:rw:$(SYSROOT) \
		-bios $(FIRMWARE)

.PHONY: qemu
qemu: $(LOADER) $(KERNEL) $(FIRMWARE)
	@bash $(LIMINE_GEN) $(SYSROOT)
	qemu-system-x86_64 \
		--no-reboot \
		--no-shutdown \
		-smp 4 \
		-serial \
		mon:stdio \
		-display none \
		-drive format=raw,file=fat:rw:$(SYSROOT) \
		-bios $(FIRMWARE)

.PHONY: clean
clean:
	rm -rf ./zig-cache/ ./zig-out/

.PHONY: nuke
nuke: clean
	rm -rf $(SYSROOT)

.PHONY: path
path:
	@echo $(KERNEL)

.PHONY: addr2line
addr2line: $(KERNEL)
	@bash ./meta/scripts/addr2line.sh $(KERNEL) $(A)
