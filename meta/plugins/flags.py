import os
from cutekit import model, cli, builder, rules, ensure

ensure((0, 7, 0))


class ToolArgs(model.TargetArgs):
    tool: str = cli.arg("t", "tool", "Tool to generate flags for", "cc")


@cli.command("f", "flag", "Generate flags for C/C++ compiler")
def _(args: ToolArgs):
    registry = model.Registry.use(args)
    target = model.Target.use(args)
    scope = builder.TargetScope(registry, target)

    if args.tool not in target.tools:
        raise Exception(f"Target {target.name} does not have tool {args.tool}")

    arguments = target.tools[args.tool].args

    if args.tool in ["cc", "cxx"]:
        flags = (
            rules.rules[args.tool].args
            + builder._vars["cincs"](scope)
            + builder._vars["cdefs"](scope)
        )

    if args.tool == "cxx":
        flags.append("-xc++")

    i = 0
    while i < len(arguments):
        if "mcmode" in arguments[i]:
            pass
        else:
            flags.append(arguments[i])
        i += 1

    with open(os.path.join(registry.project.dirname(), "compile_flags.txt"), "w") as f:
        f.write("\n".join(flags))
        f.write("\n")
