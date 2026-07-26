// libtracepoint_rt_static — Static tracepoint runtime
// Compiled into the binary at link time, activated by runtime flags
typedef unsigned uint;
typedef unsigned long ulong;

void __metal_static_trace_begin(uint category, uint label) {}
void __metal_static_trace_end(uint category, uint label) {}
void __metal_static_trace_value(uint category, uint label, uint value) {}
void __metal_static_trace_float(uint category, uint label, float value) {}
void __metal_static_trace_message(uint category, uint label, const char *msg) {}
void __metal_static_trace_counter(uint category, uint label, uint count) {}
void __metal_static_trace_timestamp(uint category, uint label) {}
void __metal_static_trace_enable(uint category) {}
void __metal_static_trace_disable(uint category) {}
void __metal_static_trace_reset(void) {}
uint __metal_static_trace_is_enabled(uint category) { return 0; }
