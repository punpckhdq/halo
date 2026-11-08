Halo 1 decompilation
[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress]
=============

<!--
Replace with your repository's URL.
-->
[Build Status]: https://github.com/punpckhdq/halo/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/punpckhdq/halo/actions/workflows/build.yml
<!--
decomp.dev progress badges
See https://decomp.dev/api for an API overview.
-->
[Code Progress]: https://decomp.dev/punpckhdq/halo.svg?mode=shield&measure=code&label=Code
[Data Progress]: https://decomp.dev/punpckhdq/halo.svg?mode=shield&measure=data&label=Data
[progress]: https://decomp.dev/punpckhdq/halo

This is a work-in-progress decompilation of Halo: Combat Evolved build 2342 (`cachebeta.exe`, sha256 `4cc87b45f721270392a96f1674ed2b5cd4a7bb4355faeab4531d1cf1884d9520`).

## Build instructions

You must source the Halo 1 PAL debug build and the August 2001 Xbox SDK yourself. You also need [ninja-build](https://ninja-build.org/) on your PATH.

Extract the `XDK/xbox` folder from the installer into the repository root such that `xbox/{bin,include}` are valid paths. Place `cachebeta.exe` from the Halo 1 build into `orig/2342/`. Navigate to the repository and run `configure.py` in the terminal.

Then, use `ninja build` to compile the game and report progress statistics, or open the `cachebeta.sln` VS2022 project under `projects` on Windows.

## Where's all the type information?

We use debug symbols from later Halo games to help map structures, enums, type definitions, function signatures and some variable names. You should obtain a copy of the Halo CEA beta and run it through [pdb-decompiler](https://github.com/camden-smallwood/pdb-decompiler) to obtain debug info for yourself. Note that CEA is based off of Halo PC and has its own modifications which make it not 100% accurate to the original Xbox title.
