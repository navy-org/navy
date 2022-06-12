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
        result[manifest_path] = json_load(fp)
        result[manifest_path]["src"] = find_source(os.path.join(basedir, src))

        if "download" in result[manifest_path]:
            tmp = result[manifest_path]["download"]
            downloadFile(tmp["src"], tmp["dst"], tmp["sysroot"])

        if "to-sysroot" in result[manifest_path]:
            for file in result[manifest_path]["to-sysroot"]:
                dst_dir = os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    ".build",
                    "sysroot",
                    os.path.dirname(file["dst"][1:]),
                )

                if not os.path.isdir(dst_dir):
                    Path(dst_dir).mkdir(
                        parents=True, exist_ok=True
                    )
                copyfile(
                    os.path.join(os.path.dirname(manifest_path), file["src"]),
                    os.path.join(dst_dir, os.path.basename(file["dst"])),
                )

        if result[manifest_path]["type"] == "lib":
            compile_header(os.path.join(basedir, src))

    return result
