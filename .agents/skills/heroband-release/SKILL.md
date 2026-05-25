---
name: heroband-release
description: Repo-only Heroband release workflow for tagging, packaging, validating, and publishing prereleases. Use when the user asks to generate, publish, tag, upload, or cut a Heroband release.
metadata:
  short-description: Publish Heroband releases
---

# Heroband Release

Use this repo-local skill when releasing Heroband. Heroband release work must
keep version strings, tags, artifacts, checksums, and public documentation in
sync while preserving the central moral rule: the player may fight evil, but may
not wield evil.

## Required Flow

1. Read `specs/release-packaging.md`, `specs/docs-help.md`, `README.md`, and
   `AGENTS.md` before editing or publishing.
2. Confirm the target version and tag, normally `heroband-X.Y.Z`.
3. Ensure user-facing download text names the same version and artifact names
   that will be published.
4. Run the gameplay/build validation required by the change being released.
5. Commit the release-ready source revision before tagging.
6. Create an annotated release tag on the exact release commit.
7. Push the final source revision and tag.
8. Verify whether remote release automation is available. If it is unavailable,
   use the manual fallback below.
9. Verify the published release assets and checksums after upload.

## Automation Availability Check

Do not assume a checked-in workflow is available remotely.

```sh
gh workflow list --repo mtomcal/heroband
gh workflow view release.yaml --repo mtomcal/heroband
```

If `release.yaml` is listed and usable, prefer the workflow path. If GitHub does
not expose that workflow, publish manually and state that the fallback was used.

## Manual Artifact Fallback

Use this path when release automation is disabled, unavailable, or unsuitable.

```sh
version=heroband-X.Y.Z
release_dir="$(mktemp -d /tmp/heroband-release-${version#heroband-}.XXXXXX)"

cmake -G Ninja -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_DOC=ON
cmake --build build-release -j2

python3 -m venv .venv/doc-builder
.venv/doc-builder/bin/pip install -U sphinx sphinx-better-theme
cmake -G Ninja -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_DOC=ON \
  -DSPHINX_EXECUTABLE="$PWD/.venv/doc-builder/bin/sphinx-build"
cmake --build build-release --target OurManual -j2

worktree="$release_dir/worktree"
git worktree add --detach "$worktree" "$version"
(cd "$worktree" && scripts/pkg_src)
mv "$worktree/angband-${version}.tar.gz" \
  "$release_dir/${version}-source.tar.gz"
git worktree remove "$worktree"

mkdir -p "$release_dir/${version}-linux-x86_64"
stage="$release_dir/${version}-linux-x86_64"
cp -a build-release/game/angband README.md changes.txt "$stage/"
cp -a build-release/game/lib "$stage/lib"
cp -a build-release/manual-output/html "$stage/docs"
find "$stage/lib/save" "$stage/lib/panic" "$stage/lib/scores" -type f -delete 2>/dev/null || true
find "$stage/lib/user" -type f -delete 2>/dev/null || true
mkdir -p "$stage/lib/user/save" "$stage/lib/user/scores" "$stage/lib/user/archive" "$stage/lib/user/panic"
touch "$stage/lib/user/delete.me" "$stage/lib/user/save/delete.me" \
  "$stage/lib/user/scores/delete.me" "$stage/lib/user/archive/delete.me" \
  "$stage/lib/user/panic/delete.me"
(cd "$release_dir" && tar -czf "${version}-linux-x86_64.tar.gz" "${version}-linux-x86_64")

(cd "$release_dir" && sha256sum "${version}-linux-x86_64.tar.gz" \
  "${version}-source.tar.gz" > "${version}-checksums.txt")
```

If the source archive prefix must be `heroband-X.Y.Z/` rather than
`angband-heroband-X.Y.Z/`, repack it before checksums and upload.

## Release Validation

Run these before publishing:

```sh
git diff --check
cmake --build build -j2
cmake --build build -t alltests -j2
sha256sum -c "$release_dir/${version}-checksums.txt"
tmp="$(mktemp -d /tmp/heroband-release-smoke.XXXXXX)"
tar -xzf "$release_dir/${version}-linux-x86_64.tar.gz" -C "$tmp"
(cd "$tmp/${version}-linux-x86_64" && ./angband -v)
```

If `alltests` exits successfully but prints an intermittent suspicious suite
line, rerun that suite directly and report both facts.

## Publish

```sh
gh release create "$version" \
  "$release_dir/${version}-checksums.txt" \
  "$release_dir/${version}-linux-x86_64.tar.gz" \
  "$release_dir/${version}-source.tar.gz" \
  --repo mtomcal/heroband \
  --title "Heroband ${version#heroband-}" \
  --notes-file "$release_dir/release-notes.md" \
  --prerelease

gh release view "$version" --repo mtomcal/heroband
```

Keep generated release directories and virtualenvs untracked.
