#include "memory.h"
#include "proc.h"

static void *pf = NULL;

void *new_page(void) {
    assert(pf < (void *)_heap.end);
    void *p = pf;
    pf += PGSIZE;
    return p;
}

void free_page(void *p) { panic("not implement yet"); }

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
    // 检查当前指针是否已经初始化
    if (current->cur_brk != 0) {
        // 检查新的指针位置是否超过最大指针位置
        if (new_brk > current->max_brk) {
            uint32_t first = PGROUNDUP(current->max_brk);
            uint32_t end = PGROUNDDOWN(new_brk);
            // 检查新的指针位置是否与页面边界对齐
            if ((new_brk & 0xfff) == 0)
                end -= PGSIZE; // 如果对齐，则少申请一页
            // 遍历从当前指针位置到新的指针位置之间的每个页面
            for (uint32_t va = first; va <= end; va += PGSIZE) {
                void *pa = new_page(); // 申请一个新的物理页
                _map(&(current->as), (void *)va, pa); // 建立映射
            }
            // 更新最大指针位置为新的指针位置
            current->max_brk = new_brk;
        }
        // 更新当前指针位置为新的指针位置
        current->cur_brk = new_brk;
    } else { // 如果当前指针尚未初始化
             // 初始化当前指针和最大指针位置为新的指针位置
        current->cur_brk = current->max_brk = new_brk;
    }
    return 0;
}

void init_mm() {
    pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
    Log("free physical pages starting from %p", pf);

    _pte_init(new_page, free_page);
}
