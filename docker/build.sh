#!/bin/bash
# Build rc-ftpd using Docker

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
IMAGE_NAME="rc-ftpd-builder"

cd "$PROJECT_DIR"

# Build the Docker image if needed
if ! docker image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
    echo "Building Docker image..."
    docker build -t "$IMAGE_NAME" -f docker/Dockerfile .
fi

# Run container to build release
echo "Building rc-ftpd..."
docker run --rm \
    -v "$PROJECT_DIR:/src" \
    "$IMAGE_NAME" \
    make clean release

echo ""
echo "Build complete. Output in release/"
ls -la release/
