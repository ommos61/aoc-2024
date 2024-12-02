
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

void print_levels(struct level *levels) {
    struct level *l = levels;
    while (l != NULL) {
        printf("%d ", l->value);
        l = l->next;
    }
    printf("\n");
}

struct level *find_unsafe(struct level *levels) {
    assert(levels->next != NULL); // make sure there are always at least two levels in a report
    struct level *unsafe = NULL;
    struct level *l = levels;
    int direction = 0;
    while (l-> next != NULL) {
        int diff = (l->next->value - l->value);
        if (direction == 0) {
            direction = diff / abs(diff);
        }
        if ((diff / abs(diff)) != direction) {
            unsafe = l;
            break;
        } else if ((diff == 0) || (abs(diff) > 3)) {
            unsafe = l;
            break;
        }
        l = l->next;
    }
    return unsafe;
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
        struct level *unsafe = find_unsafe(reports[i]);
        if (unsafe == NULL) {
            safe_count += 1;
        } else {
            // try without the first level
            if (debug) { printf("testing without first level: "); print_levels(reports[i]->next); }
            if (find_unsafe(reports[i]->next) == NULL) {
                safe_count += 1;
                continue;
            }
            // try remove the first
            int save_value = unsafe->value;
            struct level *save_next = unsafe->next;
            unsafe->value = unsafe->next->value; unsafe->next = unsafe->next->next;
            if (debug) { printf("testing remove first: "); print_levels(reports[i]); }
            if (find_unsafe(reports[i]) == NULL) {
                // restore
                unsafe->value = save_value; unsafe->next = save_next;
                safe_count += 1;
                continue;
            } else {
                struct level *new_unsafe = find_unsafe(reports[i]);
                if (debug) printf("    New unsafe at %d -> %d\n", new_unsafe->value, new_unsafe->next->value);
            }
            // restore
            unsafe->value = save_value; unsafe->next = save_next;
            // try remove the second
            save_next = unsafe->next;
            unsafe->next = unsafe->next->next;
            if (debug) { printf("testing remove second: "); print_levels(reports[i]); }
            if (find_unsafe(reports[i]) == NULL) {
                // restore
                unsafe->next = save_next;
                safe_count += 1;
                continue;
            } else {
                struct level *new_unsafe = find_unsafe(reports[i]);
                if (debug) printf("    New unsafe at %d -> %d\n", new_unsafe->value, new_unsafe->next->value);
            }
            // restore
            unsafe->next = save_next;

            if (debug) print_levels(reports[i]);
            if (debug) printf("===> Still unsafe at %d -> %d\n", unsafe->value, unsafe->next->value);
        }
    }
    printf("Counted %d safe reports\n", safe_count);

    printf("Info: the solution for the sample data should be %d\n", 4);
    printf("Info: the solution for the actual data should be %d\n", 426);
    return EXIT_SUCCESS;
}

