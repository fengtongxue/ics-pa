#ifndef __MEMORY_PADDR_H__
#define __MEMORY_PADDR_H__

#include <common.h>

#define RESET_VECTOR (CONFIG_MBASE + CONFIG_PC_RESET_OFFSET)

/* convert the guest physical address in the guest program to host virtual address in NEMU */
uint8_t* guest_to_host(paddr_t paddr);
/* convert the host virtual address in NEMU to guest physical address in the guest program */
paddr_t host_to_guest(uint8_t *haddr);

static inline bool in_pmem(paddr_t addr) {
  return (addr >= CONFIG_MBASE) && (addr < (paddr_t)CONFIG_MBASE + CONFIG_MSIZE);
}

word_t paddr_read(paddr_t addr, int len);
void paddr_write(paddr_t addr, int len, word_t data);

#endif

/*
-----------
info w

Num     Type           Disp Enb Address            What
2       hw watchpoint  keep y                      d

----------
d watchpoints


-----
我们可以把监视点的检查放在trace_and_difftest()中, 
并用一个新的宏 CONFIG_WATCHPOINT把检查监视点的代码包起来;
 然后在nemu/Kconfig中为监视点添加一个开关选项, 
 最后通过menuconfig打开这个选项, 从而激活监视点的功能.
  当你不需要使用监视点时, 
  可以在menuconfig中关闭这个开关选项来提高NEMU的性能.

在同一时刻触发两个以上的监视点也是有可能的,
 你可以自由决定如何处理这些特殊情况, 我们对此不作硬性规定.
*/