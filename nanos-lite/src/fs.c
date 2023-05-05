#include "fs.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(void *buf, off_t offset, size_t len);
typedef struct {
    char *name;        // 文件名
    size_t size;       // 文件大小
    off_t disk_offset; // 文件在ramdisk上的偏移
    off_t open_offset; // 文件被打开之后的读写指针
} Finfo;

enum {
    FD_STDIN,
    FD_STDOUT,
    FD_STDERR,
    FD_FB,
    FD_EVENTS,
    FD_DISPINFO,
    FD_NORMAL
};

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
    // extern void get_screen(int *s_width, int *s_height);
    // int width = 0, height = 0;
    // get_screen(&width, &height);
    // file_table[FD_FB].size = width * height * sizeof(uint32_t);
    // Log("set FD_DB size = %d", file_table[FD_FB].size);
}

#define concat(x, y) x##y
#define GET_FS_POINTER(ptr)                                                    \
    size_t concat(fs_, ptr)(int fd) {                                          \
        assert(fd >= 0 && fd < NR_FILES);                                      \
        return file_table[fd].ptr;                                             \
    }

// 返回文件大小
GET_FS_POINTER(size);

#undef GET_FS_POINTER

#define GET_FS_POINTER(ptr)                                                    \
    off_t concat(fs_, ptr)(int fd) {                                          \
        assert(fd >= 0 && fd < NR_FILES);                                      \
        return file_table[fd].ptr;                                             \
    }

// 磁盘偏移
GET_FS_POINTER(disk_offset);
// 读写指针
GET_FS_POINTER(open_offset);

// size_t fs_size(int fd){
//   assert(fd>=0&&fd<NR_FILES);
//   return file_table[fd].size;
// }

// off_t fs_disk_offset(int fd){
//   assert(fd>=0&&fd<NR_FILES);
//   return file_table[fd].disk_offset;
// }

// off_t fs_open_offset(int fd){
//   assert(fd>=0&&fd<NR_FILES);
//   return file_table[fd].open_offset;
// }

// 将读写偏移指针设置为n
void set_open_offset(int fd, int n) {
    if (!(n >= 0 && fd >= 0 && fd < NR_FILES))
        Log("n = %d fd = %d NR_FILES = %d", n, fd, NR_FILES);
    assert(n >= 0 && fd >= 0 && fd < NR_FILES);
    if (n > file_table[fd].size)
        n = file_table[fd].size;
    file_table[fd].open_offset = n;
}

// 打开文件，返回文件标识符
int fs_open(const char *filename, int flags, int mode) {
    Log("fs_open");
    for (int i = 0; i < NR_FILES; ++i) {
        if (strcmp(filename, file_table[i].name) == 0) {
            file_table[i].open_offset=0;
            return i;
        }
    }
    Log("filename = %s",filename);
    panic("file not exist in file_table!");
    return -1;
}

int fs_close(int fd) {
    Log("fs_close");
    assert(fd >= 0 && fd < NR_FILES);
    return 0;
}

// 从fd文件的offset处开始，读最多len个字节到buf中，返回实际字节数
ssize_t fs_read(int fd, void *buf, size_t len) {
    Log("fs_read");
    assert(fd >= 0 && fd < NR_FILES);
    if (fd < 3) { // stdout或stderr
        return 0;
    }
    int n = fs_size(fd) - fs_open_offset(fd); // 当前文件剩余长度
    if (n > len) {
        n = len; // 实际读取的长度不能超过n
    }
    // 从文件当前的位置读len个字节到buf
    ramdisk_read(buf, fs_disk_offset(fd) + fs_open_offset(fd), n);
    // 更新偏移量
    set_open_offset(fd, fs_open_offset(fd) + n);
    return n;
}

// buf写入文件
ssize_t fs_write(int fd, void *buf, size_t len) {
    Log("fs_write");
    assert(fd >= 0 && fd < NR_FILES);
    if (fd < 3) { // 写入stdout或stderr
        return 0;
    }
    int n = fs_size(fd) - fs_open_offset(fd); // 当前文件剩余长度
    if (n > len) {
        n = len; // 实际读取的长度不能超过n
    }
    ramdisk_write(buf, fs_disk_offset(fd) + fs_open_offset(fd), n);
    // 更新偏移量
    set_open_offset(fd, fs_open_offset(fd) + n);
    return n;
}

// 根据whence不同，将读写偏移指针移动到某处
off_t fs_lseek(int fd, off_t offset, int whence) {
    Log("fs_lseek");
    switch (whence) {
    case SEEK_SET: // 开始位置+offset
        set_open_offset(fd, offset);
        return fs_open_offset(fd);
    case SEEK_CUR: // 当前位置+offset
        set_open_offset(fd, fs_open_offset(fd) + offset);
        return fs_open_offset(fd);
    case SEEK_END: // 末尾
        set_open_offset(fd, fs_size(fd) + offset);
        return fs_open_offset(fd);
    default:
        panic("Unhandled whence ID = %d", whence);
        return -1;
    }
}