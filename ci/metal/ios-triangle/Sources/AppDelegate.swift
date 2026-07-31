import UIKit

// Unsigned, sceneless iOS test app for the fork's Metal toolchain.
//
// The app loads triangle.metallib -- built on Linux by the fork's clang
// (.air) and on macOS by llvm-metallib -- and draws a triangle on a real
// iOS device. Compiled with Xcode 26.5's swiftc against the iOS 26 device
// SDK; no code signing.

@main
final class AppDelegate: UIResponder, UIApplicationDelegate {
  var window: UIWindow?

  func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication
      .LaunchOptionsKey: Any]?
  ) -> Bool {
    let window = UIWindow(frame: UIScreen.main.bounds)
    window.rootViewController = TriangleViewController()
    window.makeKeyAndVisible()
    self.window = window
    return true
  }
}
