
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH (1024 * 10)
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_LINES 20
char *lines[MAX_LINES];
unsigned line_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) > 0) {
            // parse the data
            lines[line_count] = malloc(strlen(line) + 1);
            strcpy(lines[line_count], line);
            line_count += 1;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

const char *valid_mul = "mul\\([0-9][0-9]?[0-9]?,[0-9][0-9]?[0-9]?\\)";
const char *find_valid_mul(const char *str) {
    const char *p = str;
    if (debug) printf("handling: '%s'\n", p);
    if (debug) printf("regex = '%s'\n", valid_mul);
    while (*p != 0) {
        regex_t regex;
        if (regcomp(&regex, valid_mul, REG_EXTENDED)) {
            printf("regex = '%s'\n", valid_mul);
            assert(0 && "error in regex");
        }
        regmatch_t match;
        int res = regexec(&regex, p, 1, &match, 0);
        //printf("regexec returned %d for '%s'\n", res, p);
        if (res == 0) {
            if (debug) printf("found '%s' at offset %ld\n", p, match.rm_so); 
            p = p + match.rm_so;
            break;
        }
        p += 1;
    }
    return p;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    if (debug) printf("there are %d lines\n", line_count);

    // implement algorithm
    long solution = 0;
    for (unsigned i = 0; i < line_count; i++) {
        const char *p = lines[i];
        while (*p != 0) {
            p = find_valid_mul(p);
            if (*p != 0) {
                int val1, val2;
                int cnt = sscanf(p, "mul(%d,%d)", &val1, &val2);
                assert(cnt == 2);
                solution += val1 * val2;
                p += 1;
            }
        }
    }
    printf("The calculated solution is %ld\n", solution);

    printf("Info: the solution for the sample data should be %d\n", 161);
    printf("Info: the solution for the actual data should be %ld\n", 164730528L);
    return EXIT_SUCCESS;
}

