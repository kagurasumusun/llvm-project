// Clean-room substitute implementation of MTLShaderLoggingRuntime.rtlib
#include <stdint.h>
#include <stdarg.h>

extern "C" {
void loweringlib_internal_log_0() __asm__("__loweringlib.internal.0"); void loweringlib_internal_log_0() {}
void loweringlib_internal_log_1() __asm__("__loweringlib.internal.1"); void loweringlib_internal_log_1() {}
void loweringlib_internal_log_2() __asm__("__loweringlib.internal.2"); void loweringlib_internal_log_2() {}
void loweringlib_internal_log_3() __asm__("__loweringlib.internal.3"); void loweringlib_internal_log_3() {}
void loweringlib_internal_log_4() __asm__("__loweringlib.internal.4"); void loweringlib_internal_log_4() {}
void loweringlib_internal_log_5() __asm__("__loweringlib.internal.5"); void loweringlib_internal_log_5() {}

void __air_impl_os_log(const char* format, ...) {
    // OS log simulation (can be redirected or left as no-op safely)
}
}
