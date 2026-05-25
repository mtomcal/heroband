# Release Packaging

Version: 1.0.0

## Overview

Release packaging must produce versioned, reproducible Heroband artifacts for
players, source builders, platform packagers, and maintainers. A release must
ship playable assets, source archives with generated configuration files,
checksums, current documentation, and version metadata that matches the tag and
download names.

Releases must preserve Heroband's central design rule in player-facing
materials: the player may fight evil, but may not wield evil. Release notes,
manuals, downloadable archives, and packaged data must not advertise forbidden
player-accessible evil power as available gameplay.

## Dependencies

- A clean source-control checkout is required for tracked-file archive
  generation.
- A descriptive version tag is required for versioned archive names and build
  identity.
- Autotools are required when generating source archives that include generated
  configuration files.
- Platform build tools are required for playable binary archives.
- Documentation tooling is required when packaged manuals are refreshed.
- Checksum tooling is required for release integrity manifests.
- Compression tools are required for source, platform, and Windows archives.
- Platform release automation may build some assets, but maintainers remain
  responsible for validating the published artifact set.

## Parameters

| Parameter | Required Value | Rationale |
| --- | --- | --- |
| Release version source | annotated release tag or equivalent signed-off version stamp | Artifact names, build identity, documentation, and announcements must agree on the released version. |
| Source archive prefix | project name plus described version | Extracted archives must be self-identifying and avoid collisions with unrelated source trees. |
| Source archive compression | gzip-compressed tar archive | This is the established portable source-distribution format for Unix-like builders and packagers. |
| Debian source archive name style | project name and version joined by an underscore with original-source suffix | Debian packaging conventions require a distinct original-source tarball naming scheme. |
| Windows playable archive compression | zip archive | Windows users expect zip archives and the packaging flow prepares Windows line endings. |
| Checksum algorithm | SHA-256 | The current release instructions use SHA-256 for downloadable artifact integrity. |
| Source package generated files | include generated configuration script and generated autoconf header | Source-release users must be able to configure without needing autotools installed. |
| Repository metadata in source archives | excluded | Release archives should contain distributable project content, not source-control internals. |
| Windows text line endings | CRLF for text documentation and data files | Windows packages should open cleanly in native Windows tools. |
| Empty user-state directories | retained with placeholder files where needed | Runtime save, score, panic, and archive locations must exist after extraction even when empty. |
| Release validation warning threshold | no unexpected compiler warnings in supported release builds | Release builds should not ship with new warning noise that can conceal defects. |
| Checksum regeneration rule | regenerate after every artifact replacement | Integrity manifests must match the final downloadable files exactly. |

## Data Structures

- Release manifest: version, tag, artifact names, checksums, supported
  platforms, documentation status, and validation status.
- Source archive: tracked source content, version stamp, generated configure
  support, build files, data, docs, and allowed distributable assets.
- Platform playable archive: executable, required runtime data, optional
  libraries, assets, documentation, and initialized user-state directories.
- Checksum manifest: one checksum entry for each published downloadable
  artifact.
- Version registry: all user-facing and build-facing version strings that must
  match the release.
- Release checklist: compilation matrix, warning checks, documentation checks,
  version checks, tagging, upload, and announcement steps.
- Exclusion set: repository metadata, transient generated caches, and files not
  intended for a specific package format.

## Behavior

RP-B1. Source packages must be generated from tracked source content, not from a
dirty ad hoc working directory.

Test scenarios: RP-T1.

RP-B2. Source packages must include a version stamp derived from the release
description.

Test scenarios: RP-T2.

RP-B3. Source packages must include generated configuration files so release
users can perform a configure-based build without regenerating them.

Test scenarios: RP-T3.

RP-B4. Source packages must exclude source-control metadata and transient
automation files that are not part of the distributable source.

Test scenarios: RP-T4.

RP-B5. Packaging commands must fail rather than overwrite an existing output
archive or staging directory.

Test scenarios: RP-T5.

RP-B6. Debian source packages must follow Debian original-source archive
naming and exclude assets that are not intended for that source format.

Test scenarios: RP-T6.

RP-B7. Windows playable packages must include the executable, required dynamic
libraries, data, assets, sounds, manual files, and initialized user-state
directories.

Test scenarios: RP-T7.

RP-B8. Windows playable packages must convert text documentation and text data
to Windows line endings while preserving binary assets unchanged.

