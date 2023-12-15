from pathlib import Path
from cutekit import const, shell

from .build import Builder

class Image:
    def __init__(self, builder: Builder, dst: Path = Path(".") / const.PROJECT_CK_DIR / "navy"):
        self.builder = builder
        self.path = dst

        efi = getattr(self, f"efi_{builder.arch}", None)

        if efi:
            getattr(self, f"efi_{builder.arch}")()
        else:
            getattr(self, f"boot_{builder.arch}")()

    def __str__(self):
        return str(self.path)

    def __efi_common(self) -> dict[str, Path]:
        efi_boot = self.path / "EFI" / "BOOT"
        efi_boot.mkdir(parents=True, exist_ok=True)

        boot_dir = self.path / "boot"
        boot_dir.mkdir(parents=True, exist_ok=True)

        return {
            "efi_boot": efi_boot,
            "boot_dir": boot_dir,
        }


    def efi_x86_64(self):
        kernel_path = self.builder.build_core()
        paths = self.__efi_common()

        cfg = [
            "TIMEOUT=0",
            "GRAPHICS=no",
            ":navy",
            "PROTOCOL=limine",
            "KERNEL_PATH=boot:///boot/kernel.elf",
        ]

        shell.wget(
            "https://github.com/limine-bootloader/limine/raw/v6.x-branch-binary/BOOTX64.EFI",
            str(paths["efi_boot"] / "BOOTX64.EFI"),
        )

        shell.cp(
            str(kernel_path),
            str(paths["boot_dir"] / "kernel.elf"),
        )

        with (paths["boot_dir"] / "limine.cfg").open("w") as f:
            f.write('\n'.join(cfg))