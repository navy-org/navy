import os

from cutekit import shell

from .image import Image

class Qemu:
    def __init__(self, img: Image, no_reboot: bool = True, nographics: bool = False,
                 no_shutdown: bool = True, efi: bool = True, memory: str = "4G", debug: bool = False):
        self.__img = img 
        self.__binary = None
        self.__efi = efi

        self.__args = [
            "-m", str(memory),
        ]

        if debug:
            self.__args += [
                "-s", "-S"
            ]

        if no_reboot:
            self.__args.append("-no-reboot")

        if no_shutdown:
            self.__args.append("-no-shutdown")

        if nographics:
            self.__args.append("-nographic")

        if self.iskvmAvailable():
            self.__args += ["-enable-kvm", "-cpu", "host"]

        try:
            getattr(self, f"arch_{img.builder.arch}")()
        except AttributeError:
            raise NotImplementedError()

    def iskvmAvailable(self) -> bool:
        return os.path.exists("/dev/kvm") and os.access("/dev/kvm", os.R_OK)

    def arch_x86_64(self):
        self.__binary = "qemu-system-x86_64"
        self.__args += [
            "-smp", "4",
            "-debugcon", "mon:stdio"
        ]

        if self.__efi:
            ovmf = shell.wget(
                "https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd"
            )

            self.__args += [
                "-drive", f"format=raw,file=fat:rw:{self.__img},media=disk",
                "-bios", ovmf
            ]

    def run(self):
        if self.__binary is None:
            raise RuntimeError("Architecture is not set")

        shell.exec(self.__binary, *self.__args)