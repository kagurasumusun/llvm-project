// C++ standard-library end-to-end sample: iostreams + locale + chrono +
// string/vector through the WinCE libc++ configuration (pthread shim,
// WinCE NLS locale backend, coredll/mingwex C library).  The device
// harness reads the exit code and the global buffer.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <locale>
#include <map>
#include <string>
#include <vector>

static char out_buf[256];
static int out_len = 0;
static void record(char c) {
  if (out_len < 255)
    out_buf[out_len++] = c;
}

int main() {
  bool ok = true;

  // locale: the backend accepts the "C" locale and serves the classic
  // lconv (device NLS on hardware).
  try {
    std::locale loc("C");
    record('L');
    if (std::locale("").name() != std::string("C") &&
        std::locale("").name() != std::string("*"))
      record('!');
  } catch (...) {
    ok = false;
    record('x');
  }
  if (std::has_facet<std::num_put<char>>(std::locale()))
    record('F');
  else {
    ok = false;
    record('x');
  }

  // numpunct through the locale machinery exercises localeconv/printf.
  std::string s = std::to_string(1.5);
  if (s != "1.500000")
    ok = false;
  record('N');

  // iostreams over the CRT stdio layer.
  std::cout << "wince" << 42 << "\n";
  record('I');

  // chrono: system_clock (GetSystemTime) and steady_clock (QPC).
  auto now = std::chrono::system_clock::now();
  auto steady0 = std::chrono::steady_clock::now();
  if (now.time_since_epoch().count() == 0)
    record('!');
  auto steady1 = std::chrono::steady_clock::now();
  if (steady1 < steady0)
    record('!');
  record('C');

  // containers + algorithms.
  std::vector<int> v{3, 1, 2};
  std::map<std::string, int> m;
  m["wince"] = 1;
  std::sort(v.begin(), v.end());
  if (v[0] == 1 && v[2] == 2 && m["wince"] == 1)
    record('V');
  else
    ok = false;

  record(ok ? '+' : '-');
  return ok ? 0 : 1;
}
