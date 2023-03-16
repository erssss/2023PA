#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
	char expr[32];
	int hitTimes;
  int oldValue;

} WP;

void print_wp();
WP *new_wp();
// void free_wp(WP *wp);
int free_wp(int no);

#endif
