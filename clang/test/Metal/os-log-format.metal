// The metal_os_log format family.
//
// Apple's <metal_types> defines
//   #define METAL_OS_LOG_FORMAT(F) __attribute__((format(metal_os_log, F, F + 1)))
// used by every os_log method in <metal_logging>. The name must not trigger
// "format attribute not supported", and the family shares the os_log
// printf-style checking rules (GetFormatStringType).
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.0 -Wformat -fsyntax-only -verify %s

// os_log methods live inside metal::os_log and are variadic; a plain
// (non-entry-point) variadic function is fine in Metal.
struct logger {
  __attribute__((format(metal_os_log, 2, 3))) void log(const char *fmt, ...);
};

void use(logger &l) {
  l.log("%d", 42);   // no diagnostic
  l.log("%s", "ok"); // no diagnostic
  // expected-warning@+1 {{format specifies type 'int' but the argument has type 'double'}}
  l.log("%d", 1.5);
  // expected-warning@+1 {{format specifies type 'char *' but the argument has type 'int'}}
  l.log("%s", 7);
}
