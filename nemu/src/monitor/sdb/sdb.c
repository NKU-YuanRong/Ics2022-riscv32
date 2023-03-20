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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

#include <time.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();


/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  // -1 the parameter is uint64_t type, so -1 means the max value of uint64_t(full of 1)
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  cpu_quit();
  return -1;
}

static int cmd_help(char *args);

uint64_t str2u64t(char *args);

// command single executing, use uint32_t
static int cmd_si(char *args);

// command shows program status
static int cmd_info(char *args);

static int info_r() {
  isa_reg_display();
  return 0;
}

// command scan the memory
static int cmd_x(char *args);

// Solve expression
static int cmd_p(char *args);

// cmd tp to test cmd_p
static int cmd_pt(char *args);


// struct set for info command
static struct {
  const char *arg;
  const char *description;
  int (*handler) (void);
} info_table [] = {
  { "r", "Print all registers", info_r },
};


static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Execute sigle instruction", cmd_si },
  { "info", "Show program status", cmd_info },
  { "x", "Scan a storage area", cmd_x },
  { "p", "Solve an expression", cmd_p },

  // test instructions
  { "pt", "Test Instruction p", cmd_pt },
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}


uint64_t str2u64t(char *args) {
  //just use the first argument, ignore others
  args = strtok(args, " ");

  if (args == NULL) {
    assert(0);
  }

  uint64_t N = 0;

  char *args_ptr = args;
  for (int i = 0; i < strlen(args); i++) {
    if ('0' <= args_ptr[i] && '9' >= args_ptr[i]) {
      N *= 10;
      N += (uint64_t)(args_ptr[i] - '0');
    }
    else {
      // invalid input
      assert(0);
    }
  }
  return N;
}

static int cmd_si(char *args) {
  // just run N insts
  if (args == NULL) {
    cpu_exec(1);
    return 0;
  }

  // N to record the specific value of the first argument
  uint64_t N = 0;

  // ignore useless arguments
  args = strtok(args, " ");

  // calculating the value of N
  N = str2u64t(args);

  // printf("Test: N = %lu\n", N);
  // execute N insts
  cpu_exec(N);

  return 0;
}

static int cmd_info(char *args) {
  if (args != NULL) {
    // ignore useless arguments
    args = strtok(args, " ");

    // find the operation
    int i;
    for (i = 0; i < ARRLEN(info_table); i ++) {
      if (strcmp(args, info_table[i].arg) == 0) {
        // find match, call the process function and quit
        info_table[i].handler();
        return 0;
      }
    }
  }
  // no argument or invalid argument, print inst list
  int i;
  printf("For help, type 'help'.\n");
  printf("List of info subcommands:\n\n");
  for (i = 0; i < ARRLEN(info_table); i ++) {
    printf("info %s -- %s\n", info_table[i].arg, info_table[i].description);
  }
  return 0;
}

static int cmd_x(char *args) {
  // N to record the specific value of the first argument
  
  uint64_t N = 0;
  uint32_t exp_value = 0;

  // arg to store every argument
  char *arg = strtok(args, " ");
  char *exp = strtok(NULL, " ");

  if (arg != NULL) {
    N = str2u64t(arg);
    if (exp != NULL) {
      bool suc;
      exp_value = expr(exp, &suc);
      assert(suc);
      int i;
      for (i = 0; i < N; i++) {
        printf("0x%08x: ", (uint32_t)exp_value);
        printf("%02x %02x %02x %02x\n", vaddr_read(exp_value + 3, 1), vaddr_read(exp_value + 2, 1), vaddr_read(exp_value + 1, 1), vaddr_read(exp_value, 1));
        printf("%d %d %d %d\n", vaddr_read(exp_value + 3, 1), vaddr_read(exp_value + 2, 1), vaddr_read(exp_value + 1, 1), vaddr_read(exp_value, 1));
        exp_value += 4;
      }
      return 0;
    }
  }
  printf("x command with no argument!\n");
  assert(0);
  
  return 0;
}

static int cmd_p(char *args) {
  bool suc = false;
  uint32_t val = expr(args, &suc);
  if (!suc) {
    Log(ANSI_FMT("Solve fail!", ANSI_FG_RED));
  }
  printf("Token Value: %d\n", val);
  return 0;
}

// record length of expression 
int exp_len = 0;
// max token number(in expr.c) is 64, so max expression length is 63
const int max_exp_len = 63;

// Generate rand operation
void gen_rand_operation(char *exp) {
  if (exp_len > max_exp_len) {
    return;
  }
  switch (rand() % 9) {
    case 0:
      strcat(exp, "+");
      exp_len++;
      return;
    case 1:
      strcat(exp, "-");
      exp_len++;
      return;
    case 2:
      strcat(exp, "*");
      exp_len++;
      return;
    case 3:
      strcat(exp, "+");
      exp_len++;
      return;
    case 4:
      strcat(exp, "==");
      exp_len++;
      return;
    case 5:
      strcat(exp, "<=");
      exp_len++;
      return;
    case 6:
      strcat(exp, ">=");
      exp_len++;
      return;
    case 7:
      strcat(exp, "<");
      exp_len++;
      return;
    case 8:
      strcat(exp, ">");
      exp_len++;
      return;
  }
}

// Generate rand expression
void gen_rand_expr(char *exp) {
  if (exp_len > max_exp_len) {
    return;
  }
  char num[10];
  switch (rand() % 5) {
    case 0:
      snprintf(num, 10, "%d", rand());
      strcat(exp, num);
      exp_len++;
      return;
    case 1:
      snprintf(num, 10, "%x", rand());
      char *num2 = "0x";
      strcat(exp, num2);
      strcat(exp, num);
      exp_len++;
      return;
    case 2:
      strcat(exp, "(");
      exp_len++;
      gen_rand_expr(exp);
      strcat(exp, ")");
      exp_len++;
      return;
    default:
      gen_rand_expr(exp);
      gen_rand_operation(exp);
      gen_rand_expr(exp);
      return;
  }
}

// cmd tp to test cmd_p
static int cmd_pt(char *args) {
  bool suc = false;
  uint32_t val;
  args = strtok(args, " ");
  uint64_t NUM, MNUM;
  if (args == NULL) {
    NUM = 100;
  } else {
    NUM = str2u64t(args);
  }
  MNUM = NUM;
  char exp[500] = "";
  time_t t;
  srand((unsigned) time(&t));
  while (NUM-- > 0) {
    exp[0] = '\0';
    exp_len = 0;
    gen_rand_expr(exp);
    if (exp_len > max_exp_len) {
      continue;
    }
    // gen_rand_operation(exp);
    printf("%ld Expression: %s, length: %d\n", MNUM - NUM, exp, exp_len);
    val = expr(exp, &suc);
    printf("%ld Token Value: %d\n", MNUM - NUM, val);
  }
  return 0;
}


void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
