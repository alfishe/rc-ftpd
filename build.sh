#!/bin/bash
# Build rc-ftpd using bebbo's amiga-gcc
export PATH=/opt/amiga/bin:$PATH
cd "$(dirname "$0")"
make "$@"
