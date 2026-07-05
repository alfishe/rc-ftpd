# rc-ftpd build environment
# Based on bebbo's amiga-gcc toolchain

FROM amigadev/m68k-amigaos-gcc:latest

LABEL maintainer="rc-ftpd"
LABEL description="Build environment for rc-ftpd Amiga FTP server"

# Fix GPG keys and install make
RUN apt-get update --allow-insecure-repositories || true && \
    apt-get install -y --allow-unauthenticated make || \
    (apt-get install -y gnupg && \
     apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 6ED0E7B82643E131 78DBA3BC47EF2265 F8D2585B8783D481 && \
     apt-get update && apt-get install -y make) && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Default: build release
CMD ["make", "release"]
# Build trigger
