from gettext import install
import os
from ninja import Writer
from json import load as json_read
from pathlib import Path
from requests import get as get_request
from shutil import copyfile

src_folder = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "src")
dst_folder = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".build")


def build_manifest(manifest, config, writer, deps):
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

    objs = []
    out = os.path.join(dst_folder, manifest["id"])
    src = os.path.join(src_folder, manifest["id"])

    installManifest(manifest, src)

    for file in os.listdir(os.path.join(src_folder, manifest["id"])):
        if file.endswith(".c"):
            objs.append(os.path.join(out, f"{file}.o"))
            writer.build(objs[-1], cc, os.path.join(src, file))
        elif file.endswith(".s"):
            objs.append(os.path.join(out, f"{file}.o"))
            writer.build(objs[-1], "as", os.path.join(src, file))

    bin = ""
    if manifest["type"] == "exe":
        for dep in deps:
            if dep != manifest["id"]:
                objs.append(os.path.join(out, "..", "sysroot", "lib", f"{dep}.a"))

        bin = os.path.join(
            os.path.join(out, "..", "sysroot", "bin", f"{manifest['id']}.elf")
        )
        writer.build(bin, ld, objs)
    elif manifest["type"] == "lib":
        bin = os.path.join(
            os.path.join(out, "..", "sysroot", "lib", f"{manifest['id']}.a")
        )
        writer.build(bin, "ar", objs)
    else:
        return None

    writer.newline()
    return bin


def genNinja(fp, deps, cfg):
    all = []
    writer = Writer(fp)
    writer.comment("This build file is auto-generated (btw)")
    writer.newline()

    writer.rule(
        "cc",
        f"{cfg['cc']} -c -o $out $in -MD -MF $out.d {' '.join(cfg['cflags'])}",
        depfile="$out.d",
    )

    writer.rule("ld", f"{cfg['ld']} -o $out $in {' ' .join(cfg['ldflags'])}")
    writer.rule("as", f"{cfg['as']} -o $out $in {' '.join(cfg['asflags'])}")
    writer.rule("ar", f"{cfg['ar']} {cfg['arflags']} $out $in")
    writer.newline()

    for dep in deps:
        with open(os.path.join(src_folder, dep, "manifest.json"), "r") as manifest:
            all.append(build_manifest(json_read(manifest), cfg, writer, deps))

    writer.build("all", "phony", all)


def find_deps(deps):
    ret = []

    def resolve(dep):
        deps = [dep]

        with open(os.path.join(src_folder, dep, "manifest.json")) as cfg:
            json = json_read(cfg)
            if "depends" in json:
                deps += find_deps(json["depends"])
        return deps

    for dep in deps:
        if dep not in ret:
            ret += resolve(dep)

    return list(dict.fromkeys(ret))


def downloadFile(source, dest):
    r = get_request(source, allow_redirects=True)

    if r.status_code != 200:
        assert Exception(f"Impossible to download {dest}")

    if not os.path.isdir(os.path.dirname(dest)):
        Path(os.path.dirname(dest)).mkdir(parents=True, exist_ok=True)

    with open(dest, "wb") as f:
        f.write(r.content)


def installManifest(manifest, manifest_path):
    path = ""

    if "install" not in manifest:
        return

    for install in manifest["install"]:
        if len(install.keys()) > 2:
            raise Exception("What are you doing ?")

        if "dst" in install:
            path = os.path.join(dst_folder, "sysroot", install["dst"][1:])
        elif "dst-out" in install:
            path = os.path.join(
                os.path.dirname(os.path.realpath(__file__)), "..", install["dst-out"][1:]
            )
        else:
            raise Exception("No destination for install")

        if os.path.isfile(path):
            continue

        if "url" in install:
            downloadFile(install["url"], path)
        elif "src" in install:
            if not os.path.isdir(os.path.dirname(path)):
                Path(os.path.dirname(path)).mkdir(parents=True, exist_ok=True)

            copyfile(
                os.path.join(os.path.dirname(manifest_path), manifest["id"], install["src"]), path
            )
        else:
            raise Exception("No source for install")


def buildAll(profile):
    deps = find_deps(profile["depends"])
    installManifest(profile, src_folder)
    ninja_fp = open(
        os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "build.ninja"),
        "w",
    )

    genNinja(ninja_fp, deps, profile)
