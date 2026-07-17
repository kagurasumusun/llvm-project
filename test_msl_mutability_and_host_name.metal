#include <metal_stdlib>
using namespace metal;

// Mutability attribute test (MSL 2.3+) and Host Name attribute test
struct PipelineConfig {
    device float* read_buffer [[mutability("read")]];
    device float* write_buffer [[mutability("write")]];
    device float* rw_buffer [[mutability("read_write")]];
};

[[kernel]] void test_pipeline_config(device PipelineConfig& cfg [[buffer(0)]],
                                     uint tid [[thread_position_in_grid]]) [[host_name("custom_kernel_host")]] {
    float val = cfg.read_buffer[tid] + cfg.rw_buffer[tid];
    cfg.write_buffer[tid] = val * 2.0f;
    cfg.rw_buffer[tid] = val + 1.0f;
}
