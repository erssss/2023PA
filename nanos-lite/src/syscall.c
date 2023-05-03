#include "common.h"
#include "syscall.h"

//#define SYSCALL_ARG1(r) 0
//#define SYSCALL_ARG2(r) 0
//#define SYSCALL_ARG3(r) 0
//#define SYSCALL_ARG4(r) 0

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  //a[0] = SYSCALL_ARG1(r);
  a[0] = 0;

  switch (a[0]) {
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
