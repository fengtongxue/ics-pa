#include <isa.h>
#include <memory/paddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ, 
  TK_INTDEC = 258,  
  TK_PLUS = 259, 
  TK_Multi = 260, // 260
  TK_LeftBracket = 261, 
  TK_RightBracket = 262,

  TK_REGS = 263,     // register
  TK_DEREF,          // * 解引用

  TK_HEX,            // hex value
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\b[0-9]+\\b", TK_INTDEC},  // dec int
  {"0x[0-9]+", TK_HEX},
  {" ", TK_NOTYPE},             // space
  {"\\+", TK_PLUS},             // plus
  {"==", TK_EQ},                // equal

  {"\\$\\w+", TK_REGS},

  {"\\(", TK_LeftBracket},      // (
  {"\\)", TK_RightBracket},     // )
  {"\\*", TK_Multi},            // *
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
  word_t token_value;
  char str[32];
} Token;

Token new_token() {
  Token t = {0, 0, {0}};
  return t;
}

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

void
print_token(Token t) {
  switch (t.type)
  {
  case TK_PLUS:
    printf("type: '+'\n");
    break;
  case TK_Multi:
    printf("type: '*'\n");
    break;
  case TK_LeftBracket:
    printf("type: '('\n");
    break;
  case TK_RightBracket:
    printf("type: ')'\n");
    break;
  case TK_INTDEC:
    printf("type: '数字'");
    printf("value: '%d'\n", t.token_value);
  case TK_HEX:
    printf("type: '16进制数字'");
    printf("value: 0x'%s', 10进制 : %u\n", t.str, t.token_value);
    break;
  case TK_DEREF:
    printf("type: '解引用'\n");
    break;
  case TK_REGS:
    printf("type: 寄存器");
    printf("value: '%s'\n", t.str);
    break;
  default:
    panic("Print Token unkonwn type: %d, str: %s\n", t.type, t.str);
  }
}

void 
print_tokens(int start, int end) {
  int p = start;
  int q = end;

  printf("token list: start: %d, end: %d\n", p, q);
  printf("=============\n");
  for (size_t i = p; i < q+1; i++)
  {
    print_token(tokens[i]);
  }
  printf("=============\n");

}

static bool make_token(char *e) {
  int position = 0;
  
  int i;
  regmatch_t pmatch;
  nr_token = 0;

  while (e[position] != '\0') {
    Token t = new_token();

    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;


        // todo 超出长度暂时不用
        assert(substr_len <= 32);
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_INTDEC:
            t.type = TK_INTDEC;
            char *s = malloc(10);
            strncpy(s, substr_start, (size_t) substr_len);
            t.token_value = strtol(s, NULL, 10);
            tokens[nr_token++] = t;
            break;

          case TK_HEX:
            t.type = TK_HEX;
            char *s2 = malloc(20);
            strncpy(s2, substr_start, (size_t) substr_len);
            strcpy(t.str, s2);
            t.token_value =  strtol(s2, NULL, 16);
            tokens[nr_token++] = t;
            break;
          
          case TK_PLUS:
            t.type = TK_PLUS;
            tokens[nr_token++] = t;
            break;
          case TK_Multi:
            t.type = TK_Multi;
            tokens[nr_token++] = t;
            break;
          
          case TK_REGS:
            t.type = TK_REGS;
            strncpy(t.str, substr_start+1, (size_t) substr_len - 1);
            tokens[nr_token++] = t;
            break;

          case TK_EQ:
            t.type = TK_EQ;
            tokens[nr_token++] = t;
            break;
          case TK_LeftBracket:
            t.type = TK_LeftBracket;
            tokens[nr_token++] = t;
            break;
          case TK_RightBracket:
            t.type = TK_RightBracket;
            tokens[nr_token++] = t;
            break;
          default:
            print_token(tokens[nr_token]);
            panic("make token unknown type\n");
        }
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  nr_token --;

  return true;
}


