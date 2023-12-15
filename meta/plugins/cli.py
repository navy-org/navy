from cutekit import cli, model

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
            build.Builder(args)
        )
    ).run()


@cli.command("B", "navy/build", "Build Navy")
def _(args: cli.Args):
    build.Builder(args).build_core()