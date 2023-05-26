#include "common.h"

extern _RegSet* do_syscall(_RegSet *r);
static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
  	case(_EVENT_SYSCALL):
  	  return do_syscall(r);
    case (_EVENT_TRAP):
      printf("Self-trapped!\n");
      return do_syscall(r);
    default: panic("Unhandl88888ed event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
