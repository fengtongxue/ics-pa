#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"#include <stdint.h>\n"
"int main() { "
"  uint32_t result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

uint32_t  
choose(uint32_t  n) {
  int r = rand() % n;
  return (uint32_t) r;
}

int tail;

void
gen_num() {
  sprintf(bu, "%u", choose(UINT16_MAX));
  
}

void
gen(char c) {
  buf[tail++] = c;
}

void
gen_str(char* c) {
  sprintf(buf1, "%s", c);
  strcat(buf, buf1);
}

void 
gen_rand_op() {
  if (choose(10) > 5) {
     sprintf(buf1, "%c", '+');
     strcat(buf, buf1);
  } else {
    sprintf(buf1, "%c", '*');
    strcat(buf, buf1);
  }
}


static void gen_rand_expr() {
    if (tail > 60000) {
      return;
    } else {
      switch (choose(3)) {
      case 0: gen_num(); break;
      // case 1: gen_str("(uint32_t) "); gen('('); gen_rand_expr(); gen(')'); break;
      case 1: gen('('); gen_rand_expr(); gen(')'); break;
      default:  gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
    }
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    tail = 0;
    gen_rand_expr();
    gen('\0');
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -w /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint32_t result;
    int r = fscanf(fp, "%d", &result);
    assert(r != 0);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
