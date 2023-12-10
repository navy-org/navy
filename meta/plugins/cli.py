from cutekit import cli, model

from . import build
from . import image
from . import qemu

@cli.command("k", "kernel", "kernel related commands")
def _(args: cli.Args):
    pass


@cli.command("b", "kernel/boot", "Boot kernel")
def _(args: cli.Args):
    qemu.Qemu(
        image.Image(
            build.Builder(args)
        )
    ).run()


@cli.command("B", "kernel/build", "Build kernel")
def _(args: cli.Args):
    build.Builder(args).build_core()