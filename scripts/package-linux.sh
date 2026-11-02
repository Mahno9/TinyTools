#!/usr/bin/env bash
# Package a self-contained Linux bundle of TinyTools.
#
# Run INSIDE the build container, from the repository root:
#   podman run --rm -v "$PWD":/src -w /src tinytools-builder \
#       bash scripts/package-linux.sh
#
# Result: dist/TinyTools/ — copy it anywhere (no Qt installation required).
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-/src/build-linux}"
OUT_DIR="${OUT_DIR:-/src/dist/TinyTools}"
QT_PLUGINS="/usr/lib/x86_64-linux-gnu/qt6/plugins"
QT_LIBEXEC="/usr/lib/qt6/libexec"
QT_DATA="/usr/share/qt6"

if [[ ! -x "$BUILD_DIR/TinyTools" ]]; then
    echo "error: $BUILD_DIR/TinyTools not found - build the project first" >&2
    exit 1
fi

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"/{bin,lib,plugins,libexec,resources,translations}

echo "==> Copying application"
install -m 0755 "$BUILD_DIR/TinyTools" "$OUT_DIR/bin/TinyTools"

echo "==> Copying Qt plugins and WebEngine runtime"
cp -a "$QT_PLUGINS/." "$OUT_DIR/plugins/"
install -m 0755 "$QT_LIBEXEC/QtWebEngineProcess" "$OUT_DIR/libexec/"
cp -a "$QT_DATA/resources/." "$OUT_DIR/resources/"
cp -a "$QT_DATA/translations/qtwebengine_locales" "$OUT_DIR/translations/"

# Libraries provided by the host: glibc and its NSS modules, the C++ runtime,
# and the graphics/X11/Wayland stack (GPU drivers must match the host kernel).
DENY_RE='^(ld-linux.*|libc|libm|libdl|libpthread|librt|libresolv|libutil|libanl|libBrokenLocale|libthread_db|libnss_(files|dns|compat|hesiod|myhostname|mdns4.*|mdns6.*|systemd)|libstdc\+\+|libgcc_s|libGL|libEGL|libGLX|libGLdispatch|libOpenGL|libgbm|libdrm|libvulkan|libglapi|libX11|libxcb|libXau|libXdmcp|libXext|libXrender|libXi|libXfixes|libXcursor|libXrandr|libXcomposite|libXdamage|libXinerama|libXKB|libxkbfile|libxshmfence|libwayland|libxkbcommon)'

copy_deps_of() {
    local file="$1" resolved
    while IFS= read -r resolved; do
        [[ -f "$resolved" ]] || continue
        local base
        base="$(basename "$resolved")"
        # Strip version suffix for matching: libFoo.so.1.2.3 -> libFoo
        if [[ "$base" =~ $DENY_RE ]]; then
            continue
        fi
        if [[ ! -e "$OUT_DIR/lib/$base" ]]; then
            cp -L "$resolved" "$OUT_DIR/lib/$base"
        fi
    done < <(ldd "$file" 2>/dev/null | awk '/=> \// { print $3 }')
}

echo "==> Resolving shared library dependencies"
# The executable, the WebEngine helper and every bundled plugin/backend are
# entry points that can pull in libraries the main binary never references.
while IFS= read -r -d '' elf; do
    copy_deps_of "$elf"
done < <(find "$OUT_DIR" -type f \( -name '*.so*' -o -name 'TinyTools' -o -name 'QtWebEngineProcess' \) -print0)

cat > "$OUT_DIR/bin/qt.conf" <<'EOF'
[Paths]
Prefix = ..
Plugins = plugins
Libraries = lib
Data = .
Translations = translations
LibraryExecutables = libexec
EOF

cat > "$OUT_DIR/bin/tinytools" <<'EOF'
#!/bin/sh
# Launcher for the bundled TinyTools build.
self=$(readlink -f "$0")
bin_dir=$(dirname "$self")
bundle=$(dirname "$bin_dir")

LD_LIBRARY_PATH="$bundle/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH

# QtWebEngine does not always honor qt.conf for its data files, so point it
# at the bundled copies explicitly.
QTWEBENGINE_RESOURCES_PATH="$bundle/resources"
QTWEBENGINE_LOCALES_PATH="$bundle/translations/qtwebengine_locales"
export QTWEBENGINE_RESOURCES_PATH QTWEBENGINE_LOCALES_PATH

# WebEngine sandbox: an unconfined helper cannot create a user namespace on
# distributions that restrict them (Ubuntu 23.10+). Apps can opt out here if
# the sandbox is unavailable; harmless where it works.
if [ "${TINYTOOLS_NO_SANDBOX:-0}" = "1" ]; then
    QTWEBENGINE_DISABLE_SANDBOX=1
    export QTWEBENGINE_DISABLE_SANDBOX
fi

exec "$bin_dir/TinyTools" "$@"
EOF
chmod 0755 "$OUT_DIR/bin/tinytools"

echo "==> Bundle ready: $OUT_DIR"
du -sh "$OUT_DIR"
