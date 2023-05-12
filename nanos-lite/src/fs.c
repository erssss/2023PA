#include "fs.h"

extern size_t events_read(void *buf, size_t len);
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
    extern void getScreen(int *width, int *height);
    int width = 0, height = 0;
    getScreen(&width, &height);
    // FD_FB是显存的文件描述符
    file_table[FD_FB].size = width * height * sizeof(uint32_t); // 每个像素4B
    Log("FD_FB size=%d", file_table[FD_FB].size);
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
    off_t concat(fs_, ptr)(int fd) {                                           \
        assert(fd >= 0 && fd < NR_FILES);                                      \
        return file_table[fd].ptr;                                             \
    }

// 磁盘偏移
GET_FS_POINTER(disk_offset);
// 读写指针
GET_FS_POINTER(open_offset);

// size_t fs_size(int fd) {
//     assert(fd >= 0 && fd < NR_FILES);
//     return file_table[fd].size;
// }

// off_t fs_disk_offset(int fd) {
//     assert(fd >= 0 && fd < NR_FILES);
//     return file_table[fd].fs_disk_offset;
// }

// off_t fs_open_offset(int fd) {
//     assert(fd >= 0 && fd < NR_FILES);
//     return file_table[fd].open_offset;
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
    Log("fs_open: NR_FILES = %d", NR_FILES);
    // Log("filename = %s",filename);
    for (int i = 0; i < NR_FILES; ++i) {
        // Log("file_table[i].name = %s",file_table[i].name);
        if (strcmp(filename, file_table[i].name) == 0) {
            file_table[i].open_offset = 0;
            return i;
        }
    }
    panic("file not exist in file_table!");
    return -1;
}

int fs_close(int fd) {
    Log("fs_close");
    assert(fd >= 0 && fd < NR_FILES);
    return 0;
}

extern void dispinfo_read(void *buf, off_t offset, size_t len);
ssize_t fs_read(int fd, void *buf, size_t len) {
    assert(fd >= 0 && fd < NR_FILES);
    if (fd < 3 || fd == FD_FB) { // 不可读取fb
        Log("error:fd<3||fd==FD_DISPINFO");
        return 0;
    }
    int n = fs_size(fd) - fs_open_offset(fd); // 当前文件剩余长度
    if (n > len) {
        n = len; // 实际读取的长度不能超过n
    }
    if (fd == FD_DISPINFO)
        dispinfo_read(buf, fs_open_offset(fd), n);
    else if (fd == FD_EVENTS)
        return events_read(buf, len);
    else
        // 从文件当前的位置读len个字节到buf
        ramdisk_read(buf, fs_disk_offset(fd) + fs_open_offset(fd), n);
    set_open_offset(fd, fs_open_offset(fd) + n); // 更新偏移
    return n;
}

extern void fb_write(const void *buf, off_t offset, size_t len);
ssize_t fs_write(int fd, void *buf, size_t len) {
    assert(fd >= 0 && fd < NR_FILES);
    if (fd < 3 || fd == FD_DISPINFO) { // 不可写入dispinfo
        Log("error:fd<3||fd==FD_DISPINFO");
        return 0;
    }
    int n = fs_size(fd) - fs_open_offset(fd); // 当前文件剩余长度
    if (n > len) {
        n = len; // 实际读取的长度不能超过n
    }
    if (fd == FD_FB) {
        fb_write(buf, fs_open_offset(fd), n);
    } else
        ramdisk_write(buf, fs_disk_offset(fd) + fs_open_offset(fd), n);
    set_open_offset(fd, fs_open_offset(fd) + n); // 更新【偏移
    return n;
}

// 移动读写指针
off_t fs_lseek(int fd, off_t offset, int whence) {
    // Log("fs_lseek");
    switch (whence) {
    case SEEK_CUR: // 根据当前的光标，以 offset 为偏移量设置光标
        set_open_offset(fd, fs_open_offset(fd) + offset);
        break;
    case SEEK_SET: // 根据当前的 offset 设置光标
        set_open_offset(fd, offset);
        break;
    case SEEK_END: // 根据当前的光标，以 offset 为偏移量设置光标
        set_open_offset(fd, fs_size(fd) + offset);
        break;
    default:
        panic("Unhandled whence ID = %d", whence);
        return -1;
    }
    return fs_open_offset(fd);
}