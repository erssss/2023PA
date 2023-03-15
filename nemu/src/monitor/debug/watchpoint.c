#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free;

void init_wp_pool()
{
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp()
{
  if (free->next == NULL)
  {
    printf("No free wp, new_wp fail!\n");
    assert(0);
  }

  WP *new_wp = free;
  free = free->next;
  new_wp->next = head;
  head = new_wp;
  return new_wp;
}

void free_wp(WP *wp)
{
  wp->next = free;
  free = wp;
}

void print_wp(){
	if (head == NULL){
		printf("Watchpoint list is empty!\n");
		return;
	}
	printf("[watchpoints]:\n");
	printf("No.   hitTimes      expr\n");
	WP* tmp = head;
	while (tmp != NULL){
		printf("%d    %d      %s\n", tmp->NO, tmp->hitTimes, tmp->expr);
		tmp = tmp->next;
	}
}
