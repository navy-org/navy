from cutekit import builder, cli, model
from pathlib import Path

class Builder:
    def __init__(self, args: cli.Args, arch: str = "x86_64"):
        self.__registry = model.Registry.use(args)
        self.__kernel_scope = builder.TargetScope(
            self.__registry,
            self.__registry.lookup(f"kernel-{arch}", model.Target),
        )

        self.__user_scope = builder.TargetScope(
            self.__registry,
            self.__registry.lookup(f"navy-{arch}", model.Target),
        )

    def build_core(self) -> Path:
        core_component = self.__registry.lookup("core", model.Component)
        product = builder.build(self.__kernel_scope, core_component)[0]
        return Path(product.path).absolute()

    def build_modules(self) -> list[Path]:
        modules = dict(filter(lambda x: type(x[1]) is model.Component and \
                          x[1].type == model.Kind.EXE and x[0] != "core", self.__registry.manifests.items()))

        products = []
        for component in modules.values():
            products.append(Path(builder.build(self.__user_scope, component)[0].path).absolute())
        
        return products

    @property
    def arch(self) -> str:
        return self.__kernel_scope.target.id.split("-")[1]