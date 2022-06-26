#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from osdk import build, utils
from shutil import copy

LOADER = "limine"

def bootCmd(opts: dict, args: list[str]):
    sysroot = utils.mkdirP(".build/sysroot")
    efi = utils.mkdirP(".build/sysroot/EFI/BOOT")
    boot = utils.mkdirP(".build/sysroot/boot")

    ovmf = utils.downloadFile(
        "https://retrage.github.io/edk2-nightly/bin/DEBUGX64_OVMF.fd")

    navy = build.buildOne("kernel-x86_64", "kernel")
    copy(navy, os.path.join(boot, "kernel.elf"))

    match LOADER:
        case "limine":
            efi_exe = utils.downloadFile("https://github.com/limine-bootloader/limine/raw/v3.8.2-binary/BOOTX64.EFI")
            copy(efi_exe, os.path.join(efi, "BOOTX64.EFI"))
            copy("src/limine/limine.cfg", os.path.join(boot, "limine.cfg"))
        case _:
            print(f"{utils.Colors.RED} Unknown loader {LOADER} {utils.Colors.RESET}")
            exit(1)

    qemuCmd = [
        "qemu-system-x86_64",
        "-no-reboot",
        "-d", "guest_errors",
        "-serial", "mon:stdio",
        "-bios", ovmf,
        "-m", "256M",
        "-smp", "4",
        "-drive", f"file=fat:rw:{sysroot},media=disk,format=raw",
        "-enable-kvm"
    ]

    utils.runCmd(*qemuCmd)

__plugin__ = {
    "name": "boot",
    "desc": "Boot Navy !",
    "func": bootCmd
}