#!/usr/bin/env bash
# Package a Windows build (run from Git Bash or an MSYS2 shell): <base>-sdk.zip (cmake --install tree) and
# <base>-gallery.zip (self-contained gallery: windeployqt + the compiler runtime).
#
# Environment: VERSION, QT_MAJOR, STAGE (cmake --install prefix), DIST (output directory),
#              ARCH     x64 | x86 | arm64 (default x64)
#              RUNTIME  msvc (default: copy the MSVC runtime from Visual Studio's redist folder)
#                       mingw (MSYS2 toolchains: copy every DLL the package imports from the MSYS2 prefix)
#              FLAVOR   optional file name suffix, e.g. "mingw"
set -euo pipefail
: "${VERSION:?}" "${QT_MAJOR:?}" "${STAGE:?}" "${DIST:?}"
ARCH="${ARCH:-x64}"
RUNTIME="${RUNTIME:-msvc}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BASE="QFunnyWidgets-${VERSION}-windows-qt${QT_MAJOR}-${ARCH}${FLAVOR:+-$FLAVOR}"
WORK="$(mktemp -d)"
mkdir -p "$DIST"
DIST="$(cd "$DIST" && pwd)"

zipdir() { # zipdir <archive> <directory name inside $WORK>
    (cd "$WORK" && 7z a -tzip -bso0 -bsp0 "$1" "$2")
}

echo "== SDK archive"
cp -R "$STAGE" "$WORK/$BASE-sdk"
cp "$ROOT/README.md" "$ROOT/LICENSE.md" "$WORK/$BASE-sdk/"
zipdir "$DIST/$BASE-sdk.zip" "$BASE-sdk"

echo "== Gallery folder"
GALLERY="$WORK/$BASE-gallery"
mkdir -p "$GALLERY"
cp "$STAGE/bin/funnywidgets-mainwindow.exe" "$GALLERY/QFunnyWidgets.exe"
WINDEPLOYQT="$(command -v "windeployqt-qt${QT_MAJOR}" || command -v "windeployqt${QT_MAJOR}" || command -v windeployqt)"
"$WINDEPLOYQT" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "$GALLERY/QFunnyWidgets.exe"

if [ "$RUNTIME" = "msvc" ]; then
    # App-local MSVC runtime of the right architecture (System32 may hold another architecture's copy,
    # e.g. on Windows on ARM), so the gallery also starts without the VC++ redistributable installed.
    vswhere="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
    vs="$(cygpath -u "$("$vswhere" -latest -products '*' -property installationPath | tr -d '\r')")"
    crt="$(find "$vs/VC/Redist/MSVC" -maxdepth 3 -type d -path "*/$ARCH/Microsoft.VC14*.CRT" | sort -V | tail -n 1)"
    if [ -z "$crt" ]; then
        echo "::error::No MSVC runtime redist found for $ARCH under $vs/VC/Redist/MSVC"
        exit 1
    fi
    echo "MSVC runtime from $crt"
    cp "$crt"/*.dll "$GALLERY/"
else
    # MSYS2: copy every non-system DLL imported (recursively) by the executable and the deployed Qt files.
    prefix="$(cd "$(dirname "$(command -v "$WINDEPLOYQT")")/.." && pwd)"
    echo "Resolving DLL imports against $prefix/bin"
    queue=$(find "$GALLERY" -iname '*.exe' -o -iname '*.dll')
    while [ -n "$queue" ]; do
        next=""
        for file in $queue; do
            for dll in $(llvm-objdump -p "$file" | sed -n 's/^ *DLL Name: *//p' | tr -d '\r'); do
                [ -e "$GALLERY/$dll" ] && continue
                src="$(find "$prefix/bin" -maxdepth 1 -iname "$dll" | head -n 1)"
                if [ -n "$src" ]; then
                    cp "$src" "$GALLERY/"
                    next="$next $GALLERY/$(basename "$src")"
                fi
            done
        done
        queue="$next"
    done
fi
cp "$ROOT/LICENSE.md" "$GALLERY/"
test -f "$GALLERY/platforms/qwindows.dll"
ls "$GALLERY"/Qt*Widgets.dll > /dev/null

echo "== Launch check (bundled Qt only)"
PWSH="$(command -v pwsh || true)"
[ -z "$PWSH" ] && [ -x "/c/Program Files/PowerShell/7/pwsh.exe" ] && PWSH="/c/Program Files/PowerShell/7/pwsh.exe"
if [ -z "$PWSH" ]; then
    echo "::notice::PowerShell not found, skipping the launch check"
else
    # The $ signs below belong to the PowerShell script, hence the single quotes.
    # shellcheck disable=SC2016
    GALLERY_WIN="$(cygpath -w "$GALLERY/QFunnyWidgets.exe")" "$PWSH" -NoProfile -Command '
        # Keep only Windows system directories on PATH, so nothing but the package can provide DLLs.
        $env:PATH = (($env:PATH -split ";") | Where-Object { $_ -match "^[A-Za-z]:\\Windows" }) -join ";"
        Remove-Item Env:QT_PLUGIN_PATH -ErrorAction SilentlyContinue
        $p = Start-Process -FilePath $env:GALLERY_WIN -PassThru
        Start-Sleep -Seconds 6
        if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force; Write-Host "Gallery is running from the package."; exit 0 }
        $code = $p.ExitCode
        # 0xC0000135 (DLL not found) and 0xC000007B (bad image / wrong architecture) are packaging errors;
        # anything else may be the runner session (no interactive desktop), so only warn.
        if ($code -eq -1073741515 -or $code -eq -1073741701) { Write-Host "::error::Missing or broken DLL in the package (exit code $code)"; exit 1 }
        Write-Host "::warning::The packaged gallery exited early (code $code)"
        exit 0
    '
fi

zipdir "$DIST/$BASE-gallery.zip" "$BASE-gallery"
ls -la "$DIST"
