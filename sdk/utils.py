#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from pathlib import Path
from requests import get as get_request

json = dict[any]
path = str


def downloadFile(source: path, dest: path, is_sysroot: bool):
    dst_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")

    if is_sysroot:
        dst_path = os.path.join(dst_path, ".build", "sysroot")

    r = get_request(source, allow_redirects=True)

    if r.status_code != 200:
        assert Exception(f"Impossible to download {dest}")

    dst_path = os.path.join(dst_path, os.path.dirname(dest[1:]))
    if not os.path.isdir(os.path.dirname(dst_path)):
        Path(dst_path).mkdir(parents=True, exist_ok=True)

    with open(os.path.join(dst_path, os.path.basename(dest[1:])), "wb") as f:
        f.write(r.content)
