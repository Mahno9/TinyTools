#!/usr/bin/env bash
# Configure, build, test and bundle TinyTools for Linux in the Podman container.
#
# Usage: scripts/build-linux.sh
# Output: dist/TinyTools/ (self-contained, see scripts/install-linux.sh)
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

IMAGE=tinytools-builder
BUILD_DIR=build-linux

echo "==> Building container image ($IMAGE)"
podman build -f Containerfile.linux -t "$IMAGE" .

echo "==> Configuring, building and testing"
podman run --rm -v "$repo_root":/src -w /src "$IMAGE" bash -c '
    set -euo pipefail
    cmake -S . -B '"$BUILD_DIR"' -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build '"$BUILD_DIR"'
    ctest --test-dir '"$BUILD_DIR"' --output-on-failure
    bash scripts/package-linux.sh
'

echo "==> Done: $repo_root/dist/TinyTools"
echo "    Install with: scripts/install-linux.sh"
