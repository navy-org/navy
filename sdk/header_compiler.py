#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from .utils import *

import os
from typing import TextIO


def need_compile(path: path, header_path: path) -> bool:
    for file in os.listdir(path):
        h_path = os.path.join(path, file)
        if (
            not os.path.isfile(header_path)
            or os.stat(h_path).st_mtime > os.stat(header_path).st_mtime
        ):
            return True
        elif os.path.isdir(h_path) and need_compile(h_path, header_path):
            return True

    return False


def merge_headers(path: path, fp: TextIO) -> None:
    for file in os.listdir(path):
        if os.path.isfile(os.path.join(path, file)):
            if file.endswith(".h"):
                with open(os.path.join(path, file), "r") as f:
                    for line in f.readlines():
                        if line.strip() == "#pragma once":
                            pass
                        else:
                            fp.write(line)
        elif os.path.isdir(os.path.join(path, file)):
            merge_headers(os.path.join(path, file), fp)
        else:
            raise Exception(f"Unknown path type: {os.path.join(path, file)}")


def compile_header(lib_path):
    header_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".headers")
    if not os.path.isdir(header_path):
        os.mkdir(header_path)

    if need_compile(
        lib_path, header_path
    ):
        fp = open(os.path.join(header_path, os.path.basename(lib_path)), "w")
        fp.write("#pragma once\n")
        merge_headers(lib_path, fp)
        fp.close()
