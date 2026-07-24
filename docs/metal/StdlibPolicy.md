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
