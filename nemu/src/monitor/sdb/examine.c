#include "sdb.h"
#include <memory/paddr.h>

// todo 暂时直接读物理地址
int
examine_addr(int count, word_t addr) {
    for (size_t i = 0; i < count; i+=4)
    {
        word_t value = paddr_read(addr + i, 4);
        printf("0x%x  ", value);
    }
    printf("\n");
    
    return 0;
}
