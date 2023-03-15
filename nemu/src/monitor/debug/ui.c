#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/**
| 命令         | 格式        | 说明                                                                          | 使用举例              |
| ------------ | ----------- | ---------------------------------------------------------------------------- | --------------------- |
| 帮助(1)      | help        | 打印命令的帮助信息                                                            | help                  |
| 继续运行(1)  | c           | 继续运行被暂停的程序                                                          | c                     |
| 退出(1)      | q           | 退出NEMU                                                                     | q                     |
| 单步执行     | si [N]     | 让程序单步执行N条指令后暂停执行，当N没有给出时，缺省为1                           | si 10                 |
| 打印程序状态 | info SUBCMD | 打印寄存器状态，打印监视点信息                                                 | info r info w         |
| 表达式求值   | p EXPR      | 求出表达式EXPR的值，EXPR支持的运算请见调试中的表达式求值小节                    | p $eax + 1            |
| 扫描内存(2)  | x N EXPR    | 求出表达式EXPR的值，将结果作为起始内存地址，以十六进制形式输出连续的N个4字节     | x 10 $esp             |
| 设置监视点   | w EXPR      | 当表达式EXPR的值发生变化时，暂停程序执行                                       | w\*0x2000             |
| 删除监视点   | d N         | 删除序号为N的监视点                                                           | d 2                   |
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
		  printf("cmd_si error! input = %d\n",input);
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
		printf("cmd_info error! input = %d \n",input);
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

/* x */
static int cmd_x(char *args){ return 0;}

/* p */
static int cmd_p(char *args){ return 0;}

/* w */
static int cmd_w(char *args){ return 0;}

/* d */
static int cmd_d(char *args){ return 0;}


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
  { "si", "Execute single-step", cmd_si },
  { "info", "Print reg info", cmd_info },
  { "x", "Scan memory", cmd_x },
  { "p", "Expr evaluation", cmd_p },
  { "w", "New watchpoints", cmd_w },
  { "d", "Delete watchpoints", cmd_d },
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
