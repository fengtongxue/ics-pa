#include <isa.h>
#include "local-include/reg.h"

#define REG_LEN 32


/*
riscv 32   the x registers are 32 bits wide


$0  always 0
ra  return address

*/
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

// Print the names and values of all registers except floating-point registers
void isa_reg_display() {
  // Todo fake to test
  cpu.gpr[1]._32 = 30;

  for (size_t i = 0; i < REG_LEN; i++)
  {
    printf("%*s   ", 10, regs[i]);
    printf("%*d   \n", 15, cpu.gpr[i]._32);
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  bool find = false;
  int index = 0;
  for (size_t i = 0; i < REG_LEN; i++)
  {
    if (strcmp(s, regs[i]) == 0) {
      find = true;
      index = i;
      break;
    }
  }
  if (find) {
    return cpu.gpr[index]._32;
  } else {
    *success = false;
  }

  return 0;
}
