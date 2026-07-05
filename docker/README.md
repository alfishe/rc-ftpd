# Docker Build

Build rc-ftpd in a container without installing the toolchain locally.

## Prerequisites

- Docker

## Quick Start

```bash
# Build Docker image (first time only)
docker build -t rc-ftpd-builder -f docker/Dockerfile .

# Build and create release
docker run --rm -v $(pwd):/src rc-ftpd-builder make clean release
```

Output will be in `release/` folder.

## Make Targets

```bash
# Build binary only
docker run --rm -v $(pwd):/src rc-ftpd-builder make

# Clean build
docker run --rm -v $(pwd):/src rc-ftpd-builder make clean

# Build and copy to dist/
docker run --rm -v $(pwd):/src rc-ftpd-builder make dist

# Create release package
docker run --rm -v $(pwd):/src rc-ftpd-builder make release

# Interactive shell
docker run --rm -it -v $(pwd):/src rc-ftpd-builder /bin/bash
```

## Base Image

Uses `amigadev/m68k-amigaos-gcc` which includes:
- bebbo's amiga-gcc cross-compiler
- m68k-amigaos-gcc, as, ld, etc.
- NDK 3.2 headers and libraries
- libnix runtime
- lha archiver
