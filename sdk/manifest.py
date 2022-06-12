#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from .utils import *
from .header_compiler import compile_header

import os
from json import load as json_load
from shutil import copyfile


def find_source(basedir) -> list[path]:
    ret = []
    for node in os.listdir(basedir):
        filename = os.path.join(basedir, node)
        if os.path.isfile(filename):
            if filename.endswith(".c"):
                ret.append(filename)
        elif os.path.isdir(node):
            ret += find_source(filename)

    return ret


def compileManifests(basedir: path) -> dict[path, json]:
    result: dict[path, json] = {}
    for src in os.listdir(basedir):
        manifest_path = os.path.join(basedir, src, "manifest.json")
        if not os.path.isfile(manifest_path):
            raise Exception("Invalid package")

        fp = open(manifest_path, "r")
        manifest = json_load(fp)
        result[manifest["id"]] = manifest
        result[manifest["id"]]["src"] = find_source(os.path.join(basedir, src))

        if "install" in manifest:
            for install in manifest["install"]:
                if len(install.keys()) > 2 or "dst" not in install:
                    raise Exception("What are you doing ?")

                if "url" in install:
                    downloadFile(install["url"], install["dst"], True)
                elif "src" in install:
                    dst_dir = os.path.join(
                        os.path.dirname(os.path.realpath(__file__)),
                        "..",
                        ".build",
                        "sysroot",
                        os.path.dirname(install["dst"][1:]),
                    )

                    if not os.path.isdir(dst_dir):
                        Path(dst_dir).mkdir(parents=True, exist_ok=True)
                    copyfile(
                        os.path.join(os.path.dirname(manifest_path), install["src"]),
                        os.path.join(dst_dir, os.path.basename(install["dst"])),
                    )

        if manifest["type"] == "lib":
            compile_header(os.path.join(basedir, src))

    return result
