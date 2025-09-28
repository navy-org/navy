from cutekit import builder, model
from typing import Callable, Union, Literal

Hook = Callable[[builder.TargetScope], None]
_hooks: dict[str, Hook] = {}


def hook(name: str) -> Callable[[Hook], Hook]:
    def decorator(func: Hook):
        _hooks[name] = func
        return func

    return decorator


# ----- Monkey patch -----

_original_build = builder.build


def build(
    scope: builder.TargetScope,
    components: Union[list[model.Component], model.Component, Literal["all"]] = "all",
    generateCompilationDb: bool = False,
) -> list[builder.ProductScope]:
    for k, v in _hooks.items():
        print(f"Running hook '{k}'")
        v(scope)
    return _original_build(scope, components, generateCompilationDb)


builder.build = build
