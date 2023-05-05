#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)
#define RAMDISK_SIZE ((&ramdisk_end)-(&ramdisk_start))
extern uint8_t ramdisk_start,ramdisk_end;
extern void ramdisk_read(void *buf, off_t offset, size_t len);

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd,void *buf,size_t len);
extern int fs_close(int fd);
extern size_t fs_offset(int fd);
extern size_t fs_lseek(int fd,size_t offset,int whence);
extern size_t fs_disk_offset(int fd);
extern size_t fs_size(int fd);

// 将 ramdisk 中从 0 开始的所有内容放置在 0x4000000
// 并把这个地址作为程序的入口返回
uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  // ramdisk_read(DEFAULT_ENTRY,0,RAMDISK_SIZE);

  /* PA3.2 */
  int fd=fs_open(filename,0,0);
  Log("filename=%s,fd=%d",filename,fd);
  fs_read(fd,DEFAULT_ENTRY,fs_size(fd));//把文件整个读入内存DEFAULT_ENTRY处
  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
