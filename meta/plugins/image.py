from hashlib import blake2b
from cutekit import model, builder, const, shell
from pathlib import Path


class Image:
    def __init__(self, registry: model.Registry, name: str):
        self.__registry: model.Registry = registry
        self.__name = name

    @property
    def path(self) -> Path:
        return Path(const.PROJECT_CK_DIR) / "images" / self.__name

    def build(self, component_spec: str, target_spec: str) -> Path:
        comp = self.__registry.lookup(component_spec, model.Component)
        target = self.__registry.lookup(target_spec, model.Target)

        assert comp is not None
        assert target is not None

        scope = builder.TargetScope(self.__registry, target)
        return Path(builder.build(scope, comp)[0].path)

    def cp(self, src: Path, dst: Path):
        shell.cp(str(src), str(self.path / str(dst).removeprefix("/")))

    def mkdir(self, name: str):
        (self.path / str(name).removeprefix("/")).mkdir(parents=True, exist_ok=True)

    def wget(self, link: str, dst: str):
        shell.wget(link, str(self.path / str(dst).removeprefix("/")))

    def get_hash(self, target: Path) -> str:
        with (self.path / str(target).removeprefix("/")).open("rb") as f:
            return blake2b(f.read()).hexdigest()

    def export_limine(self, target: Path, kernel: Path, modules: list[Path]):
        rel_path = Path(self.path / str(target).removeprefix("/"))
        with (rel_path / "limine.cfg").open("w") as f:
            f.writelines(
                [
                    "RANDOMIZE_MEMORY=yes\n",
                    "TIMEOUT=0\n",
                    f":{self.__name}\n",
                    "PROTOCOL=limine\n",
                    f"KERNEL_PATH=boot://{kernel}#{self.get_hash(kernel)}\n",
                ]
                + [
                    f"MODULE=boot://{module}#{self.get_hash(module)}\n"
                    for module in modules
                ]
            )
