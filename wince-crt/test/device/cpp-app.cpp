// C++ end-to-end sample: global constructors/destructors, exceptions,
// RTTI, virtual dispatch.  Deliberately free of iostream/locale (the
// WinCE coredll does not provide them); output is captured through a
// global buffer the device-test harness reads after the run.
struct Buffer {
  char buf[256];
  int len;
  Buffer() : len(0) { append('['); }
  ~Buffer() { append(']'); }
  void append(char c) { if (len < 255) buf[len++] = c; }
};

static Buffer out;

struct Base { virtual ~Base() {} virtual const char *name() { return "B"; } };
struct Derived : Base { const char *name() override { return "D"; } };

static int thrown = 0;

int run() {
  Derived d;
  Base *p = &d;

  out.append(p->name()[0]);          // virtual dispatch -> 'D'

  try {
    throw 42;
  } catch (int v) {                  // exception + landing pad
    thrown = v;
    out.append('E');
  }

  // RTTI
  void *rt = &typeid(d);
  out.append(rt ? 'T' : 'x');

  return (thrown == 42 && rt) ? 0 : 1;
}

struct Global {
  int value;
  Global() : value(7) { out.append('C'); }
  ~Global() { out.append('X'); }
};

static Global g;

int main() {
  int r = run();
  return r;
}
