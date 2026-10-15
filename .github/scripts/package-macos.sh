#!/usr/bin/env bash
# Package a macOS build: <base>-sdk.tar.gz (cmake --install tree) and <base>-gallery.dmg (self-contained app).
#
# Environment: VERSION, QT_MAJOR, STAGE (cmake --install prefix), DIST (output directory), ARCH (e.g. universal).
set -euo pipefail
: "${VERSION:?}" "${QT_MAJOR:?}" "${STAGE:?}" "${DIST:?}"
ARCH="${ARCH:-universal}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BASE="QFunnyWidgets-${VERSION}-macos-qt${QT_MAJOR}-${ARCH}"
WORK="$(mktemp -d)"
mkdir -p "$DIST"
DIST="$(cd "$DIST" && pwd)"

echo "== SDK archive"
cp -R "$STAGE" "$WORK/$BASE-sdk"
cp "$ROOT/README.md" "$ROOT/LICENSE.md" "$WORK/$BASE-sdk/"
tar -C "$WORK" -czf "$DIST/$BASE-sdk.tar.gz" "$BASE-sdk"

echo "== Gallery app"
mkdir -p "$WORK/dmg"
APP="$WORK/dmg/QFunnyWidgets Gallery.app"
cp -R "$STAGE/funnywidgets-mainwindow.app" "$APP"
macdeployqt "$APP" -always-overwrite
# macdeployqt rewrites the binaries, which invalidates their signatures; Apple Silicon refuses to run unsigned
# code, so re-sign ad hoc (no Apple Developer ID needed; users right-click > Open the first time).
codesign --force --deep --sign - "$APP"
codesign --verify --deep --strict "$APP"
test -d "$APP/Contents/Frameworks/QtWidgets.framework"
test -f "$APP/Contents/PlugIns/platforms/libqcocoa.dylib"
lipo -info "$APP/Contents/MacOS/funnywidgets-mainwindow"

echo "== Launch check (non-fatal: depends on the runner having a window server)"
set +e
env -u DYLD_LIBRARY_PATH -u DYLD_FRAMEWORK_PATH -u QT_PLUGIN_PATH \
    "$APP/Contents/MacOS/funnywidgets-mainwindow" > "$WORK/launch.log" 2>&1 &
pid=$!
sleep 6
if kill -0 "$pid" 2>/dev/null; then
    kill "$pid"
    echo "Gallery is running from the bundle."
else
    wait "$pid"
    echo "::warning::The packaged gallery exited early (code $?); check the launch log below"
    cat "$WORK/launch.log"
fi
set -e

echo "== Disk image"
ln -s /Applications "$WORK/dmg/Applications"
for attempt in 1 2 3; do                           # hdiutil occasionally reports "Resource busy" on CI
    if hdiutil create -volname "QFunnyWidgets $VERSION" -srcfolder "$WORK/dmg" -ov -format UDZO \
        "$DIST/$BASE-gallery.dmg"; then
        break
    fi
    [ "$attempt" -eq 3 ] && exit 1
    sleep 5
done
ls -la "$DIST"
