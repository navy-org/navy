import os
from cutekit import model, cli, builder, rules, ensure

ensure((0, 7, 0))


@cli.command("f", "flag", "Generate flags for C/C++ compiler")
def _(args: cli.Args):
    tool = args.consumeOpt("tool", "cc")

    registry = model.Registry.use(args)
    target = model.Target.use(args)
    scope = builder.TargetScope(registry, target)

    if tool not in target.tools:
        raise Exception(f"Target {target.name} does not have tool {tool}")

    arguments = target.tools[tool].args

    if tool in ["cc", "cxx"]:
        flags = rules.rules[tool].args + builder._vars['cincs'](scope).split() + builder._vars['cdefs'](scope).split()

    i = 0
    while i < len(arguments):
        if "mcmode" in arguments[i]:
            pass
        elif "-target" in arguments[i]:
            i += 1
        else:
            flags.append(arguments[i])
        i += 1

    with open(os.path.join(registry.project.dirname(), "compile_flags.txt"), "w") as f:
        f.write("\n".join(flags))
        f.write("\n")