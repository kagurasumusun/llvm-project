# Metal language-standard spellings and platform version guide

This file records the current working model for Metal language standard names and approximate platform/deployment versions. It is based primarily on the `metal-info` reference artifact filenames/AST dumps and should be refined against Apple PDF/spec text and real toolchain behavior as implementation continues.

## Key rule

Apple's older Metal language-standard names are platform-qualified. Do not model old versions primarily as unqualified `metal1.0`, `metal1.1`, `metal2.0`, etc.

Use:

- iOS-family: `ios-metal1.0` ... `ios-metal2.4`
- macOS-family: `macos-metal1.1` ... `macos-metal2.4`
- legacy macOS aliases: `osx-metal1.1`, `osx-metal1.2`, `osx-metal2.0`
- Metal 3+: unqualified `metal3.0`, `metal3.1`, `metal3.2`, `metal4.0`, `metal4.1`

`-std=metal` remains an alias for `metal4.0` in this branch.

## Approximate version mapping observed in metal-info artifacts

### iOS-family artifacts

The `metal-info/reference/metal-ast-ios-*` artifacts use the following mapping:

| Standard spelling | Observed OS version in artifact filenames |
|---|---:|
| `ios-metal1.0` | iOS 10.3 |
| `ios-metal1.1` | iOS 11.4 |
| `ios-metal1.2` | iOS 12.5 |
| `ios-metal2.0` | iOS 13.7 |
| `ios-metal2.1` | iOS 14.8 |
| `ios-metal2.2` | iOS 15.8 |
| `ios-metal2.3` | iOS 16.7 |
| `ios-metal2.4` | iOS 17.7 |
| `metal3.0` | iOS 18.7 |
| `metal3.1` | iOS 26.0 |
| `metal3.2` | iOS 26.0 |
| `metal4.0` | iOS 26.0 |

### tvOS-family artifacts

The tvOS and tvOS-simulator artifacts use `ios-metal*` spellings for old versions:

| Standard spelling | Observed OS version in artifact filenames |
|---|---:|
| `ios-metal1.1` | tvOS artifact OS 11.4 |
| `ios-metal1.2` | tvOS artifact OS 12.5 |
| `ios-metal2.0` | tvOS artifact OS 13.7 |
| `ios-metal2.1` | tvOS artifact OS 14.8 |
| `ios-metal2.2` | tvOS artifact OS 15.8 |
| `ios-metal2.3` | tvOS artifact OS 16.7 |
| `ios-metal2.4` | tvOS artifact OS 17.7 |
| `metal3.0` | tvOS artifact OS 18.7 |
| `metal3.1` | tvOS artifact OS 26.0 |
| `metal3.2` | tvOS artifact OS 26.0 |
| `metal4.0` | tvOS artifact OS 26.0 |

No `ios-metal1.0` entries were observed in tvOS artifacts.

### watchOS-family artifacts

The watchOS artifacts currently show fewer old standards:

| Standard spelling | Observed OS version in artifact filenames |
|---|---:|
| `ios-metal1.0` | watchOS artifact OS 10.3 |
| `ios-metal1.1` | watchOS artifact OS 11.4 |
| `metal3.1` | watchOS artifact OS 26.0 |
| `metal3.2` | watchOS artifact OS 26.0 |
| `metal4.0` | watchOS artifact OS 26.0 |

The missing 1.2/2.x/3.0 entries may be an artifact collection gap or a real platform/toolchain constraint. Treat as tentative until verified against Apple docs/PDFs/toolchain.

### macOS artifacts

The `metal-info/reference/metal-ast-macos-*` artifacts use `macos-metal*` and older `osx-metal*` spellings:

| Standard spelling | Observed OS version in artifact filenames |
|---|---:|
| `macos-metal1.1` / `osx-metal1.1` | macOS 10.13 |
| `macos-metal1.2` / `osx-metal1.2` | macOS 10.14 |
| `macos-metal2.0` / `osx-metal2.0` | macOS 10.15 |
| `macos-metal2.1` | macOS 11.7 |
| `macos-metal2.2` | macOS 12.7 |
| `macos-metal2.3` | macOS 13.7 |
| `macos-metal2.4` | macOS 14.7 |
| `metal3.0` | macOS 15.7 |
| `metal3.1` | macOS 26.0 |
| `metal3.2` | macOS 26.0 |
| `metal4.0` | macOS 26.0 |

No `macos-metal1.0` was observed. macOS should be modeled as beginning at `macos-metal1.1` for old-platform-qualified standards.

## PDF coverage in metal-info

The `metal-info/metal-reference` directory contains specification PDFs for these versions:

- Metal Shading Language Specification 1.2
- 2.0
- 2.1
- 2.2
- 2.3
- 2.4
- 3.0
- 3.1
- 3.2
- 4.0
- 4.1

The PDFs are the authoritative spec-version source, but the artifact filename matrix is currently the most directly machine-readable guide for toolchain `-std` spellings and observed platform/deployment target pairings.

## Current implementation policy

- Remove unqualified `metal1.*` and `metal2.*` standard names.
- Keep `metal3.*` and `metal4.*` unqualified.
- Add compatibility checks so `ios-metal*` is not used with macOS targets, and `macos-metal*` / `osx-metal*` are not used with iOS-family targets.
- Later add deployment-target minimum checks using the tables above.
