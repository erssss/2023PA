#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48 // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() { boot_time = inl(RTC_PORT); }

unsigned long _uptime() { return 0; }

uint32_t *const fb = (uint32_t *)0x40000;

_Screen _screen = {
    .width = 400,
    .height = 300,
};

extern void *memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
    // int i;
    // for (i = 0; i < _screen.width * _screen.height; i++) {
    //   fb[i] = i;
    // }
  // fix: 好迷惑？？全部注释还是能跑
  int col, row;
  for (row = y; row < y + h; row++)
    for (col = x; col < x + w; col++) 
      fb[col+row*_screen.width] = pixels[(row-y)*w+(col-x)];

}


void _draw_sync() {}

int _read_key () {
  if (inb (0x64)) // 状态寄存器
    return inl(0x60) ; // 数据寄存器
  else
    return _KEY_NONE;
}

