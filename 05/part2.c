
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
#define MAX_RULES 1500
struct rule {
    unsigned first;
    unsigned second;
} rules[MAX_RULES];
unsigned rule_count = 0;
#define MAX_UPDATES 500
struct page {
    unsigned number;
    struct page *next;
} *updates[MAX_UPDATES];
unsigned update_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    int still_rules = 1;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        int value1 = 0, value2 = 0;
        if (strlen(line) == 0) { still_rules = 0; continue; }
        if (still_rules) {
            if (sscanf(line, "%d|%d", &value1, &value2) == 2) {
                // store the data
                rules[rule_count].first = value1;
                rules[rule_count].second = value2;
                rule_count += 1;
            } else if (errno != 0) {
                perror("sscanf");
            }
        } else {
            char *s = line;
            struct page **update_last = &updates[update_count];
            while (*s != 0) {
                int value = 0;
                while (isdigit(*s)) {
                    value = 10 * value + (*s - '0');
                    s += 1;
                }
                struct page *p = malloc(sizeof(struct page));
                p->number = value; p->next = NULL;
                *update_last = p;
                update_last = &(p->next);
                if (*s == ',') s += 1;
            }
            update_count += 1;
        }
        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_rules(void) {
    for (unsigned i = 0; i < rule_count; i++) {
        printf("%d|%d\n", rules[i].first, rules[i].second);
    }
}

void print_update(struct page *update) {
        struct page *p = update;
        while (p != NULL) {
            printf("%d%s", p->number, (p->next != NULL) ? ", " : "");
            p = p->next;
        }
        printf("\n");
}

void print_updates(void) {
    for (unsigned i = 0; i < update_count; i++) {
        print_update(updates[i]);
    }
}

int is_valid_order(unsigned p1, unsigned p2) {
    int valid = 1;
    for (unsigned r = 0; r < rule_count; r++) {
        if ((p1 == rules[r].second) && (p2 == rules[r].first)) {
            valid = 0;
            break;
        }
    }
    return valid;
}

int is_valid(struct page *update) {
    int valid = 1;
    struct page *el1 = update;
    while (valid &&el1 != NULL) {
        struct page *el2 = el1->next;
        while (el2 != NULL) {
            if (!is_valid_order(el1->number, el2->number)) {
                valid = 0;
                break;
            }
            el2 = el2->next;
        }
        el1 = el1->next;
    }
    return valid;
}

unsigned middle_element(struct page *update) {
    struct page *middle = update;
    struct page *p = update;
    int count = 0;
    while (p != NULL) {
        count += 1;
        if ((count % 2) == 0) middle = middle->next;
        p = p->next;
    }
    return middle->number;
}

struct page *fix_order(struct page *update) {
    int unchanged = 1;
    struct page *el1 = update;
    while (unchanged && el1 != NULL) {
        struct page *el2 = el1->next;
        while (el2 != NULL) {
            if (!is_valid_order(el1->number, el2->number)) {
                // swap the two elements and try again
                unsigned tmp = el1->number; el1->number = el2->number; el2->number = tmp;
                unchanged = 0;
                break;
            }
            el2 = el2->next;
        }
        el1 = el1->next;
    }
    if (!unchanged) update = fix_order(update);
    return update;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d rules.\n", rule_count);
    printf("There are %d updates.\n", update_count);
    //if (debug) print_rules();
    //if (debug) print_updates();

    // implement algorithm
    long solution = 0;
    for (unsigned u = 0; u < update_count; u++) {
        // only try to fix the invalid updates
        if (!is_valid(updates[u])) {
            struct page *fixed = fix_order(updates[u]);
            if (debug) print_update(fixed);
            if (debug) printf("middle is %d\n", middle_element(fixed));
            solution += middle_element(fixed);
        }
    }
    printf("The sum of the middle elements of the fixed invalid updates is %ld\n", solution);

    printf("Info: the solution for the sample data should be %ld\n", 123L);
    printf("Info: the solution for the actual data should be %ld\n", 4480L);
    return EXIT_SUCCESS;
}

