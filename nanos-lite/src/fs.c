#include "fs.h"

extern void ramdisk_read(void *buf,off_t offset,size_t len);
extern void ramdisk_write(void *buf,off_t offset,size_t len);
typedef struct {
  char *name;         // 文件名
  size_t size;        // 文件大小
  off_t disk_offset;  // 文件在ramdisk上的偏移
  off_t open_offset;  // 文件被打开之后的读写指针
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

#define concat(x, y) x ## y
#define GET_FS_POINTER(ptr) \
size_t concat(fs,ptr)(int fd){ \
  assert(fd>=0&&fd<NR_FILES); \
  return file_table[fd].ptr;}


GET_FS_POINTER(size);
// //返回文件大小
// size_t fs_filesz(int fd){
//   assert(fd>=0&&fd<NR_FILES);
//   return file_table[fd].size;
// }

// //磁盘偏移
// off_t disk_offset(int fd){
//   assert(fd>=0&&fd<NR_FILES);
//   return file_table[fd].disk_offset;
// }

// //读写指针
// off_t get_open_offset(int fd){
//   assert(fd>=0&&fd<NR_FILES);
//   return file_table[fd].open_offset;
// }

// //将读写偏移指针设置为n
// void set_open_offset(int fd,int n){
//   assert(fd>=0&&fd<NR_FILES);
//   assert(n>=0);
//   if(n>file_table[fd].size){
//     n=file_table[fd].size;
//   }
//   file_table[fd].open_offset=n;
// }