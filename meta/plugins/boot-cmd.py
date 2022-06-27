#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from json import dump as json_dump
from osdk import build, utils
from shutil import copy

_DEFAULT_LOADER = "limine"

def bootCmd(opts: dict, args: list[str]):
    sysroot = utils.mkdirP(".build/sysroot")
    efi = utils.mkdirP(".build/sysroot/EFI/BOOT")
    boot = utils.mkdirP(".build/sysroot/boot")

    loader =  _DEFAULT_LOADER if "loader" not in opts else opts["loader"]

    match loader:
        case "limine":
            efi_exe = utils.downloadFile("https://github.com/limine-bootloader/limine/raw/v3.8.2-binary/BOOTX64.EFI")
            copy(efi_exe, os.path.join(efi, "BOOTX64.EFI"))
            copy("src/limine/limine.cfg", os.path.join(boot, "limine.cfg"))
        case _:
            print(f"{utils.Colors.RED}Unknown loader {loader} {utils.Colors.RESET}")
            exit(1)

    ovmf = utils.downloadFile(
        "https://retrage.github.io/edk2-nightly/bin/DEBUGX64_OVMF.fd")

    manifest = utils.loadJson("./meta/targets/kernel-x86_64.json")
    manifest["props"]["loader"] = loader

    if "debug" in opts:
        manifest["props"]["debug_mode"] = True

    with open("./meta/targets/current_build.json", "w") as f:
        json_dump(manifest, f)

    navy = build.buildOne("current_build", "kernel")
    copy(navy, os.path.join(boot, "kernel.elf"))

    os.remove("./meta/targets/current_build.json")

    qemuCmd = [
        "qemu-system-x86_64",
        "-no-shutdown",
        "-no-reboot",
        "-d", "guest_errors",
        "-serial", "mon:stdio",
        "-bios", ovmf,
        "-m", "256M",
        "-smp", "4",
        "-drive", f"file=fat:rw:{sysroot},media=disk,format=raw",
        "-enable-kvm"
    ]


    if "debug" in opts:
        qemuCmd += ["-s", "-S"]

    utils.runCmd(*qemuCmd)

__plugin__ = {
    "name": "boot",
    "desc": "Boot Navy ! (use --loader=value to change the bootloader)",
    "func": bootCmd
}