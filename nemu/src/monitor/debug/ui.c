#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/**
 * @brief 
 * help Display informations about all supported commands
 * h:      Display informations about all supported commands
 * c:      Continue the execution of the program
 * q:      Exit NEMU
 * si:     单步执行N 条指令后暂停, N 缺省为1: si 3
 * s:      单步执行N 条指令后暂停, N 缺省为1: s 3
 * info:   打印寄存器状态和监视点信息: info r , info w
 * i:      打印寄存器状态和监视点信息: i r, i w
 * p:      求表达式EXPR 的值: p $eax+1, p 0x 12+3 * 4
 * x:      查看内存(显示N 个4 字节): x 50 x 100000
 * w:      设置监视点: 当EXPR 值变化时暂停程序: w * 0 x 2000, w $eax+1
 * d:      删除序号为N 的监视点: d 2
 * 
 */
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
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

/* c */
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

/* q */
static int cmd_q(char *args) {
  return -1;
}

/* h */
static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si","exec gradully",cmd_si},
  { "info","print reg",cmd_info},
  { "x","scan memory",cmd_x},
  { "p","expr calculate",cmd_p},
  { "w","new a watchpoint",cmd_w},
  { "d","delete watchpoint",cmd_d},

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
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

/* si */
static int cmd_si(char *args){
  uint64_t N=0;
  if(args==NULL)
  {
	  N=1;
  }
  else
	{
	  int input=sscanf(args,"%llu",&N);
	  if(input<=0){
		  printf("cmd_si error! input = %llu\n",input);
		  return 0;
		}
	}
	cpu_exec(N);
	return 0;
}

/* info */
static int cmd_info(char* args) {
	char s;
	if (args == NULL) {
		printf("args error in cmd_info\n");
		return 0;
	};

	int input = sscanf(args, "%c", &s);
	if (input <= 0) {
		printf("cmd_info error! input = %llu \n",input);
		return 0;
	}
  if (s == 'w') {
		print_wp();
		return 0;
	}
	else if (s == 'r') {
		for (int i = 0; i < 8; i++)
			printf("%s  0x%x\n", regsl[i], reg_l(i));

		for (int i = 0; i < 8; i++)
			printf("%s  0x%x\n", regsw[i], reg_w(i));

		for (int i = 0; i < 8; i++)
			printf("%s  0x%x\n", regsb[i], reg_b(i));
      
		return 0;
	}
	return 0;
}