Test scenarios: RP-T8.

RP-B9. Self-contained playable archives must run from their extracted directory
without requiring a system install.

Test scenarios: RP-T9.

RP-B10. Checksum manifests must list every published downloadable release
artifact and must be regenerated whenever an artifact is replaced.

Test scenarios: RP-T10.

RP-B11. Release documentation must match current gameplay, including Heroband
replacement classes, corruption consequences, and unavailable forbidden powers.

Test scenarios: RP-T11.

RP-B12. All user-facing and build-facing version strings must be reviewed and
updated before tagging.

Test scenarios: RP-T12.

RP-B13. Release candidates must compile across the supported platform matrix
or document which manual platform checks remain outstanding.

Test scenarios: RP-T13.

RP-B14. Release candidates must have no unexpected warnings in the warning-
sensitive build configurations.

Test scenarios: RP-T14.

RP-B15. The release tag and final source revision must be pushed together so
release automation can see both the code and the tag.

Test scenarios: RP-T15.

RP-B16. Release announcements must describe Heroband accurately and must not
claim player access to forbidden evil power.

Test scenarios: RP-T16.

## Error Handling

- Packaging must stop if the requested output archive already exists.
- Packaging must stop if the requested staging directory already exists.
- Packaging must stop on missing required executables, libraries, generated
  documentation, runtime data, or assets.
- Source archive generation must fail if generated configuration files cannot
  be produced.
- Checksum generation must be repeated after any artifact replacement; stale
  checksum manifests are invalid.
- Version mismatch across release metadata, documentation, README text, build
  identity, and archive names must block release publication.
- Missing platform validation must be recorded as a release risk, not silently
  treated as success.
- New compiler warnings in warning-sensitive builds must block release unless
  explicitly triaged.
- Player-facing documentation that advertises forbidden player-accessible evil
  power must block release until corrected.

## Implementation Notes

- Generate release archives from source-control tracked content to avoid
  leaking local build products or user state.
- Generate configuration support before archiving source releases and remove
  generation-only helper files from the packaged tree when appropriate.
- Keep platform archive staging temporary and delete it after successful
  compression.
- Preserve empty runtime user-state directories through placeholder files when
  archive formats would otherwise omit them.
- Treat generated documentation as release content and proofread it alongside
  in-game help and top-level download instructions.
- Separate gameplay changes, bug fixes, and code maintenance in release notes
  so players and maintainers can evaluate risk.
- Prefer automated platform builds where available, then supplement them with
  documented manual checks for unautomated platforms.
- Update checksums last, after all downloadable artifacts are final.

## Test Scenarios

RP-T1. Create a source package from a clean checkout; verify only tracked
distributable content is staged.

RP-T2. Inspect the source package version stamp; verify it matches the release
tag description.

RP-T3. Extract the source package on a machine without autotools; verify the
configure-based build can begin without regenerating configuration files.

RP-T4. Inspect a source package; verify source-control metadata and transient
automation caches are absent.

RP-T5. Pre-create the requested archive or staging directory; verify packaging
fails before writing output.

RP-T6. Build the Debian source package; verify the name follows Debian
original-source conventions and excluded asset classes are absent.

RP-T7. Build the Windows playable package; verify executable, dynamic
libraries, runtime data, assets, sounds, docs, and user-state directories are
present.

RP-T8. Inspect Windows package text and binary files; verify text line endings
are converted and binary files remain valid.

RP-T9. Extract a playable archive into a new directory; verify the game starts
and finds its data without installation.

RP-T10. Generate a checksum manifest, verify all published artifacts, replace
one artifact, regenerate the manifest, and verify the old manifest no longer
matches.

RP-T11. Proof release docs and in-game help against current gameplay; verify
forbidden player-accessible evil power is not presented as available.

RP-T12. Review the version registry; verify each user-facing and build-facing
version string matches the release.

RP-T13. Run or collect platform build results for the release matrix; verify
unsupported or unchecked platforms are explicitly recorded.

RP-T14. Run warning-sensitive builds; verify no unexpected warnings remain.

RP-T15. Push the final release revision and tag in one release operation; verify
release automation can resolve both.

RP-T16. Review release announcements; verify they describe Heroband's moral
rule and current downloads accurately.

## Changelog

- 1.0.0: Fully authored release-packaging specification from existing release
  checklist, packaging scripts, versioning script, download notes, and Heroband
  release guidance.
