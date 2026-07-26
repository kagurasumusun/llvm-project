// libtracepoint_rt_workaround — Workaround tracepoint runtime
// Handles edge cases on specific GPU families (A11, A12, M1)
typedef unsigned uint;

void __metal_workaround_trace_begin(uint tag) {}
void __metal_workaround_trace_end(uint tag) {}
void __metal_workaround_trace_flush(void) {}
void __metal_workaround_trace_reset(void) {}
void __metal_workaround_trace_set_buffer(void *buffer, uint size) {}
uint __metal_workaround_trace_get_write_offset(void) { return 0; }
void __metal_workaround_trace_write(uint value) {}
void __metal_workaround_trace_write64(unsigned long value) {}
