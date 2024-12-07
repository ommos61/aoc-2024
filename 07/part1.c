
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
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_EQUATIONS 1000
struct in {
    long value;
    struct in *next;
};
struct eq {
    long result;
    struct in *inputs;
} equations[MAX_EQUATIONS];
unsigned equation_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        long value = 0;
        if (sscanf(line, "%ld:", &value) == 1) {
            // store the data
            equations[equation_count].result = value;
            equations[equation_count].inputs = NULL;
            char *p = line;
            while (isdigit(*p)) p += 1;
            assert(*p == ':'); p += 1;
            struct in **cur = &(equations[equation_count].inputs);
            while (*p != 0) {
                while (isspace(*p)) p += 1;
                long val = 0;
                while (isdigit(*p)) {
                    val = 10 * val + (*p - '0');
                    p += 1;
                }
                struct in *input = malloc(sizeof(struct in));
                assert(input != NULL);
                input->value = val;
                input->next = NULL;
                *cur = input;
                cur = &input->next;
            }
            equation_count += 1;
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

void print_equation(struct eq *equation) {
    printf("%ld: ", equation->result);
    struct in *ins = equation->inputs;
    while (ins != NULL) {
        printf("%ld ", ins->value);
        ins = ins->next;
    }
    printf("\n");
}

int is_valid(long result, long current, struct in *inputs) {
    if (inputs == NULL) {
        return (current == result);
    } else {
        if (is_valid(result, current * inputs->value, inputs->next)) {
            return 1;
        } else if (is_valid(result, current + inputs->value, inputs->next)) {
            return 1;
        }
    }
    return 0;
}

int check_equation(struct eq *equation) {
    long res = equation->inputs->value;
    int valid = is_valid(equation->result, res, equation->inputs->next);
    return valid;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d equations\n", equation_count);

    // TODO: implement algorithm
    long solution = 0;
    for (unsigned i = 0; i < equation_count; i++) {
        if (debug) print_equation(equations + i);
        if (check_equation(equations + i)) {
            solution += equations[i].result;
        }
    }
    printf("calculated solution is %ld\n", solution);

    printf("Info: the solution for the sample data should be %ld\n", 3749L);
    printf("Info: the solution for the actual data should be %ld\n", 1298103531759L);
    return EXIT_SUCCESS;
}

