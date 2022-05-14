#include "sdb.h"
#include <memory/paddr.h>

// todo 暂时直接读物理地址
word_t
examine_addr(int words, word_t addr) {
    word_t value = 0;
    for (size_t i = 0; i < words; i+=4)
    {
        value = paddr_read(addr + i, 4);
        printf("0x%x  ", value);
    }
    printf("\n");
    return 0;
}
