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

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_LE,
  TK_ME,
  TK_LT,
  TK_MT,
  TK_LP,
  TK_RP,
  TK_HEX,
  TK_DEC,

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
  {"\\+", '+'},         // plus
  {"-", '-'},           // minus
  {"\\*", '*'},         // nulti
  {"/", '/'},           // devide
  {"==", TK_EQ},        // equal
  {"<=", TK_LE},        // less equal
  {">=", TK_ME},        // more equal
  {"<", TK_LT},         // less than
  {">", TK_MT},         // more than
  {"\\(", TK_LP},         // left paren
  {"\\)", TK_RP},         // right paren
  {"0[xX][0-9a-fA-F]+", TK_HEX}, // hex number
  {"[0-9]+", TK_DEC},   // dec number
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

static Token tokens[32] __attribute__((used)) = {};
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
          case '+':
          case '-':
          case '*':
          case '/':
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

bool check_parentheses(int p, int q, bool *bp) {
  *bp = false;
  return false;
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
      Log("Decimal str: %s, value: %d", tokens[p].str, val);
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
      Log("Heximal str: %s, value: %d", tokens[p].str, val);
      return val;
    } else {
      Log("Wrong token type in pos: %d", p);
      assert(0);
    }
  }
  else if (check_parentheses(p, q, &bad_expression) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    if (bad_expression) {
      Log("Bad expression, p: %d, q: %d", p, q);
      assert(0);
    }
    return eval(p + 1, q - 1);
  }
  else {
    // op = the position of 主运算符 in the token expression;
    /*
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);

    switch (op_type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }*/
    return 0;
  }
  Log("Bad control stream at eval");
  assert(0);
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  Log("match succesfully! valid token number: %d", nr_token);

  for (int i = 0; i < nr_token; i++) {
    printf("Token%d-Type:-%d,-Value:-%s\n", i, tokens[i].type, tokens[i].str);
  }

  /* TODO: Insert codes to evaluate the expression. */
  eval(0, nr_token - 1);

  return 0;
}
