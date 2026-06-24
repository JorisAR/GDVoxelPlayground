#!/usr/bin/env python
import os
import sys
# from scons_compiledb import compile_db # Import the compile_db function # Call the compile_db function to enable compile_commands.json generation 
# compile_db()


# Import the SConstruct from godot-cpp
env = SConscript("godot-cpp/SConstruct")

# Add necessary include directories
env.Append(CPPPATH=[
    "src/gdcs/include/",
    "src/utility/",
    "src/",
    "src/voxel_rendering/",
    "src/voxel_world/",
    "src/voxel_world/generator/",
    "src/voxel_world/generator/cpu_passes/",
    "src/voxel_world/generator/cpu_passes/wave_function_collapse/",
    "src/voxel_world/cellular_automata/"
    "src/voxel_world/voxel_edit/"
    "src/voxel_world/colliders/"
    "src/voxel_world/data/"
])

# # Add main source files
sources = Glob("src/*.cpp") + Glob("src/utility/*.cpp") + Glob("src/gdcs/src/*.cpp") + \
      Glob("src/voxel_rendering/*.cpp") + Glob("src/voxel_world/*.cpp") + \
      Glob("src/voxel_world/generator/*.cpp") + Glob("src/voxel_world/generator/cpu_passes/*.cpp") + Glob("src/voxel_world/generator/cpu_passes/wave_function_collapse/*.cpp") +\
      Glob("src/voxel_world/cellular_automata/*.cpp") + Glob("src/voxel_world/voxel_edit/*.cpp") + \
      Glob("src/voxel_world/colliders/*.cpp") + Glob("src/voxel_world/data/*.cpp")

#compiler flags
if env['PLATFORM'] == 'windows':
    if env['CXX'] == 'x86_64-w64-mingw32-g++':
        env.Append(CXXFLAGS=['-std=c++11'])  # Example flags for MinGW
    elif env['CXX'] == 'cl':
        env.Append(CXXFLAGS=['/EHsc'])  # Apply /EHsc for MSVC


# Handle different platforms
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "demo/addons/voxel_playground/bin/voxel_playground.{}.{}.framework/voxel_playground.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "demo/addons/voxel_playground/bin/voxel_playground.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "demo/addons/voxel_playground/bin/voxel_playground.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "demo/addons/voxel_playground/bin/voxel_playground{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
