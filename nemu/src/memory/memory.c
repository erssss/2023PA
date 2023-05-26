#include "device/mmio.h"
#include "memory/mmu.h"
#include "nemu.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define PDXSIZE 22
#define PTXSIZE 12
#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xfff) // 获取地址高20位，即基址
#define PDX(va)                                                                \
    (((uint32_t)(va) >> PDXSIZE) & 0x3ff) // 查找页目录，获取二级页表
#define PTX(va) (((uint32_t)(va) >> PTXSIZE) & 0x3ff) // 获取页
#define OFF(va) ((uint32_t)(va)&0xfff)                // 获取页内偏移

#define pmem_rw(addr, type)                                                    \
    *(type *)({                                                                \
        Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound",   \
               addr);                                                          \
        guest_to_host(addr);                                                   \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
    int map_id = is_mmio(addr);
    // 好吧!不让用auto
    // auto map_id = is_mmio(addr);
    if (map_id != -1)
        return (uint32_t)mmio_read(addr, len, map_id);

    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
    int map_id = is_mmio(addr);
    // auto map_id = is_mmio(addr);
    if (map_id != -1)
        mmio_write(addr, len, data, map_id);
    else
        memcpy(guest_to_host(addr), &data, len);
}

// uint32_t vaddr_read(vaddr_t addr, int len) {
//   return paddr_read(addr, len);
// }

// void vaddr_write(vaddr_t addr, int len, uint32_t data) {
//   paddr_write(addr, len, data);
// }

// 根据虚拟地址addr解析出物理地址
//  paddr_t page_translate(vaddr_t addr, bool w1r0) {
//      //aka page_walk
//      PDE pde, *pgdir;
//      PTE pte, *pgtab;
//      //
//      只有进入保护模式并开启分页机制后才会进行页级地址转换。。。。。。。。。。
//      if (cpu.cr0.protect_enable && cpu.cr0.paging) {
//  	    pgdir = (PDE *)(PTE_ADDR(cpu.cr3.val));
//  //cr3存放20位的基址作为页目录入口 	    pde.val =
//  paddr_read((paddr_t)&pgdir[PDX(addr)], 4); 	    assert(pde.present);
//  pde.accessed = 1;

// 	    pgtab = (PTE *)(PTE_ADDR(pde.val));
// //页目录存放20位的基址作为页表入口 	    pte.val =
// paddr_read((paddr_t)&pgtab[PTX(addr)], 4); 	    assert(pte.present);
// pte.accessed = 1; 	    pte.dirty = w1r0 ? 1 : pte.dirty; //写则置脏位

// 	    //pte高20位和线性地址低12位拼接成真实地址
// 	    return PTE_ADDR(pte.val) | OFF(addr);
// 	}

//     return addr;
// }

// 虚实地址转换
paddr_t page_translate(vaddr_t addr, bool write) {
    CR0 cr0 = (CR0)cpu.CR0;
    // 进入保护模式并开启分页机制后才会进行页级地址转换
    if (cr0.paging && cr0.protect_enable) {
        CR3 cr3 = (CR3)cpu.CR3;
        PDE *pgdirs = (PDE *)PTE_ADDR(cr3.val); // 页目录表基址
        PDE pde = (PDE)paddr_read((uint32_t)(pgdirs + PDX(addr)),
                                  4); // 在页目录中查找页目录项
        PTE *ptab = (PTE *)PTE_ADDR(pde.val); // 页表基址
        PTE pte = (PTE)paddr_read((uint32_t)(ptab + PTX(addr)),
                                  4); // 在页表中查找页表项
        pde.accessed = 1;
        pte.accessed = 1;
        if (write)
            pte.dirty = 1;

        paddr_t paddr = PTE_ADDR(pte.val) | OFF(addr); // 物理地址
        return paddr;
    }
    return addr;
}

// 根据虚拟地址读取len个字节的内存并返回
uint32_t vaddr_read(vaddr_t addr, int len) {
    // 访存跨页
    // 要读取内容的首部和尾部对应的页目录基址不同
    if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1)) {
        int len1 = 0x1000 - OFF(addr); // 首页读取字节数
        int len2 = len - len1;         // 尾页读取字节数
        paddr_t paddr1 = page_translate(addr, true);
        paddr_t paddr2 = page_translate(addr + len1, true);

        uint32_t low = paddr_read(paddr1, len1);
        uint32_t high = paddr_read(paddr2, len2);

        return high << (8 * len1) | low; // 拼接数据
    } else {
        paddr_t paddr = page_translate(addr, false);
        return paddr_read(paddr, len);
    }
}

// 将len字节的数据写到内存虚址addr处
void vaddr_write(vaddr_t addr, int len, uint32_t data) {
    // 访存跨页
    if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1)) {
        int len1 = 0x1000 - OFF(addr); // 首页读取字节数
        int len2 = len - len1;         // 尾页读取字节数
        paddr_t paddr1 = page_translate(addr, false);
        paddr_t paddr2 = page_translate(addr + len1, false);

        // 拆解数据为两份并分别写入相应地址
        uint32_t low = data & (~0u >> ((4 - len1) << 3));
        uint32_t high = data >> ((4 - len2) * 8);
        paddr_write(paddr1, len1, low);
        paddr_write(paddr2, len2, high);

        return;
    } else {
        paddr_t paddr = page_translate(addr, true);
        paddr_write(paddr, len, data);
    }
}