
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 4096
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_PATTERNS 500
#define MAX_DESIGNS  500
char *patterns[MAX_PATTERNS];
unsigned pattern_count = 0;
char *designs[MAX_DESIGNS];
unsigned design_count = 0;

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

        if (strlen(line) > 0) {
            // parse and store the data
            if (pattern_count == 0) {
                char *p = line;
                while (*p != 0) {
                    assert(pattern_count < MAX_PATTERNS);
                    char buffer[LINE_LENGTH];
                    unsigned index = 0;
                    while (isalpha(*p)) {
                        buffer[index] = *p;
                        index += 1;
                        p += 1;
                    }
                    buffer[index] = 0;
                    char *tmp = malloc(strlen(buffer) + 1);
                    assert(tmp != NULL);
                    strcpy(tmp, buffer);
                    patterns[pattern_count] = tmp;
                    pattern_count += 1;
                    while (isspace(*p) || (*p == ',')) p += 1;
                }
            } else {
                assert(design_count < MAX_DESIGNS);
                char *tmp = malloc(strlen(line) + 1);
                assert(tmp != NULL);
                strcpy(tmp, line);
                designs[design_count] = tmp;
                design_count += 1;
            }
        } else if (strlen(line) == 0) {
            // ignore empty lines
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

void print_patterns(void) {
    for (unsigned i = 0; i < pattern_count; i++) {
        printf("%s%s", patterns[i], (i < (pattern_count - 1)) ? ", " : "");
    }
    printf("\n");
}
void print_designs(void) {
    for (unsigned i = 0; i < design_count; i++) {
        printf("%s%s", designs[i], (i < (design_count - 1)) ? ", " : "");
    }
    printf("\n");
}

char *colors = "wubrg";
struct patterns {
    char *pats[MAX_PATTERNS];
    unsigned count;
} split_patterns[5] = {0};
unsigned char2index(char c) {
    switch (c) {
        case 'w': return 0;
        case 'u': return 1;
        case 'b': return 2;
        case 'r': return 3;
        case 'g': return 4;
        default: printf("Illegal character '%c' in pattern\n", c);
    }
    assert(0 && "illegal character in pattern");
    return 0;
}
void do_split_patterns(void) {
    for (unsigned i = 0; i < pattern_count; i++) {
        unsigned index = char2index(patterns[i][0]);
        split_patterns[index].pats[split_patterns[index].count] = patterns[i];
        split_patterns[index].count += 1;
    }
}
void print_split_patterns(void) {
    for (unsigned i = 0; i < array_count(split_patterns); i++) {
        printf("'%c': ", colors[i]);
        for (unsigned j = 0; j < split_patterns[i].count; j++) {
            printf("%s%s", split_patterns[i].pats[j], (j < (split_patterns[i].count - 1)) ? ", " : "");
        }
        printf("\n");
    }
    assert(pattern_count == (split_patterns[0].count + split_patterns[1].count + split_patterns[2].count + split_patterns[3].count + split_patterns[4].count));
}

char *pattern_stack[1024];
unsigned matched_pattern_count = 0;
// keep a cache of what we already checked at the end that seemed ot fail
int checked[LINE_LENGTH];
int check_design(char *design, unsigned index, unsigned depth) {
    int check_ok = 0;

    if (depth == 0) memset(checked, 0, LINE_LENGTH * sizeof(int));

    //if (debug) printf("recurse(%s)\n", design + index);
    if (design[index] == 0) {
        //if (debug) printf("    found a combination of patterns\n");
        check_ok = 1;
    } else if (checked[index]) {
        check_ok = 0;
    } else {
        struct patterns pats = split_patterns[char2index(design[index])];
        char *des = design + index;
        for (unsigned pi = 0; pi < pats.count; pi++) {
            char *p = pats.pats[pi];
            int ok = 1;
            unsigned di = 0;
            while (p[di] != 0) {
                if (p[di] != des[di]) {
                    ok = 0; break;
                }
                di += 1;
            }
            if ((des[di] == 0) && (p[di] != 0)) {
                ok = 0;
            }
            if (ok) {
                check_ok = check_design(design, index + strlen(pats.pats[pi]), depth + 1);
                if (check_ok) {
                    //if (debug) printf("    last matched pattern was '%s'\n", patterns[pi]);
                    pattern_stack[matched_pattern_count] = pats.pats[pi];
                    matched_pattern_count += 1;
                    break;
                }
            }
        }
        checked[index] = 1;
    }

    return check_ok;
}

int lencmp(const void *v1, const void *v2) {
    return strlen(*(char **)v2) - strlen(*(char **)v1);
}
int lencmp2(const void *v1, const void *v2) {
    return strlen(*(char **)v1) - strlen(*(char **)v2);
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u patterns\n", pattern_count);
    printf("There are %u designs\n", design_count);
    if (debug) print_patterns();
    //if (debug) print_designs();

    // sort the patterns (larges first)
    qsort(patterns, pattern_count, sizeof(char *), lencmp);
    do_split_patterns();
    if (debug) print_split_patterns();

    // implement algorithm
    unsigned count = 0;
    for (unsigned i = 0; i < design_count; i++) {
        //unsigned des = design_count - 1 - i;
        unsigned des = i;
        if (debug) printf("Checking design %u/%u\n", i + 1, design_count);
        if (debug) printf("  design = '%s'\n", designs[des]);
        matched_pattern_count = 0;
        if (check_design(designs[des], 0, 0)) {
            if (debug) {
                ///printf("    used number of patterns is %u\n", matched_pattern_count);
                printf("  design = '");
                for (int i = matched_pattern_count - 1; i >= 0; i--) {
                    printf("%s", pattern_stack[i]);
                }
                printf("'\n");
            }
            count += 1;
        }
    }
    printf("There are %u designs that can be made with the patterns\n", count);

    printf("Info: the solution for the sample data should be %d\n", 6);
    printf("Info: the solution for the actual data should be %d\n", 308);
    return EXIT_SUCCESS;
}

