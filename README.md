# glxgears-newos

A NEWOS userspace gears demo rendered by the upstream TinyGL software rasterizer and presented through NEWOS's XRGB8888 framebuffer syscall.

This is **not an X11/GLX implementation**: NEWOS does not currently provide an X server or GLX ABI. The familiar gears scene is rendered directly to the Limine framebuffer using TinyGL. The upstream TinyGL repository is a pinned Git submodule at `vendor/tinygl`.

## Status

The port source and build integration are in progress. It must be built against a NEWOS checkout with the `x86_64-elf` toolchain and this repository's TinyGL submodule initialized. The app opts into floating-point code; NEWOS's x86_64 FPU setup saves/restores per-thread state. This repository has not yet been boot-tested in QEMU, so it must not be described as a verified working port until that test passes.

## Build

From a NEWOS checkout after initializing this component and its nested TinyGL dependency:

```sh
git submodule update --init --recursive ports/glxgears-newos
make toolchain
make glxgears-newos
```

The NEWOS Makefile invokes this component's Makefile, which uses `toolchain/newos-gcc`, the NEWOS user ABI/library, and TinyGL's C sources. It produces `build/userland/glxgears-newos.elf`; the optional package recipe installs it as `/bin/glxgears-newos`. The demo needs a Limine boot with a framebuffer and is capped to a finite number of frames by default so it returns to the shell.

When building the component outside the NEWOS tree, set `NEWOS_ROOT` explicitly:

```sh
make -C /path/to/glxgears-newos NEWOS_ROOT=/path/to/NEWOS
```

## Upstream

- TinyGL: https://github.com/jserv/tinygl (MIT; pinned as a submodule)
- Gear scene: based on the public-domain 3D gears demo by Brian Paul, adapted for NEWOS's framebuffer ABI.

Keep TinyGL's upstream license and attribution with all redistributions.
