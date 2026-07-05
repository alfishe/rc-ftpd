# Minimal rc-ftpd build environment
# Multi-stage build to extract only the toolchain (~460MB vs 7.4GB)

FROM amigadev/m68k-amigaos-gcc:latest AS toolchain

FROM debian:bookworm-slim

# Copy the toolchain and lha
COPY --from=toolchain /opt/amiga /opt/amiga
COPY --from=toolchain /usr/local/bin/lha /usr/local/bin/lha

# Install minimal build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    libmpc3 \
    && rm -rf /var/lib/apt/lists/*

ENV PATH="/opt/amiga/bin:${PATH}"

WORKDIR /src

CMD ["make", "release"]
