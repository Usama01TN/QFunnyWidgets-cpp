#!/usr/bin/env bash
# Package a Linux build: <base>-sdk.tar.gz (cmake --install tree) and <base>-gallery.AppImage (self-contained).
#
# Environment: VERSION, QT_MAJOR, STAGE (cmake --install prefix), DIST (output directory),
#              ARCH (default x86_64), QMAKE (default: qmake from PATH; used by linuxdeploy-plugin-qt).
set -euo pipefail
: "${VERSION:?}" "${QT_MAJOR:?}" "${STAGE:?}" "${DIST:?}"
ARCH="${ARCH:-x86_64}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BASE="QFunnyWidgets-${VERSION}-linux-qt${QT_MAJOR}-${ARCH}"
WORK="$(mktemp -d)"
mkdir -p "$DIST"
DIST="$(cd "$DIST" && pwd)"

echo "== SDK archive"
cp -R "$STAGE" "$WORK/$BASE-sdk"
cp "$ROOT/README.md" "$ROOT/LICENSE.md" "$WORK/$BASE-sdk/"
tar -C "$WORK" -czf "$DIST/$BASE-sdk.tar.gz" "$BASE-sdk"

echo "== Gallery AppImage"
TOOLS="${RUNNER_TEMP:-/tmp}/linuxdeploy"
mkdir -p "$TOOLS"
for tool in linuxdeploy linuxdeploy-plugin-qt; do
    file="$TOOLS/$tool-$ARCH.AppImage"
    if [ ! -s "$file" ]; then
        curl -fsSL --retry 3 -o "$file" \
            "https://github.com/linuxdeploy/$tool/releases/download/continuous/$tool-$ARCH.AppImage"
    fi
    chmod +x "$file"
done
APPDIR="$WORK/AppDir"
mkdir -p "$APPDIR/usr/bin" "$WORK/icon"
cp "$STAGE/bin/funnywidgets-mainwindow" "$APPDIR/usr/bin/"
cp "$ROOT/QFunnyWidgets/googlyeyes/logo.png" "$WORK/icon/qfunnywidgets.png"
export APPIMAGE_EXTRACT_AND_RUN=1                  # no FUSE needed (CI containers, runners)
export QMAKE="${QMAKE:-$(command -v qmake)}"
export EXTRA_PLATFORM_PLUGINS="libqoffscreen.so"   # lets the launch check below run headless
export OUTPUT="$DIST/$BASE-gallery.AppImage"       # file name for the appimage output plugin
export LDAI_OUTPUT="$OUTPUT"
(cd "$WORK" && "$TOOLS/linuxdeploy-$ARCH.AppImage" --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/funnywidgets-mainwindow" \
    --desktop-file "$ROOT/packaging/linux/qfunnywidgets-gallery.desktop" \
    --icon-file "$WORK/icon/qfunnywidgets.png" \
    --plugin qt --output appimage)
test -s "$OUTPUT"
chmod +x "$OUTPUT"

echo "== Launch check (bundled Qt only, headless)"
set +e
env -u LD_LIBRARY_PATH -u QT_PLUGIN_PATH -u QML2_IMPORT_PATH QT_QPA_PLATFORM=offscreen \
    timeout 8 "$APPDIR/AppRun" > "$WORK/launch.log" 2>&1
rc=$?
set -e
if [ "$rc" -ne 124 ]; then                         # 124 = still running when timeout stopped it
    cat "$WORK/launch.log"
    echo "::error::The packaged gallery exited early (code $rc)"
    exit 1
fi
echo "Gallery is running from the AppDir."
ls -la "$DIST"
