/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

// Just declare before use
word_t vaddr_read(vaddr_t addr, int len);
word_t isa_reg_str2val(const char *s, bool *success);


const char *regsters_on_expr[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

enum {
  TK_NOTYPE = 256,

  // Double
  TK_DOUBLE_BEGIN,
  TK_EQ,
  TK_NE,
  TK_LE,
  TK_ME,
  TK_LT,
  TK_MT,
  TK_LAND,
  TK_LOR,
  TK_DOUBLE_END,

  // Single
  TK_SINGLE_BEGIN,
  TK_NEG,   // Negative
  TK_SOLV,  // Decoding
  TK_NOT,   // Logic not
  TK_SINGLE_END,

  //----------KEEP-OPERATIONS-TOGETHER----------

  // Paren
  TK_LP,
  TK_RP,

  // Number
  TK_HEX,
  TK_DEC,

  // Register
  TK_REG_BEGIN,
  TK_$0, TK_ra, TK_sp, TK_gp, TK_tp, TK_t0, TK_t1, TK_t2,
  TK_s0, TK_s1, TK_a0, TK_a1, TK_a2, TK_a3, TK_a4, TK_a5,
  TK_a6, TK_a7, TK_s2, TK_s3, TK_s4, TK_s5, TK_s6, TK_s7,
  TK_s8, TK_s9, TK_s10, TK_s11, TK_t3, TK_t4, TK_t5, TK_t6,
  TK_REG_END,

  /* TODO: Add more token types */

};


static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces

  // Double operations
  {"\\+", '+'},         // plus
  {"-", '-'},           // minus
  {"\\*", '*'},         // nulti
  {"/", '/'},           // devide
  {"==", TK_EQ},        // equal
  {"!=", TK_NE},        // not equal
  {"<=", TK_LE},        // less equal
  {">=", TK_ME},        // more equal
  {"<", TK_LT},         // less than
  {">", TK_MT},         // more than
  {"&&", TK_LAND},      // logic and
  {"\\|\\|", TK_LOR},     // logic or

  // Single operations
  {"!", TK_NOT},          // not

  // Parens
  {"\\(", TK_LP},         // left paren
  {"\\)", TK_RP},         // right paren

  // Numbers
  {"0[xX][0-9a-fA-F]+", TK_HEX}, // hex number
  {"[0-9]+", TK_DEC},   // dec number

  // Registers
  {"\\$0", TK_$0}, {"\\$ra", TK_ra}, {"\\$sp", TK_sp}, {"\\$gp", TK_gp},
  {"\\$tp", TK_tp}, {"\\$t0", TK_t0}, {"\\$t1", TK_t1}, {"\\$t2", TK_t2},
  {"\\$s0", TK_s0}, {"\\$s1", TK_s1}, {"\\$a0", TK_a0}, {"\\$a1", TK_a1},
  {"\\$a2", TK_a2}, {"\\$a3", TK_a3}, {"\\$a4", TK_a4}, {"\\$a5", TK_a5},
  {"\\$a6", TK_a6}, {"\\$a7", TK_a7}, {"\\$s2", TK_s2}, {"\\$s3", TK_s3},
  {"\\$s4", TK_s4}, {"\\$s5", TK_s5}, {"\\$s6", TK_s6}, {"\\$s7", TK_s7},
  {"\\$s8", TK_s8}, {"\\$s9", TK_s9}, {"\\$s10", TK_s10}, {"\\$s11", TK_s11},
  {"\\$t3", TK_t3}, {"\\$t4", TK_t4}, {"\\$t5", TK_t5}, {"\\$t6", TK_t6},
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

// max token number is 64
static Token tokens[64] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_EQ:
          case TK_LE:
          case TK_ME:
          case TK_LT:
          case TK_MT:
          case TK_LP:
          case TK_RP:
          case TK_NOT:
          case '+':
          case '-':
          case '*':
          case '/':
          case TK_$0: case TK_ra: case TK_sp: case TK_gp: case TK_tp: case TK_t0: case TK_t1: case TK_t2:
          case TK_s0: case TK_s1: case TK_a0: case TK_a1: case TK_a2: case TK_a3: case TK_a4: case TK_a5:
          case TK_a6: case TK_a7: case TK_s2: case TK_s3: case TK_s4: case TK_s5: case TK_s6: case TK_s7:
          case TK_s8: case TK_s9: case TK_s10: case TK_s11: case TK_t3: case TK_t4: case TK_t5: case TK_t6:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_HEX:
            tokens[nr_token].type = rules[i].token_type;
            if (substr_len <= 31) {
              int i;
              for (i = 0; i < substr_len; i++) {
                tokens[nr_token].str[i] = e[position - substr_len + i];
              }
              tokens[nr_token].str[i] = '\0';
            }
            else {
              printf("Token string is loger than 32!\n");
            }
            nr_token++;
            break;
          case TK_DEC:
            tokens[nr_token].type = rules[i].token_type;
            if (substr_len <= 31) {
              int i;
              for (i = 0; i < substr_len; i++) {
                tokens[nr_token].str[i] = e[position - substr_len + i];
              }
              tokens[nr_token].str[i] = '\0';
            }
            else {
              printf("Token string is loger than 32!\n");
            }
            nr_token++;
            break;
          default:
            Log("Wrong token type!");
            assert(0);
        }

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

