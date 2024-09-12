from cutekit import cli, model, shell
from .image import Image
from pathlib import Path


def generateImg(img: Image, target: str) -> Path:
    img.mkdir("/EFI/BOOT")
    img.cp(img.build("kernel", f"kernel-{target}"), Path("/kernel.elf"))
    if target == "x86_64":
        img.wget(
            "https://github.com/limine-bootloader/limine/raw/v7.9.1-binary/BOOTX64.EFI",
            "EFI/BOOT/BOOTX64.EFI",
        )
        img.export_limine(Path("/EFI/BOOT"), Path("/kernel.elf"), [])
    return img.path


@cli.command("i", "image", "Generate the boot image")
def _(): ...


class StartArgs(model.RegistryArgs):
    debug: bool = cli.arg(None, "debug", "Build the image in debug mode")
    arch: str = cli.arg(None, "arch" "The architecture of the image", default="x86_64")


@cli.command("d", "image/debug", "Run the system inside the emulator in debug mode")
def _(args: StartArgs):
    if args.arch not in ["x86_64"]:
        raise RuntimeError(f"Unsupported architecture: {args.arch}")

    p = generateImg(
        Image(model.Registry.use(args), f"continuum-{args.arch}"), args.arch
    )
    shell.exec(
        *[
            "qemu-system-x86_64",
            "-no-reboot",
            "-no-shutdown",
            "-smp",
            "4",
            "-debugcon",
            "mon:stdio",
            "-drive",
            f"format=raw,file=fat:rw:{p},media=disk",
            "-bios",
            shell.wget("https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd"),
            "-s",
            "-S",
        ]
    )


@cli.command("s", "image/start", "Run the system inside the emulator")
def _(args: StartArgs):
    if args.arch not in ["x86_64"]:
        raise RuntimeError(f"Unsupported architecture: {args.arch}")

    p = generateImg(
        Image(model.Registry.use(args), f"continuum-{args.arch}"), args.arch
    )
    shell.exec(
        *[
            "qemu-system-x86_64",
            "-no-reboot",
            "-no-shutdown",
            # "-display",
            # "none",
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
