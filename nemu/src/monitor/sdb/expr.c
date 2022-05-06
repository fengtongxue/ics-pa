#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_PLUS, TK_INTDEC, 
  

  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.

十进制整数
+, -, *, /
(, )
空格串(一个或多个空格)

   */

  {"\\b[0-9]+\\b", TK_INTDEC},  // dec int
  
  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;
/*
todo:
  会匹配多个结果， 重复生成 token

*/
static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int token_index = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  Token t;

  // 匹配到了， 开始 算了不写了
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        // 为什么不能在 switch 里面声明变量啊        
        // todo 超出长度暂时不用
        assert(substr_len <= 32);
        
        printf("rules[i].token_type: %d\n", rules[i].token_type);
        switch (rules[i].token_type) {
          case TK_INTDEC:
            t.type = TK_INTDEC;
            strncpy(t.str, e, (size_t) substr_len);
            tokens[token_index++] = t;
            break;
          case TK_NOTYPE:
            t.type = TK_PLUS;
            tokens[token_index++] = t;
            break;
          case TK_EQ:
            t.type = TK_EQ;
            tokens[token_index++] = t;            
            break;
        }
        printf("toekns len: %d,  \n", token_index);


        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  printf("call expr");

  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  return 0;
}
