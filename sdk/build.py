#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from .ninja import Writer
from .manifest import *
from .utils import *

import os
from json import load as json_load
from typing import TextIO


def genNinja(fp: TextIO, config: json, manifests: dict[path, json]) -> None:
    all = []
    writer = Writer(fp)

    writer.comment("This build file is auto-generated (btw)")
    writer.newline()

    writer.rule(
        "cc",
        f"{config['cc']} -c -o $out $in -MD -MF $out.d {' '.join(config['cflags'])}",
        depfile="$out.d",
    )

    writer.rule("ld", f"{config['ld']} -o $out $in {' ' .join(config['ldflags'])}")
    writer.rule("as", f"{config['as']} -o $out $in {' '.join(config['asflags'])}")
    writer.newline()

    for manifest in manifests.values():
        objs = []
        cc = "cc"
        ld = "ld"

        if "flag_addons" in manifest:
            for add in manifest["flag_addons"]:
                match add:
                    case "cflags":
                        cc = f"{manifest['id']}_cc"
                        writer.rule(
                            f"{manifest['id']}_cc",
                            f"{config['cc']} -c -o $out $in -MD -MF $out.d {' '.join(config['cflags'] + manifest['flag_addons']['cflags'])}",
                        )
                    case "ldflags":
                        ld = f"{manifest['id']}_ld"
                        writer.rule(
                            f"{manifest['id']}_ld",
                            f"{config['ld']} -o $out $in {' '.join(config['ldflags'] + manifest['flag_addons']['ldflags'])}",
                        )
                    case _:
                        raise Exception(f"Unknown addon flag: {manifest}")

            writer.newline()

        out = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), "..", ".build"
        )

        for file in manifest["src"]:
            objs.append(os.path.join(out, f"{os.path.basename(file)}.o"))
            writer.build(objs[-1], cc, file)

        bin = ""
        if manifest["type"] == "exe":
            bin = os.path.join(os.path.join(out, "sysroot", "bin", f"{manifest['id']}.elf"))
            print(bin)
            writer.build(bin, ld, objs)
        else:
            continue

        all.append(bin)
        writer.newline()

    writer.build("all", "phony", all)


def buildAll(cfg: json) -> None:
    manifests = compileManifests(
        os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "src")
    )

    ninjaFp = open(
        os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "build.ninja"),
        "w",
    )
    genNinja(ninjaFp, cfg, manifests)
