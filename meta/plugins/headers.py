import os

from .hooks import hook

from cutekit import const, model, shell
from cutekit.builder import TargetScope
from pathlib import Path

@hook("generate-global-aliases")
def _(scope: TargetScope):
    generatedDir = Path(shell.mkdir(os.path.join(const.GENERATED_DIR, "__aliases__")))
    for c in scope.registry.iter(model.Component):
        if c.type != model.Kind.LIB:
            continue

        modPath = shell.either(
            [
                os.path.join(c.dirname(), "_mod.h"),
                os.path.join(c.dirname(), "mod.h"),
            ]
        )

        aliasPath = generatedDir / c.id
        if modPath is None or os.path.exists(aliasPath):
            continue

        targetPath = f"{c.id}/{os.path.basename(modPath)}"
        print(f"Generating alias  <{c.id}> -> <{Path(modPath).absolute()}>")
        # We can't generate an alias using symlinks because
        # because this will break #pragma once in some compilers.
        with open(aliasPath, "w") as f:
            f.write("#pragma once\n")
            f.write(f'#include "{Path(modPath).absolute()}"\n')
