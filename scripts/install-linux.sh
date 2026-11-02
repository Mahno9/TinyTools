#!/usr/bin/env bash
# Install the bundled Linux build for the current user and register it with
# the desktop environment (application menu + autostart).
#
# Usage: scripts/install-linux.sh
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
bundle="$repo_root/dist/TinyTools"

if [[ ! -x "$bundle/bin/tinytools" ]]; then
    echo "error: $bundle not found - run scripts/build-linux.sh first" >&2
    exit 1
fi

install_dir="$HOME/.local/opt/TinyTools"
bin_link="$HOME/.local/bin/tinytools"
apps_dir="$HOME/.local/share/applications"
icons_dir="$HOME/.local/share/icons/hicolor/256x256/apps"
autostart_dir="$HOME/.config/autostart"

echo "==> Installing to $install_dir"
rm -rf "$install_dir"
mkdir -p "$install_dir"
cp -a "$bundle/." "$install_dir/"

mkdir -p "$(dirname "$bin_link")" "$apps_dir" "$icons_dir" "$autostart_dir"
ln -sfn "$install_dir/bin/tinytools" "$bin_link"
cp "$repo_root/src/resources/icons/icon.png" "$icons_dir/tinytools.png"

echo "==> Writing desktop entries"
cat > "$apps_dir/tinytools.desktop" <<EOF
[Desktop Entry]
Type=Application
Version=1.0
Name=TinyTools
Comment=Quick access to translators, AI tools and docs from one overlay
Exec=$bin_link
Icon=tinytools
Terminal=false
Categories=Utility;
StartupNotify=false
StartupWMClass=TinyTools
Keywords=tools;overlay;translate;ai;web;
EOF

cat > "$autostart_dir/tinytools.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=TinyTools
Comment=Start TinyTools with the desktop session
Exec=$bin_link
Icon=tinytools
Terminal=false
X-GNOME-Autostart-enabled=true
Hidden=false
EOF

command -v update-desktop-database >/dev/null && \
    update-desktop-database "$apps_dir" 2>/dev/null || true
command -v gtk-update-icon-cache >/dev/null && \
    gtk-update-icon-cache -q "$HOME/.local/share/icons/hicolor" 2>/dev/null || true

echo "==> Installed"
echo "    Run:        tinytools  (or launch TinyTools from the app menu)"
echo "    Autostart:  $autostart_dir/tinytools.desktop"
