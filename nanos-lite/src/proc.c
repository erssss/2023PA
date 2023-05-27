#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;
int current_game = 0;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
    int i = nr_proc++;
    _protect(&pcb[i].as);

    uintptr_t entry = loader(&pcb[i].as, filename);

    // TODO: remove the following three lines after you have implemented
    // _umake() _switch(&pcb[i].as); current = &pcb[i];
    // ((void (*)(void))entry)();

    _Area stack;
    stack.start = pcb[i].stack;
    stack.end = stack.start + sizeof(pcb[i].stack);

    pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

// _RegSet *schedule(_RegSet *prev) {
//     if (current != NULL) {  // 如果当前进程的PCB指针不为空
//         current->tf = prev; // 保存tf
//     }
//     // 切换进程
//     current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
//     // current = &pcb[0];     // 切换到第一个用户进程
//     _switch(&current->as); // 切换虚拟地址空间
//     return current->tf;
// }

#define FREQUENCY 1000
_RegSet *schedule(_RegSet *prev) {
    // printf("schedule");
    if (current != NULL)    // 如果当前进程的PCB指针不为空
        current->tf = prev; // 保存tf
    else
        current = &pcb[current_game];
    static int count = 0;
    if (current == &pcb[current_game]) // 计时
        count++;
    else
        current = &pcb[current_game]; // 切换进程
    if (count >= FREQUENCY) {     // 超时
        current = &pcb[1];
        count = 0;
    }
    _switch(&current->as); // 切换虚拟地址空间
    return current->tf;
}

void change_proc() {
  current_game = 2-current_game;
  Log("current_game = %d",current_game);
}