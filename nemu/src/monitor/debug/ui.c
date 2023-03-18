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

/**
 * d，lx，ld，，lu，这几个都是输出32位的
 * hd，hx，hu，这几个都是输出16位数据的，
 * hhd，hhx，hhu，这几个都是输出8位的，
 * lld，ll，llu，llx，这几个都是输出64位的，
 * 
 */

static int wp_count = 0;

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
  int N=0;
  if(args==NULL){
	  N=1;
  }
  else{
	  N = atoi(strtok(NULL, " "));
	  if(N <= 0){
		  printf("cmd_si error! input = %d\n",N);
		  return 0;
		}
	}
	cpu_exec(N);
	return 0;
}


		// for (int i = 0; i < 8; i++){
		// 	printf("%s  0x%x\t", regsl[i], reg_l(i));
    //   if(i%2==1)
    //       printf("\n");
    // }

		// for (int i = 0; i < 8; i++){
		// 	printf("%s  0x%x\t", regsw[i], reg_w(i));
    //   if(i%2==1)
    //       printf("\n");

    // }
		// for (int i = 0; i < 8; i++){
		// 	printf("%s  0x%x\t", regsb[i], reg_b(i));
    //   if(i%2==1)
    //       printf("\n");

void print_reg(){
  printf("%-8s0x%08x%16d\n", "eip", cpu.eip, cpu.eip);
    for (int i = 0; i < 8; ++i) {
      printf("%-8s0x%08x%16d\t", regsl[i], reg_l(i), reg_l(i));
      if(i%2==1)
          printf("\n");
  }

  for (int i = 0; i < 8; ++i) {
      printf("%-8s0x%08x%16d\t", regsw[i], reg_w(i), reg_w(i));
      if(i%2==1)
          printf("\n");
  }

  for (int i = 0; i < 8; ++i) {
      printf("%-8s0x%08x%16d\t", regsb[i], reg_b(i), reg_b(i));
      if(i%2==1)
          printf("\n");
  }
  return;
}

/* info */
static int cmd_info(char* args) {
	if (args == NULL) {
		printf("Command info should have a argument!\n");
		return 0;
	};

  if (strcmp(args,"w") == 0) {
		print_wp();
	}
	else if (strcmp(args,"r") == 0) {
    print_reg();
  }
  else{
    printf("The arguments must be r or w!\n");
  }

	return 0;
}

/* x */
static int cmd_x(char *args){

  if (args == NULL)
    printf("cmd_x args is NULL!\n");

  int len = atoi(strtok(NULL, " "));;
  char *exp = strtok(NULL, " ");

	uint32_t res;
  bool success;
  res = expr(exp,&success);
  if(!success)
	  printf("Expr calculation error!\n");

  for (int i = 0; i < len; i++) {
    printf("0x%08x\n", vaddr_read(res, 4));
    res += 4;
  }

  return 0;

}

/* p */
static int cmd_p(char *args){
	bool success=true;
	uint32_t res=expr(args,&success);

	if(success == false)
	  printf("Expr calculation error!\n");
	else
	  printf("Expr value = %d\n",res);

	return 0;
}

/* w */
static int cmd_w(char* args) {
  bool success=true;
	uint32_t res=expr(args,&success);

	if(success == false)
	  printf("Expr calculation error!\n");

	WP* wp = new_wp(res);
  strcpy(wp->expr, args);
  wp->result=res;

  wp->NO = ++wp_count;

  printf("Start watch: %u \n",res);
	return 0;
}

/* d */
static int cmd_d(char* args) {
	int num = 0;
	int input = sscanf(args, "%d", &num);
	if (input <= 0) {
		printf("args error in cmd_si\n");
		return 0;
	}
	bool res = delete_wp(num);
	if (res == true)
    return 1;
		// printf("error: no watchpoint %d\n", num);
	else
		printf("Success delete watchpoint %d\n", num);
	return 0;
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
  { "si", "Execute single-step", cmd_si },
  { "info", "Print reg info", cmd_info },
  { "x", "Scan memory", cmd_x },
  { "p", "Expr evaluation", cmd_p },
  { "w", "New a watchpoints", cmd_w },
  { "d", "Delete a watchpoints", cmd_d },
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
