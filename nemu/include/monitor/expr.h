#ifndef __EXPR_H__
#define __EXPR_H__

#include "common.h"

uint32_t expr(char *q, bool *success);
uint32_t hex_to_dec(char str[32]);
uint32_t eval(int p, int q);
int dominant_op(int p, int q);
bool paren_match(int left, int right);
int get_priority(int type, int layer);

#endif
