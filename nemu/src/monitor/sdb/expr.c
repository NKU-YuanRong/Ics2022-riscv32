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


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  Log("match succesfully! valid token number: %d\n", nr_token);

  for (int i = 0; i < nr_token; i++) {
    printf("Token%d-Type:-%d,-Value:-%s\n", i, tokens[i].type, tokens[i].str);
  }

  return 1;

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
