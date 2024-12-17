
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
int program[MAX_PROGRAM];
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
    printf("program: ");
    for (unsigned i = 0; i < program_size; i++) {
        printf("%d%s", program[i], (i != program_size - 1) ? ", " : "");
    }
    printf("\n");
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
char outputs[MAX_OUTPUT];
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
#define MAX_STEPS 100000
    unsigned step_count = 0;
    while ((state.pc < program_size) && (step_count < MAX_STEPS)) {
        debugger_step();
        step_count += 1;
        //if ((step_count % 10) == 0) { printf("(pc=%u)", state.pc); getchar(); }
    }
    printf("Program ended after %u steps\n", step_count);
    for (unsigned i = 0; i < output_count; i++) {
        printf("%u%s", outputs[i], (i < output_count - 1) ? "," : "");
    }
    printf("\n");

    printf("Info: the solution for the sample data should be %s\n", "4,6,3,5,6,3,5,2,1,0");
    printf("Info: the solution for the actual data should be %s\n", "2,3,6,2,1,6,1,2,1");
    return EXIT_SUCCESS;
}

