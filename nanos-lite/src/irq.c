#include "common.h"

extern _RegSet *schedule(_RegSet *prev);
extern _RegSet *do_syscall(_RegSet *r);
static _RegSet *do_event(_Event e, _RegSet *r) {
    switch (e.event) {
    case (_EVENT_SYSCALL):
        // printf("syscall!\n");
        do_syscall(r);
        return schedule(r);
    case (_EVENT_TRAP):
        // printf("Self-trapped!\n");
        return schedule(r);
    case _EVENT_IRQ_TIME:
        // printf("IRQ_TIME!\n");
        return schedule(r);
    default:
        panic("Unhandled event ID = %d", e.event);
    }

    return NULL;
}

void init_irq(void) { _asye_init(do_event); }
