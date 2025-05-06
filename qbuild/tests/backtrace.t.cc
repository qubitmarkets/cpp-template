#include "qbuild/CaptureBacktrace.h"
#include "qbuild/SigHandler.h"
#include "stdio.h"
#include <execinfo.h>

void unhandled_exception_handler();

void foo2() {
  while (true) {
  };
}
void foo() { foo2(); }

int main(int argc, char **argv) {
  CaptureBacktrace::init(argv[0]);
  SigHandler::install();

  foo();

  printf("Done\n");
  return 0;
}
