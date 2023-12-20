from cutekit import cli, model, shell
from pathlib import Path

from . import build
from . import image
from . import qemu

@cli.command("n", "navy", "Navy related commands")
def _(args: cli.Args):
    pass


@cli.command("b", "navy/boot", "Boot Navy")
def _(args: cli.Args):
    qemu.Qemu(
        image.Image(
            build.Builder(args),
        ),
        no_display=args.consumeOpt("no-display"),
        soft_dbg=args.consumeOpt("soft-dbg")
    ).run()


@cli.command("B", "navy/build", "Build Navy")
def _(args: cli.Args):
    build.Builder(args).build_core()
    build.Builder(args).build_modules()
