#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// this should be enough
static char buf[65536] = {};
static char output_buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format = "#include <stdio.h>\n"
                           "int main() { "
                           "  unsigned result = (%s); "
                           "  printf(\"%%u\", result); "
                           "  return 0; "
                           "}";

uint32_t choose(uint32_t n) { return rand() % n; }

void gen(char c) {
    char cha_buffer[2] = {c, '\0'};
    strcat(buf, cha_buffer);
}

void gen2(char *ch) {
    char cha_buffer[3] = {ch[0], ch[1], '\0'};
    strcat(buf, cha_buffer);
}

void gen_rand_op() {
    char cha_buffer[3];
    switch (choose(8)) {
    case 0:
        gen('+');
        break;
    case 1:
        gen('-');
        break;
    case 2:
        gen('*');
        break;
    case 3:
        gen('/');
        break;
    case 4:
        gen2("&&");
        break;
    case 5:
        gen2("||");
        break;
    case 6:
        gen2("==");
        break;
    case 7:
        gen2("!=");
        break;
    }
    return;
}

uint32_t gen_num() {
    char num_buffer[1024];
    num_buffer[0] = '\0';
    uint32_t number = rand() % 100 + 1;
    sprintf(num_buffer, "%du", number);
    strcat(buf, num_buffer);
    return number;
}

void generate_output() {
    int j = 0;
    for (int i = 0; buf[i] != '\0'; ++i) {
        if (buf[i] != 'u') {
            output_buf[j++] = buf[i];
        }
    }
    output_buf[j] = '\0';
}


static void gen_rand_expr(int depth) {
    if (strlen(buf) > 65536 - 10000 || depth > 15) {
        gen('(');
        gen_num();
        gen(')');
        return;
    }

    switch (choose(3)) {
    case 0:
        gen_num();
        break;

    case 1:
        gen('(');
        gen_rand_expr(depth + 1);
        gen(')');
        break;

    default: {
        gen_rand_expr(depth + 1);
        gen_rand_op();
        gen_rand_expr(depth + 1);
        break;
    }
}
}

int main(int argc, char *argv[]) {
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }
    int i;
    for (i = 0; i < loop; i++) {
        buf[0] = '\0';
        gen_rand_expr(0);
        generate_output();

        sprintf(code_buf, code_format, buf);
        FILE *fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        system("gcc /tmp/.code.c -o /tmp/.expr");

        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        uint32_t result = 0u;
        fscanf(fp, "%u", &result);
        pclose(fp);

        fp = fopen("expr.txt", "a");
        assert(fp != NULL);
        sprintf(code_buf, "%u %s\n", result, output_buf);
        fputs(code_buf, fp);
        fclose(fp);
        printf("%u %s\n", result, output_buf);
    }
    return 0;
}