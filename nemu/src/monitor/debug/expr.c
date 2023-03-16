#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

uint32_t expr(char *q, bool *success);
uint32_t hex_to_dec(char str[32]);
uint32_t eval(int p, int q);
int dominant_op(int p, int q);
bool check_parentheses(int left, int right);
int get_priority(int type, int layer);

#define DEBUG_CHECK 1

/* TODO: Add more token types */
enum {
    TK_NOTYPE = 256,
    TK_EQ,
    HEX,
    NUM,
    REG,
    TK_AND,
    TK_NOT,
    TK_OR,
    TK_NEQ,
    TK_MINUS,
    TK_DER
};

char* print_ch[]={
    "TK_NOTYPE",
    "TK_EQ",
    "HEX",
    "NUM",
    "REG",
    "TK_AND",
    "TK_NOT",
    "TK_OR",
    "TK_NEQ",
    "TK_MINUS",
    "TK_DER"
};

static struct rule {
    char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */
    {" +", TK_NOTYPE},    // spaces
    {"\\+", '+'},         // plus
    {"==", TK_EQ},         // equal
    {"0[xX][0-9a-fA-F]+",HEX},
    {"0|[1-9][0-9]*",NUM},
    {"\\-",'-'},
    {"\\*",'*'},
    {"\\/",'/'},
    {"\\(",'('},
    {"\\)",')'},
    {"\\$[a-z]+",REG},
    {"&&",TK_AND},
    {"[\\|]{2}", TK_OR}, //{"\\|\\|",TK_OR},
    {"!=",TK_NEQ}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %p\n%p", error_msg,
                  rules[i].regex);
        }
    }
}

const int max_len = 32;

typedef struct token {
    int type;
    char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *q) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (q[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], q + position, 1, &pmatch, 0) == 0 &&
                pmatch.rm_so == 0) {
                char *substr_start = q + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len,
                    substr_start);
                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */
                tokens[nr_token].type = rules[i].token_type;
                switch (rules[i].token_type) {
                case TK_NOTYPE:
                    break;
                case NUM:
                case REG:
                case HEX:
                    if (substr_len >= max_len) {
                        printf("str: %p 'p len %d out of the max size of char "
                               "array len %d!\n",
                               substr_start, substr_len, max_len);
                        break;
                    } else {
                        strncpy(tokens[nr_token++].str, substr_start,
                                substr_len);
                    }
                    break;

                default:
                    nr_token++;
                    break;
                }

                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%p\n%*.s^\n", position, q,
                   position, "");
            return false;
        }
    }

    if (tokens[0].type == '-') {
        tokens[0].type = TK_MINUS;
    } else if (tokens[0].type == '*') {
        tokens[0].type = TK_DER;
    }

    for (int j = 1; j < nr_token; ++j) {
        if (tokens[j].type == '-' && tokens[j - 1].type != ')' &&
            (tokens[j - 1].type > REG || tokens[j - 1].type < HEX))
            tokens[j].type = TK_MINUS;
        else if (tokens[j].type == '*' && tokens[j - 1].type != ')' &&
                 (tokens[j - 1].type > REG || tokens[j - 1].type < HEX))
            tokens[j].type = TK_DER;
    }

    return true;
}

uint32_t expr(char *q, bool *success) {
    if (!make_token(q)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    // TODO();
    if (!check_parentheses(0, nr_token)) {
        *success = false;
        return 0;
    } else {
        return eval(0, nr_token - 1);
    }

    return 0;
}

uint32_t hex_to_dec(char str[32]) {
    uint result = 0;
    for (int i = 2; i < 10; ++i) {
        int tmp = 0;
        if (str[i] >= '0' && str[i] <= '9') {
            tmp = (int)(str[i] - '0');
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            tmp = (int)(str[i] - 'a') + 10;
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            tmp = (int)(str[i] - 'A') + 10;
        }
        result = 16 * result + tmp;
    }
    return result;
}
uint32_t eval(int p, int q) {
#ifdef DEBUG_CHECK
    printf("p = %d str[p] = %d, q = %d str[q] = %d\n", p, tokens[p].type, q,
           tokens[q].type);
#endif
    if (p > q) {
        /*Bad expression */
        printf("p > q\n");
        assert(0);

    } else if (p == q) {
        /* Single token .
         * For now this token should be a number .
         * Return the value of the number .
         */
        if (tokens[p].type == NUM) {
            return atoi(tokens[p].str);
        } else if (tokens[p].type == HEX) {
            return hex_to_dec(tokens[p].str);
        } else if (tokens[p].type == REG) {
            int reg_res = get_reg_val(tokens[p].str + 1);
            if (reg_res != -1)
                return reg_res;
            else {
                printf("get_reg_val fail! \n");
                assert(0);
            }
        }
    } else if (p == q - 1) {
        if (tokens[p].type == TK_NOT) {
            return !eval(q, q);
        } else if (tokens[p].type == TK_MINUS) {
            return -1 * eval(p + 1, q);
        }

    } else if (tokens[p].type == '(' && tokens[q].type == ')') {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case，just throw away the parentheses.
         */
        return eval(p + 1, q - 1);

    } else {
        /* we should do more things here. */
        int op = dominant_op(p, q);
#ifdef DEBUG_CHECK
        printf("\n=== op = %d \n", op);
#endif
        uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);

        switch (tokens[op].type) {
        case '+':
            return val1 + val2;
        case '-':
            return val1 - val2;
        case '*':
            return val1 * val2;
        case '/':
            return val1 / val2;
        case TK_EQ:
            return val1 == val2;
        case TK_NEQ:
            return val1 != val2;
        case TK_AND:
            return val1 && val2;
        case TK_OR:
            return val1 || val2;
        default:
            printf("token computing fail!\n");
            assert(0);
        }
    }
    return 0;
}

int dominant_op(int p, int q) {
    int op = 0;
    int minPrt = 6;
    int prt = -1;
    int layer = 0;
    for (int i = p; i <= q; ++i) {
        if (tokens[i].type == '(') {
            layer++;
            continue;
        } else if (tokens[i].type == ')') {
            layer--;
            continue;
        } else if (get_priority(tokens[i].type, layer) == 0) {
            continue;
        }
        prt = get_priority(tokens[i].type, layer);
#ifdef DEBUG_CHECK
        printf("i = %d ; prt = %d ; %d\n", i, prt, tokens[i].type);
#endif
        if (prt < minPrt) {
            minPrt = prt;
            op = i;
        }
    }
    return op;
}

bool check_parentheses(int left, int right) {
    int layer = 0;
    for (int i = left; i < right; ++i) {
        if (tokens[i].type == '(')
            layer++;
        else if (tokens[i].type == ')')
            layer--;
    }
    if (layer == 0)
        return true;
    else
        return false;
}

int get_priority(int type, int layer) {
    if (layer == 0) {
        switch (type) {
        case NUM:
        case REG:
        case HEX:
        case TK_MINUS:
        case TK_DER:
            return 0;
        case TK_OR:
            return 1;
        case TK_AND:
            return 2;
        case TK_EQ:
        case TK_NEQ:
            return 3;
        case '+':
        case '-':
            return 4;
        case '*':
        case '/':
            return 5;
        default:
            printf("get_priority fail!\n");
            assert(0);
        }
    } else {
        return 6;
    }
}