int decode_addr(uint32_t addr) {
  uint32_t value = 0;
  value = vaddr_read(addr, 4);
  // printf("%d\n", vaddr_read(exp_value, 4));
  return value;
}

bool trans_sing() {
  // include * and -, which should be translated
  bool find = false;
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*') {
      if ((i == 0)
      || (tokens[i - 1].type >= TK_DOUBLE_BEGIN && tokens[i - 1].type <= TK_DOUBLE_END)
      || (tokens[i - 1].type >= TK_SINGLE_BEGIN && tokens[i - 1].type <= TK_SINGLE_END)
      || (tokens[i - 1].type == '+' || tokens[i - 1].type == '-')
      || (tokens[i - 1].type == '*' || tokens[i - 1].type == '/')
      || (tokens[i - 1].type == TK_LP))
      {
        tokens[i].type = TK_SOLV;
        printf("Change position %d to TK_SOLV\n", i);
        find = true;
      }
    } else if (tokens[i].type == '-') {
      if ((i == 0)
      || (tokens[i - 1].type >= TK_DOUBLE_BEGIN && tokens[i - 1].type <= TK_DOUBLE_END)
      || (tokens[i - 1].type >= TK_SINGLE_BEGIN && tokens[i - 1].type <= TK_SINGLE_END)
      || (tokens[i - 1].type == '+' || tokens[i - 1].type == '-')
      || (tokens[i - 1].type == '*' || tokens[i - 1].type == '/')
      || (tokens[i - 1].type == TK_LP))
      {
        tokens[i].type = TK_NEG;
        printf("Change position %d to TK_NEG\n", i);
        find = true;
      }
    }
  }
  return find;
}

bool check_parentheses(int p, int q, bool *bp) {
  // b, p is the edge of expression, bp shows whether the expression is valid or  not
  // bool to record the ret
  bool parenth = true;

  // stack to record paren match
  int stack = 0;

  // scan every token, ignore non-paren token
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_LP) {
      stack += 1;
    }
    else if (tokens[i].type == TK_RP) {
      stack -= 1;
    }
    // check the stack
    if (stack < 0) {
      // right paren with no left paren! bad expression
      *bp = true;
      return false;
    } else if (stack == 0 && i != q) {
      // the whole expression is not surrounded by a couple of paren
      parenth = false;
    }
  }

  // legal expression
  *bp = false;

  /*
  if (parenth) {
    printf("from %d to %d surounded!\n", p, q);
  }
  else {
    printf("from %d to %d not surounded!\n", p, q);
  }*/
  
  return parenth;
}

