#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from gettext import install
import os
import sys
import argparse
import subprocess

from shutil import rmtree
from build import *
from json import load as json_load


def runProfile(profile):
    sysroot = os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    ".build",
                    "sysroot",
                )
    
    run = [x.replace("$SYSROOT", sysroot) for x in profile["run"]] 
    subprocess.Popen(run)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Navy build system")

    parser.add_argument("profile", help="Path to a build profile", type=str, nargs="?")
    parser.add_argument(
        "-b", "--build", action="store_true", help="Compile a profile"
    )
    parser.add_argument(
        "-r", "--run", action="store_true", help="Run a profile"
    )
    parser.add_argument(
        "-c", "--clean", action="store_true", help="Clean the build directory"
    )
    parser.add_argument(
        "-n", "--nuke", action="store_true", help="Nuke the project! Like in fallout"
    )

    args = parser.parse_args()

    if not sum(filter(lambda x: type(x) != str and x is not None, vars(args).values())):
        parser.print_help()
        exit(1)

    if args.build or args.run:
        if not os.path.isfile(args.profile):
            sys.stderr.write(f"cannot find {args.profile}: no such file\n")
            exit(1)

        profile_file = open(args.profile, "r")
        profile = json_load(profile_file)

        if profile["type"] != "profile":
            sys.stderr.write("Wrong package type: should be a profile\n")
            exit(1)

    if args.build:
        buildAll(profile)
        os.system(
            f"ninja -f {os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'build.ninja')}"
        )

    if args.clean:
        if os.path.isdir(".build"):
            rmtree(".build")

    if args.run:
        buildAll(profile)

        if (
            os.system(
                f"ninja -f {os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'build.ninja')}"
            )
            == 0
        ):
            runProfile(profile)

    if args.nuke:
        try:
            os.remove("build.ninja")
            rmtree(".build")
        except FileNotFoundError:
            pass

    if args.run or args.build:
        profile_file.close()
