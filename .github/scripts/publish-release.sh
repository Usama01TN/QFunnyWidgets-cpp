#!/usr/bin/env bash
# Publish the packages in dist/ as a GitHub release, with the gh CLI (preinstalled on GitHub-hosted runners).
#
# Environment:
#   PUBLISH   release | nightly
#   TAG       release: the version tag (v1.2.3); created at COMMIT when it does not exist yet (manual release).
#             nightly: the rolling tag (nightly), moved to COMMIT on every run.
#   VERSION   the version used in the package names (1.2.3, 1.2.3-rc1, 1.2.3-dev.<sha>)
#   COMMIT    the commit to release
#   DIST      directory with the packages and SHA256SUMS.txt (default: dist)
#   GH_TOKEN, GH_REPO  for gh
set -euo pipefail
: "${PUBLISH:?}" "${TAG:?}" "${VERSION:?}" "${COMMIT:?}"
DIST="${DIST:-dist}"
NOTES="$(mktemp)"

cat > "$NOTES" <<'EOF'
**Packages** (one pair per OS / architecture / Qt version):
- `*-gallery.*`: the self-contained widget gallery (AppImage / zip / dmg); no Qt installation needed.
- `*-sdk.*`: everything to build against QFunnyWidgets with that Qt: headers, the **static** library
  (`QFunnyWidgetsStatic`), the **shared** library (`QFunnyWidgets`: `.so` / `.dylib` / `.dll` + import library),
  the CMake package (`find_package(QFunnyWidgets)` -> `QFunnyWidgets::Static`, `QFunnyWidgets::Shared`),
  the Qt Designer plugin and `funnywidgets-designer`.

`SHA256SUMS.txt` lists the checksums. See the README for which Qt and compiler each SDK expects.
EOF

case "$PUBLISH" in
nightly)
    {
        echo
        echo "---"
        echo "Nightly build of commit ${COMMIT} ($(date -u '+%Y-%m-%d %H:%M UTC'))."
        echo "This pre-release is replaced on every push to the main branch; for stable packages use a versioned release."
    } >> "$NOTES"
    # Replace the previous nightly: delete the release and its tag, then publish this commit under the same tag.
    if gh release view "$TAG" > /dev/null 2>&1; then
        gh release delete "$TAG" --cleanup-tag --yes
    fi
    # A bare tag without a release (e.g. left by an interrupted run) would pin the old commit: remove it too.
    if gh api "repos/{owner}/{repo}/git/ref/tags/$TAG" > /dev/null 2>&1; then
        gh api -X DELETE "repos/{owner}/{repo}/git/refs/tags/$TAG" > /dev/null
    fi
    gh release create "$TAG" "$DIST"/* \
        --target "$COMMIT" \
        --title "QFunnyWidgets nightly ($VERSION)" \
        --notes-file "$NOTES" \
        --prerelease \
        --latest=false
    ;;
release)
    if gh release view "$TAG" > /dev/null 2>&1; then
        # Re-run of a release (e.g. after a flaky job): keep the release, refresh its packages.
        echo "Release $TAG already exists: replacing its packages"
        gh release upload "$TAG" "$DIST"/* --clobber
    else
        extra=()
        if [[ "$TAG" == *-* ]]; then
            extra+=(--prerelease)                 # v1.1.0-rc1 and similar
        fi
        # --target only matters when the tag does not exist yet (manual release); a pushed tag is kept as is.
        gh release create "$TAG" "$DIST"/* \
            --target "$COMMIT" \
            --title "QFunnyWidgets $VERSION" \
            --notes-file "$NOTES" \
            --generate-notes \
            "${extra[@]}"
    fi
    ;;
*)
    echo "::error::Unknown PUBLISH mode '$PUBLISH' (expected release or nightly)"
    exit 1
    ;;
esac

url="$(gh release view "$TAG" --json url --jq .url)"
echo "Published: $url"
if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
    echo "### Published [$TAG]($url)" >> "$GITHUB_STEP_SUMMARY"
fi
