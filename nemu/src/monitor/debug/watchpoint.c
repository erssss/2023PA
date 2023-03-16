#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp() {
    if (free_->next == NULL) {
        printf("No free_ wp, new_wp fail!\n");
        assert(0);
    }

    WP *new_wp = free_;
    free_ = free_->next;
    new_wp->next = NULL;
    new_wp->next = head;
    head = new_wp;
    return new_wp;
}

void free_wp(WP *wp) {
    wp->NO = 0;
    wp->next = free_;
    free_ = wp;
}

bool delete_wp(int no) {
    WP *tmp = head->next;
    WP *cur = head;

    while (tmp && tmp->NO < no) {
        tmp = tmp->next;
        cur = cur->next;
    }

    if (tmp->NO == no) {
        cur->next = tmp->next;
        free_wp(tmp);
    } else {
      printf("Not found watchpoint number %d\n",no);
      return 1;
    }
    // for (int i = 0; i < no - 1; i++) {
    //     if (tmp == NULL) {
    //         printf("free_wp no. out of list!\n");
    //         return -1;
    //     }
    //     tmp = tmp->next;
    // }
    // tmp->next = free_;
    // free_ = tmp;

    return 0;
}

void print_wp() {
    if (head == NULL) {
        printf("Watchpoint list is empty!\n");
        return;
    }
    printf("[watchpoints]:\n");
    printf("No.\t\thitTimes\t\texpr\n");
    WP *tmp = head;
    while (tmp != NULL) {
        printf("%d\t\t%d\t\t%s\n", tmp->NO, tmp->hitTimes, tmp->expr);
        tmp = tmp->next;
    }
}
