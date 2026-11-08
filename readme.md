# Plushie 🧸

*My namespace for working on self-learning projects. You might find that they are slightly disconnected in theme but will primarily focus on `C/C++` `graphics` and other low-level concepts.*

## Build

The build runs through cmake presets, although this is primarily a meta build for CI on all my projects 

```bash
# Configure and build with preset for your desired platform
cmake --preset windows-x64-msvc    

# Build presets are the config prefix name plus a optimization
cmake build --preset windows-x64-msvc-dbg
cmake build --preset windows-x64-msvc-rel 


# Available presets for your host can be listed with 
cmake --list-presets
```

## Modules

Once a project reaches a point that it is not just an exploritory phase, or I know I am actually going to be working on it *[For example gfx module needs lots of work, but I will keep using it]* Then it moves from `modules_lab` to `modules`

The `plushie_cstl` is it's own standalone module which is header only implementations which are reusable across multiple projects and functions as a standard library. Which has it's own synchronisation rules for publishing into it's own lighter repositorty. 

## Targets

List of priorities of the targets, and notes about the target

| Target Name                     | Notes                                                                                                                            |
| ------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| `windows-x64-msvc`              | Native windows msvc build. Doesn't specify a generator so that CMake selects the correct visual studio generator                 |
| `windows-x64-msvc-cross`        | Cross compiling for msvc from linux or windows. MSVC is the only target with a special cross target, to compensate for generator |
| `windows-x64-mingw`             | Windows mingw via native or cross compile                                                                                        |
| ............................... | .                                                                                                                                |
