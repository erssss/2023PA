#include "common.h"

#define NAME(key) [_KEY_##key] = #key,

static const char *keyname[256]
    __attribute__((used)) = {[_KEY_NONE] = "NONE", _KEYS(NAME)};

unsigned long _uptime(); // 返回系统启动后经过的毫秒数
extern int _read_key(); // 获得按键的键盘码，无按键则返回_KEY_NONE

// 把事件写入buf中，最长len字节
size_t events_read(void *buf, size_t len) {
    bool down = false;
    int key = _read_key();
    // 获得按键位置
    if (key & 0x8000) {
        key ^= 0x8000;
        down = true;
    }
    if (key != _KEY_NONE) {
        sprintf(buf, "%s %s\n", down ? "kd" : "ku", keyname[key]);
    } else {
        sprintf(buf, "t %d\n", _uptime());
    }
    return strlen(buf); // 返回实际写入字节数
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
    strncpy(buf, dispinfo + offset, len);
}

extern void getScreen(int *width, int *height);

// 把buf中len个字节，写到屏幕偏移offset处
void fb_write(const void *buf, off_t offset, size_t len) {
    assert(offset % 4 == 0 && len % 4 == 0); // 像素以4B为单位
    int index, column, row, row2;
    int width = 0, height = 0;
    getScreen(&width, &height);

    index = offset / 4;
    column = index % width;
    row = index / width;

    index = (offset + len) / 4;
    row2 = index / width;

    /* 一行 */
    if (row2 == row) {
        _draw_rect(buf, column, row, len / 4, 1); 
        // 内容，左上角起的行列，长度，行数
        return;
    }
    int tempx = width - column;
    /* 二行 */
    if (row2 - row == 1) {
        _draw_rect(buf, column, row, tempx, 1);
        _draw_rect(buf + 4 * tempx, 0, row2, len / 4 - tempx, 1);
        return;
    }
    /* 三行及以上 */
    _draw_rect(buf, column, row, tempx, 1);
    int tmp = row2 - row - 1;
    _draw_rect(buf + 4 * tempx, 0, row + 1, width, tmp);
    _draw_rect(buf + 4 * tempx + 4 * width * tmp, 0, row2,
               len / 4 - tempx - tmp * width, 1);
}

void init_device() {
    _ioe_init();

    // TODO: print the string to array `dispinfo` with the format
    // described in the Navy-apps convention
    int width = 0, height = 0;
    getScreen(&width, &height);
    sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", width, height);
}