// p q 同时是 （）, 并且 整个在一个括号里面
bool check_parentheses(int p, int q) {
  int bracket_count = 0;
  if ((tokens[p].type == TK_LeftBracket) && (tokens[q].type == TK_RightBracket)) 
  {
    for (size_t i = p+1; i < q; i++)
    {
      if (tokens[i].type == TK_LeftBracket) {
        bracket_count ++;
      } else if (tokens[i].type == TK_RightBracket){
        bracket_count --;
      } else {
        continue;
      }
    }
    if (bracket_count == 0) {
      printf("find bracket\n");
      return true; 
    } else {
        return false;
    }
  } else {
    return false;
  }
}

bool is_op(Token t) {
  // todo 先做一个加号
  if (t.type == TK_PLUS || t.type == TK_Multi) {
    return true;
  } else {
    return false;
  }
}

// 返回 token 的优先级， * / 为 2， + - 为 1；
int priority(Token t) {
  if (t.type == TK_Multi) {
    return 2;
  } else if (t.type == TK_PLUS) {
    return 1;
  } else {
    assert(0);
    return -1;
  }
}

// 拿到 子 token list main op
int main_op(int p, int q) {
  //
  int op_index = -1;
  bool in_bracket = false;
  for (size_t i = p; i < q + 1; i++)
  {
    if (in_bracket) continue;
    Token t = tokens[i];
    if (t.type == TK_LeftBracket) {
      in_bracket = true;
    }
    else if (t.type == TK_RightBracket) {
      in_bracket = false;
    }
    else if (is_op(t)) {
      // 第一个运算符
      if (op_index == -1) {
        op_index = i;
      // 不是第一个, 但是优先级比你低
      } else if ((priority(t) <= priority(tokens[op_index]))){
        op_index = i;
      }
      // print_token(tokens[op_index]);
    }
  }
  return op_index;
}


// token list start, end
// eval 怎么处理 * token

int eval(int p, int q) {  
  print_tokens(p, q);
  if (p > q) {
    printf("eval token list start > end: %d, %d\n", p, q);
    assert(0);
  }
  // eval one token
  else if (p == q) {
    word_t tv = 0;
    if (tokens[p].type == TK_REGS) {
      bool success = true;
      word_t v = isa_reg_str2val(tokens[p].str, &success);
      if (success) {
        printf("token reg is : %s, vlaue: %d\n", tokens[p].str, v); 
      } else {
        printf("call p reg error");
      }
    } else {
      tv = tokens[p].token_value;
    }
    return tv;
  } 
  // * 1232
  else if (q - p == 1) {
    word_t tv = 0;
    if (tokens[p].type == TK_DEREF) {
      word_t addr = tokens[q].token_value;
      tv = paddr_read(addr, 1);
      printf("addr: %u, value: %d\n", addr, tv);
    }
    return tv;
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }
  else {
    // find main 运算符
    int o = main_op(p, q);
    if (o == -1) {
      assert(0);
    }
    printf("opindex:  %d \n", o);

    int val1 = eval(p,  o - 1);
    int val2 = eval(o + 1, q);

    switch (tokens[o].type) {
      // +
      case TK_PLUS: 
        printf("val1 + val2: %d, %d\n", val1, val2);
        return val1 + val2;
    //   case '-': /* ... */
      // *
      case TK_Multi: 
        printf("val1 * val2: %d, %d\n", val1, val2);
        return val1 * val2;

      
    //   case '/': /* ... */
      default: assert(0);
    }
    return 0;
  }
}

bool certain_type(int type) {
  bool v = false;
  if (type == TK_PLUS || type == TK_PLUS) {
    v = true;
  }
  return v;
}

word_t expr(char *e, bool *success) {
  printf("call expr\n");

  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  // format 解引用 token
  // *a
  // a + *b
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == TK_Multi && (i == 0 || certain_type(tokens[i - 1].type))) {
      tokens[i].type = TK_DEREF;
    }
  }
  
  int v = eval(0, nr_token);
    *success = true;
  return v;
}

// 写测试