uint32_t eval(int p, int q) {
  bool bad_expression = false;
  if (p > q) {
    /* Bad expression */
    Log("Bad expression, p: %d, q: %d", p, q);
    assert(0);
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_DEC) {
      // Translate decimal to uint
      uint32_t val = 0;
      for (int i = 0; i < strlen(tokens[p].str); i++) {
        if (tokens[p].str[i] >= '0' && tokens[p].str[i] <= '9') {
          val *= 10;
          val += (int)(tokens[p].str[i] - '0');
        } else {
          Log("Wrong decimal input in pos: %d, str: %s", p, tokens[p].str);
          assert(0);
        }
      }
      // Log("Decimal str: %s, value: %d", tokens[p].str, val);
      return val;
    } else if (tokens[p].type == TK_HEX) {
      // translate heximal to uint
      uint32_t val = 0;
      // remember to ignore "0x"
      for (int i = 2; i < strlen(tokens[p].str); i++) {
        if (tokens[p].str[i] >= '0' && tokens[p].str[i] <= '9') {
          val *= 16;
          val += (int)(tokens[p].str[i] - '0');
        } else if (tokens[p].str[i] >= 'a' && tokens[p].str[i] <= 'f') {
          val *= 16;
          val += (int)(tokens[p].str[i] - 'a' + 10);
        } else if (tokens[p].str[i] >= 'A' && tokens[p].str[i] <= 'F') {
          val *= 16;
          val += (int)(tokens[p].str[i] - 'A' + 10);
        } else {
          Log("Wrong heximal input in pos: %d, str: %s", p, tokens[p].str);
          assert(0);
        }
      }
      // Log("Heximal str: %s, value: %d", tokens[p].str, val);
      return val;
    } else if (tokens[p].type >= TK_REG_BEGIN && tokens[p].type <= TK_REG_END) {
      // find register index
      uint32_t reg_index = 0;
      reg_index = tokens[p].type - TK_$0;
      assert(reg_index >= 0);
      assert(reg_index <= 31);

      // get register value
      bool suc = false;
      uint32_t reg_value = isa_reg_str2val(regsters_on_expr[reg_index], &suc);
      if (!suc) {
        Log("Wrong register read in pos: %d, reg: %s", p, regsters_on_expr[reg_index]);
        assert(0);
      }
      return reg_value;
    } else {
      Log("Wrong token type in pos: %d", p);
      assert(0);
    }
  }
  else if (check_parentheses(p, q, &bad_expression)) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    if (bad_expression) {
      Log("Bad expression, p: %d, q: %d", p, q);
      assert(0);
    }
    return eval(p + 1, q - 1);
  }
  else if (tokens[p].type >= TK_SINGLE_BEGIN && tokens[p].type <= TK_SINGLE_END) {
    /* The first token is single operation */
    switch (tokens[p].type) {
      case TK_NEG:
        return -eval(p+1, q);
      case TK_SOLV:
      // TODO
        return decode_addr(eval(p+1, q));
      case TK_NOT:
        return !eval(p+1, q);
    }
  }
  else {
    // If it's bad expression, error
    if (bad_expression) {
      Log("Bad expression, p: %d, q: %d", p, q);
      assert(0);
    }

    // Find main operation
    int stack = 0; // Ignore operations in parens
    int op = -1; // main operation
    for (int i = p; i <= q; i++) {
      if (tokens[i].type == '+' || tokens[i].type == '-'
        || tokens[i].type == '*' || tokens[i].type == '/'
        || (tokens[i].type >= TK_DOUBLE_BEGIN && tokens[i].type <= TK_DOUBLE_END))
        // || tokens[i].type == TK_EQ || tokens[i].type == TK_LE
        // || tokens[i].type == TK_ME || tokens[i].type == TK_LT
        // || tokens[i].type == TK_MT
      {
        // Ignore operations in parens
        if (stack != 0) {
          continue;
        }

        // find main operation
        if (tokens[i].type == '+' || tokens[i].type == '-') {
          if (op == -1 || tokens[op].type == '+' || tokens[op].type == '+') {
            op = i;
          }
        } else if (tokens[i].type == '*' || tokens[i].type == '/') {
          if (op == -1 || tokens[op].type == '+' || tokens[op].type == '+'
            || tokens[op].type == '*' || tokens[op].type == '/')
          {
            op = i;
          }
        } else if (tokens[i].type == TK_EQ || tokens[i].type == TK_LE
          || tokens[i].type == TK_ME || tokens[i].type == TK_LT
          || tokens[i].type == TK_MT)
        {
          if (op == -1 || tokens[op].type == '+' || tokens[op].type == '+'
            || tokens[op].type == '*' || tokens[op].type == '/'
            || tokens[op].type == TK_EQ || tokens[op].type == TK_LE
            || tokens[op].type == TK_ME || tokens[op].type == TK_LT
            || tokens[op].type == TK_MT)
          {
            op = i;
          }
        }
      } else if (tokens[i].type == TK_LP) {
        stack += 1;
        // printf("Stack plus: %d\n", stack);
      } else if (tokens[i].type == TK_RP) {
        stack -= 1;
        // printf("Stack minus: %d\n", stack);
      }
    }
    assert(op > 0);
    // then op contaims the main operation
    printf("main operation: %d\n", tokens[op].type);
    
    
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_EQ: return (int)(val1 == val2);
      case TK_LE: return (int)(val1 <= val2);
      case TK_ME: return (int)(val1 >= val2);
      case TK_LT: return (int)(val1 < val2);
      case TK_MT: return (int)(val1 > val2);
      default: assert(0);
    }
    return 0;
  }
  Log("Bad control stream at evaluation");
  assert(0);
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  Log("match succesfully! valid token number: %d", nr_token);

  if (trans_sing()) {
    Log("Single operation found!");
  } else {
    Log("No single operation found.");
  }

  /*
  for (int i = 0; i < nr_token; i++) {
    Log("Token%d Type: %d, Value: %s", i, tokens[i].type, tokens[i].str);
  }*/

  /* TODO: Insert codes to evaluate the expression. */
  uint32_t val = 0;
  val = eval(0, nr_token - 1);
  *success = true;

  return val;
}
