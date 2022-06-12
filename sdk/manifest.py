#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from .utils import *
from .header_compiler import compile_header

import os
from json import load as json_load

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

        if result[manifest_path]["type"] == "lib":
            compile_header(os.path.join(basedir, src))
    
    return result