//===----------------------------------------------------------------------===//
// libtracepoint_rt — Metal debug tracepoint runtime
// Provides GPU debug/profiling tracepoints
//===----------------------------------------------------------------------===//
typedef unsigned uint;

void __metal_tracepoint_begin(uint tag) {}
void __metal_tracepoint_end(uint tag) {}
void __metal_tracepoint_value(uint tag, uint value) {}
void __metal_tracepoint_string(uint tag, const char *str) {}
void __metal_tracepoint_counter(uint tag, uint count) {}
void __metal_tracepoint_timestamp(uint tag) {}
