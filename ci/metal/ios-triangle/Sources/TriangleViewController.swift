import Metal
import MetalKit
import UIKit

/// Draws the triangle defined by the fork-built triangle.metallib.
///
/// The library is loaded explicitly from the app bundle rather than through
/// the default library, so the renderer exercises exactly the bytes the
/// fork's clang + llvm-metallib produced.
final class TriangleViewController: UIViewController, MTKViewDelegate {
  private var mtkView: MTKView?
  private var queue: MTLCommandQueue?
  private var pipeline: MTLRenderPipelineState?

  override func viewDidLoad() {
    super.viewDidLoad()
    view.backgroundColor = .black

    guard let device = MTLCreateSystemDefaultDevice() else {
      showError("MTLCreateSystemDefaultDevice() returned nil (Metal device or simulator?)")
      return
    }
    queue = device.makeCommandQueue()

    let mtk = MTKView(frame: view.bounds, device: device)
    mtk.autoresizingMask = [.flexibleWidth, .flexibleHeight]
    mtk.colorPixelFormat = .bgra8Unorm
    mtk.clearColor = MTLClearColor(red: 0.05, green: 0.05, blue: 0.08, alpha: 1)
    mtk.delegate = self
    view.addSubview(mtk)
    mtkView = mtk

    // The fork-produced container, not a shader archive from Xcode.
    guard let url = Bundle.main.url(forResource: "triangle", withExtension: "metallib")
    else {
      showError("triangle.metallib missing from the app bundle")
      return
    }

    do {
      let library = try device.makeLibrary(URL: url)
      guard
        let vertex = library.makeFunction(name: "triangle_vertex"),
        let fragment = library.makeFunction(name: "triangle_fragment")
      else {
        showError("triangle entry points not found in the library")
        return
      }

      let desc = MTLRenderPipelineDescriptor()
      desc.label = "ForkTriangle"
      desc.vertexFunction = vertex
      desc.fragmentFunction = fragment
      desc.colorAttachments[0].pixelFormat = mtk.colorPixelFormat
      pipeline = try device.makeRenderPipelineState(descriptor: desc)
    } catch {
      showError("failed to build render pipeline: \(error.localizedDescription)")
      return
    }
  }

  func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {}

  func draw(in view: MTKView) {
    guard
      let pipeline = pipeline,
      let queue = queue,
      let drawable = view.currentDrawable,
      let pass = view.currentRenderPassDescriptor,
      let buffer = queue.makeCommandBuffer(),
      let encoder = buffer.makeRenderCommandEncoder(descriptor: pass)
    else { return }

    encoder.setRenderPipelineState(pipeline)
    encoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3)
    encoder.endEncoding()
    buffer.present(drawable)
    buffer.commit()
  }

  private func showError(_ message: String) {
    let label = UILabel(frame: view.bounds.insetBy(dx: 16, dy: 16))
    label.text = message
    label.textColor = .systemRed
    label.font = .preferredFont(forTextStyle: .body)
    label.numberOfLines = 0
    label.autoresizingMask = [.flexibleWidth, .flexibleHeight]
    view.addSubview(label)
  }
}
