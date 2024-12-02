
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
struct level {
    int value;
    struct level *next;
};
#define MAX_REPORTS 2000
struct level *reports[MAX_REPORTS];
unsigned report_count = 0;

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

        struct level **cur = &reports[report_count];
        *cur = NULL;
        char *p = line;
        while (*p != 0) {
            int value = 0;
            while (isdigit(*p)) {
                value = value * 10 + (*p - '0');
                p += 1;
            }
            *cur = malloc(sizeof(struct level));
            (*cur)->value = value;
            (*cur)->next = NULL;
            cur = &(*cur)->next;
            while (isspace(*p)) p += 1;
        }
        report_count += 1;

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

int is_safe(struct level *levels) {
    assert(levels->next != NULL); // make sure there are always at least two levels in a report
    int safe = 1;
    struct level *l = levels;
    int direction = 0;
    while (l-> next != NULL) {
        int diff = (l->next->value - l->value);
        if (direction == 0) {
            direction = diff / abs(diff);
        }
        if ((diff / abs(diff)) != direction) {
            safe = 0;
            break;
        } else if ((diff == 0) || (abs(diff) > 3)) {
            safe = 0;
            break;
        }
        l = l->next;
    }
    return safe;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);

    // implement algorithm
    int safe_count = 0;
    for (unsigned i = 0; i < report_count; i++) {
        if (is_safe(reports[i])) {
            safe_count += 1;
        }
    }
    printf("Counted %d safe reports\n", safe_count);

    printf("Info: the solution for the sample data should be %d\n", 2);
    printf("Info: the solution for the actual data should be %d\n", 371);
    return EXIT_SUCCESS;
}

