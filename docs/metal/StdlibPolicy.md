# Metal stdlib policy

The Metal frontend bring-up should use Apple Metal stdlib headers as the source of truth.

## Policy

- Do **not** grow a hand-written replacement `metal::` stdlib in Clang.
- The compiler may provide internal builtin hooks that Apple headers expect, such as opaque builtin object types and `__metal_*` entry-point declarations/lowering.
- User-facing APIs such as `metal::sin`, `metal::texture2d<T>`, texture method overload sets, and sampler/texture wrapper classes should come from Apple stdlib headers or generated compatibility data derived from those headers.
- `kagurasumusun/metal-info` must be consumed through GitHub API/raw files, not cloned, when extracting stdlib facts.

## Current bootstrap exception

The current lightweight prelude still contains minimal scalar/vector aliases, opaque builtin object declarations, and some `__metal_*` builtin entry declarations/lowering so that Clang can compile focused frontend/AIR smoke tests before the full Apple header path is wired in. These are compiler hooks, not a substitute for Apple stdlib.

## Next direction

1. Wire an external Apple stdlib include/resource path for development and CI smoke.
2. Generate builtin signature tables from Apple headers where Clang needs compiler builtin declarations.
3. Remove any remaining user-facing wrapper shims once Apple headers provide the real API in tests.


## CI Apple stdlib setup

The fast Metal smoke workflow now fetches Apple Metal headers and Darwin runtime artifacts from `kagurasumusun/metal-info` into `third-party/metal-info-apple` at test time. Tests that need the real Apple stdlib should pass `-I third-party/metal-info-apple/reference-apple/clang/32023.883/include/metal` and include `<metal_stdlib>`. Runtime artifacts are placed under `reference-apple/clang/32023.883/lib/darwin` for later link/runtime experiments.


## CI environment

The fast Metal smoke workflow downloads Apple Metal stdlib headers and Darwin runtime libraries from `kagurasumusun/metal-info` into `metal-info-resource/clang/32023.883` at test time. The workflow does this via GitHub API/raw file downloads, not by cloning `metal-info`.

Current blocking smoke verifies that `<metal_stdlib>` can be found and preprocessed through this fetched include path. Full semantic compilation of Apple stdlib is expected to expose additional Clang compatibility gaps and should be enabled incrementally as those gaps are fixed.
