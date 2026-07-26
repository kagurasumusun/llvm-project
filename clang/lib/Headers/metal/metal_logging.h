// metal_logging — MSL logging (cleanroom)
#ifndef _METAL_LOGGING_H_
#define _METAL_LOGGING_H_
#include <metal/metal_common>
namespace metal {
struct os_log_t {};
METAL_FUNC os_log_t get_default_log() { return {}; }
METAL_FUNC void os_log(os_log_t log, const char *fmt) {}
METAL_FUNC void os_log_with_args(os_log_t log, const char *fmt, void *args) {}
METAL_FUNC bool os_log_type_enabled(os_log_t log, int type) { return false; }
METAL_FUNC void os_log_fault(os_log_t log, const char *fmt) {}
METAL_FUNC void os_log_error(os_log_t log, const char *fmt) {}
METAL_FUNC void os_log_info(os_log_t log, const char *fmt) {}
METAL_FUNC void os_log_debug(os_log_t log, const char *fmt) {}
METAL_FUNC int os_log_get_type(os_log_t log) { return 0; }
} // namespace metal
#endif