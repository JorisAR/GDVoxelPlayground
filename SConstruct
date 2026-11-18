#!/usr/bin/env python
import os

# Default to an optimised release build; pass target=template_debug (or editor) to override.
env = Environment(tools=["default"], PLATFORM="", target=ARGUMENTS.get("target", "template_release"))
Export("env")
env = SConscript("godot-cpp/SConstruct")

ADDON_DIR = "demo/addons/voxel_playground"

env.Append(CPPPATH=[
    "src/",
    "src/utility/",
    "src/compute/",
    "src/voxel_rendering/",
    "src/voxel_world/",
    "src/voxel_world/generator/",
    "src/voxel_world/generator/cpu_passes/",
    "src/voxel_world/generator/cpu_passes/wave_function_collapse/",
    "src/voxel_world/cellular_automata/",
    "src/voxel_world/voxel_edit/",
    "src/voxel_world/colliders/",
    "src/voxel_world/data/",
])


def collect_sources(root):
    result = []
    for dirpath, _, filenames in os.walk(root):
        for name in filenames:
            if name.endswith(".cpp"):
                result.append(File(os.path.join(dirpath, name)))
    return result


sources = collect_sources("src")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{}/bin/voxel_playground.{}.{}.framework/voxel_playground.{}.{}".format(
            ADDON_DIR, env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    suffix = ".simulator.a" if env["ios_simulator"] else ".a"
    library = env.StaticLibrary(
        "{}/bin/voxel_playground.{}.{}{}".format(ADDON_DIR, env["platform"], env["target"], suffix),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/bin/voxel_playground{}{}".format(ADDON_DIR, env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
