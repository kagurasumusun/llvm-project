# ios-triangle — end-to-end device app for the fork's Metal toolchain

This directory is the payload of the two-leg CI pipeline in
`ci/workflows/metal-build.yml`:

1. **Linux job** — the freshly built fork clang compiles `triangle.metal`
   through the full driver pipeline to `triangle.air` (raw LLVM bitcode):
   ```
   build/bin/clang -target air64_v28-apple-ios26.0.0 -x metal -std=metal3.2 \
       -emit-llvm -c ci/metal/ios-triangle/triangle.metal -o triangle.air
   ```
2. **macOS job** — `llvm-metallib` (this tree's in-tree container writer,
   built from source on the runner) wraps the `.air` into
   `triangle.metallib`, and Xcode 26.5's `swiftc` compiles the Swift sources
   against the iOS 26 device SDK into an **unsigned** `TriangleApp.app`
   (`arm64` iPhoneOS only — no simulator slice, no codesign invocation, no
   signing identity required).

The app loads the container with `-[MTLDevice makeLibraryWithURL:]`** —
not the Xcode-generated default library** — and draws a single RGB triangle
via an `MTKView`. Because the library bytes come from this repository's
clang and container writer, a green pipeline end-to-end is proof the fork's
frontend output loads under the real Metal runtime on a production iOS SDK.

## Layout

| file | role |
|---|---|
| `triangle.metal` | vertex/fragment shaders for the triangle; written without `<metal_stdlib>` (typdefs like in `clang/test/Metal`) so the stock fork suffices |
| `Sources/AppDelegate.swift` | sceneless UIKit app delegate (`@main`) |
| `Sources/TriangleViewController.swift` | `MTKViewDelegate` rendering a 3-vertex `.triangle` primitive list, positions computed inside the vertex shader |
| `Info.plist` | minimal device bundle description; `MinimumOSVersion` 26.0 |

An unsigned device app cannot be installed through the ordinary flow; the
artifact exists to prove the toolchain chain `.metal -> .air -> .metallib ->
device binary` and to be signed downstream by whoever owns an identity.
