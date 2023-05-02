#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)
#define RAMDISK_SIZE ((&ramdisk_end)-(&ramdisk_start))
extern uint8_t ramdisk_start,ramdisk_end;
extern void ramdisk_read(void *buf, off_t offset, size_t len);

// 将 ramdisk 中从 0 开始的所有内容放置在 0x4000000
// 并把这个地址作为程序的入口返回
uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  ramdisk_read(DEFAULT_ENTRY,0,RAMDISK_SIZE);
  return (uintptr_t)DEFAULT_ENTRY;
}
