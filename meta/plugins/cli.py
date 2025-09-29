from pathlib import Path

from cutekit import cli, model, shell

from .image import Image


def generateImg(img: Image, target: str) -> Path:
    modules = []
    img.mkdir("/efi/boot")
    img.mkdir("/bin")
    img.cp(img.build("core", f"kernel-{target}"), Path("/kernel.elf"))

    if target == "x86_64":
        img.wget(
            "https://codeberg.org/Limine/Limine/raw/branch/v10.x-binary/BOOTX64.EFI",
            "efi/boot/bootx64.efi",
        )
        img.export_limine(Path("/efi/boot"), Path("/kernel.elf"), modules)
    return img.path


@cli.command("image", "Generate the boot image")
def _(): ...


class StartArgs(model.RegistryArgs):
    debug: bool = cli.arg(None, "debug", "Build the image in debug mode")
    arch: str = cli.arg(None, "arch", "The architecture of the image", default="x86_64")


@cli.command("image/start", "Run the system inside the emulator")
def _(args: StartArgs):
    if args.arch not in ["x86_64", "riscv32"]:
        raise RuntimeError(f"Unsupported architecture: {args.arch}")

    p = generateImg(Image(model.Registry.use(args), f"kernel-{args.arch}"), args.arch)

    match args.arch:
        case "x86_64":
            shell.exec(
                *[
                    "qemu-system-x86_64",
                    # "-enable-kvm",
                    # "-d", "int,guest_errors,cpu_reset",
                    "-no-reboot",
                    "-no-shutdown",
                    "-display",
                    "none",
                    "-m",
                    "2G",
                    "-smp",
                    "4",
                    "-debugcon",
                    "mon:stdio",
                    "-drive",
                    f"format=raw,file=fat:rw:{p},media=disk",
                    "-bios",
                    shell.wget("https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd"),
                ]
            )
        case "riscv32":
            shell.exec(
                *[
                    "qemu-system-riscv32",
                    "-machine", "virt",
                    "-bios", "default",
                    "-nographic",
                    "-serial", "mon:stdio",
                    "-kernel", str(p / "kernel.elf"),
                    "--no-reboot",
                ]
            )
