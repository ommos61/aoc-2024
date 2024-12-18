
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
struct state {
    long A, B, C;
    unsigned pc;
} state;
#define MAX_PROGRAM 50
unsigned program[MAX_PROGRAM];
unsigned program_size = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    state.A = -1; state.B = -1; state.C = -1; state.pc = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        int value = 0;
        // store the data
        if (sscanf(line, "Register A: %d", &value) == 1) {
            state.A = value;
        } else if (sscanf(line, "Register B: %d", &value) == 1) {
            state.B = value;
        } else if (sscanf(line, "Register C: %d", &value) == 1) {
            state.C = value;
#define PROGRAM_PREFIX "Program: "
        } else if (strncmp(line, PROGRAM_PREFIX, strlen(PROGRAM_PREFIX)) == 0) {
            char *p = line + strlen(PROGRAM_PREFIX);
            printf("X: %s\n", p);
            while (*p != 0) {
                value = 0;
                while (isdigit(*p)) {
                    value = 10 * value + (*p - '0');
                    p += 1;
                }
                program[program_size] = value;
                program_size += 1;
                if (*p == ',') p += 1;
            }
        } else if (strlen(line) == 0) {
            // ignore empty line
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_state(void) {
    printf("state = { A: %ld, B: %ld, C: %ld, pc=%u }\n", state.A, state.B, state.C, state.pc);
}
void print_program(void) {
    printf("program(len=%u): ", program_size);
    for (unsigned i = 0; i < program_size; i++) {
        printf("%d%s", program[i], (i != program_size - 1) ? ", " : "");
    }
    printf("\n-------------------------------------------------------------------------\n");
}

unsigned combo(unsigned op) {
    unsigned result = 0;
    switch (op) {
        case 0:
        case 1:
        case 2:
        case 3:
            result = op; break;
        case 4: result = state.A; break;
        case 5: result = state.B; break;
        case 6: result = state.C; break;
        case 7:
            printf("Illegal combo operand 7\n"); break;
        default:
            printf("Illegal combo operand %u\n", op); break;
    }
    return result;
}

void adv(unsigned operand) {
    state.A = state.A / (1 << combo(operand));
    state.pc += 2;
}
void blx(unsigned operand) {
    state.B = state.B ^ operand;
    state.pc += 2;
}
void bst(unsigned operand) {
    state.B = combo(operand) & 0x7;
    state.pc += 2;
}
void jnz(unsigned operand) {
    if (state.A != 0) {
        state.pc = operand;
    }
}
void bxc(unsigned operand) {
    (void)operand;
    state.B = state.B ^ state.C;
    state.pc += 2;
}
#define MAX_OUTPUT 100
unsigned outputs[MAX_OUTPUT];
unsigned output_count = 0;
void out(unsigned operand) {
    assert(output_count < MAX_OUTPUT);
    outputs[output_count] = combo(operand) % 8;
    output_count += 1;
    state.pc += 2;
}
void bdv(unsigned operand) {
    state.B = state.A / (1 << combo(operand));
    state.pc += 2;
}
void cdv(unsigned operand) {
    state.C = state.A / (1 << combo(operand));
    state.pc += 2;
}

void print_output(void) {
    printf("(len=%u): ", output_count);
    for (unsigned i = 0; i < output_count; i++) {
        printf("%d%s", outputs[i], (i != output_count - 1) ? ", " : "");
    }
    printf("\n");
}

void debugger_step(void) {
    assert(state.pc < program_size);
    unsigned opcode = program[state.pc];
    unsigned operand = program[state.pc + 1];
    switch (opcode) {
    case 0: {
            adv(operand);
        } break;
    case 1: {
            blx(operand);
        } break;
    case 2: {
            bst(operand);
        } break;
    case 3: {
            jnz(operand);
        } break;
    case 4: {
            bxc(operand);
        } break;
    case 5: {
            out(operand);
        } break;
    case 6: {
            bdv(operand);
        } break;
    case 7: {
            cdv(operand);
        } break;
    default: {
        printf("Unknown opcode %u at pc=%u\n", opcode, state.pc);
        } break;
    }
}

int check_program(unsigned *a, unsigned lena, unsigned *b, unsigned lenb) {
    int same = (lena == lenb);
    if (same) {
        for (unsigned i = 0; i < lena; i++) {
            if (a[i] != b[i]) {
                same = 0;
                break;
            }
        }
    }
    return same;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    print_state();
    print_program();

    // run the program
    //if (debug) printf("LONG_MAX is %ld\n", LONG_MAX);
    long factors[16];
    factors[0] = 1L;
    for (unsigned i = 1; i < 16; i++) factors[i] = 8L * factors[i - 1];

    unsigned muls[16]; for (unsigned i = 0; i < array_count(muls); i++) muls[i] = 0;
    muls[15] = 2;
    muls[14] = 4;
    muls[13] = 5;
    muls[12] = 3;
    muls[11] = 2;
    muls[10] = 5;
    muls[ 9] = 3;
    muls[ 8] = 4;
    muls[ 7] = 3;
    muls[ 6] = 5;
    muls[ 5] = 4;
    muls[ 4] = 0;
    muls[ 3] = 0;
    muls[ 2] = 3;
    muls[ 1] = 5;
    muls[ 0] = 1;
#if 0
#endif
    long value = 0L;
    for (unsigned i = 0; i < array_count(factors); i++) value += (factors[i] * muls[i]);
    long add = 0 * factors[0];
    unsigned s = 0;
    long startA = 0;
    while (1) {
#define MAX_STEPS 100000
#define INACTIVE_STEP_COUNT 1000
        unsigned step_count = 0;
        unsigned save_output_count = 0, save_step_count = 0;
        startA = value + s * add;
        state.A = startA;
        output_count = 0;
        // perform the program
        while ((state.pc < program_size) && (step_count < MAX_STEPS)) {
            debugger_step();
            step_count += 1;
            if (output_count > program_size) {
                break;
            } else if (save_output_count != output_count) {
                save_output_count = output_count;
                save_step_count = step_count;
            } else if ((step_count - save_step_count) > INACTIVE_STEP_COUNT) {
                break;
            }
        }
        if (program_size == output_count) {
            if (debug) { printf("%5u: ", s); print_output(); }
        }
        int same = check_program(program, program_size, outputs, output_count);
        if (same) {
            printf("==============SAME===============\n");
            if (debug) { printf("%5ld: ", startA); print_output(); }
            break;
        }

        s += 1; if (s > 7) break;
    }
    printf("The calculated start value for A is %ld\n", startA);

    printf("Info: the solution for the sample data should be %ld\n", 117440L);
    printf("Info: the solution for the actual data should be %ld\n", 90938893795561L);
    return EXIT_SUCCESS;
}

