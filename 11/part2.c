
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
struct elem {
    long number;
    long count;
    struct elem *next;
};
struct counts {
    long total;
    long numbers[100]; // for the 0..99 numbers
    long simple2024[9];
    struct elem *others;
} mycounts[2];
unsigned current = 0;

struct elem *new_elem(long val, long count, struct elem *next) {
    struct elem *tmp = malloc(sizeof(struct elem));
    assert(tmp != NULL);
    tmp->number = val;
    tmp->count = count;
    tmp->next = next;
    return tmp;
}

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

        memset(&mycounts[current], 0, sizeof(struct counts));
        assert(mycounts[current].others == NULL);
        assert(mycounts[current].total == 0L);
        if (strlen(line) > 0) {
            // parse and store the data
            char *p = line;
            while (*p != 0) {
                long value = 0;
                while (isdigit(*p)) {
                    value = 10 * value + (*p - '0');
                    p += 1;
                }
                if (value < 100) {
                    mycounts[current].numbers[value] += 1;
                } else if ((value % 2024 == 0) && ((value / 2024) < 10)) {
                    mycounts[current].simple2024[value / 2024] += 1;
                } else {
                    struct elem *tmp = new_elem(value, 1, mycounts[current].others);
                    mycounts[current].others = tmp;
                }
                mycounts[current].total += 1;

                // skip til next number
                while (isspace(*p)) p += 1;
            }
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

void print_stones(struct elem *st) {
    struct elem *e = st;
    while (e != NULL) {
        printf("%ld:%ld%s", e->number, e->count, (e->next != NULL) ? " " : "");
        e = e->next;
    }
    printf("\n");
}

long count_stones(struct elem *st) {
    long count = 0;
    struct elem *e = st;
    while (e != NULL) {
        count += e->count;
        e = e->next;
    }
    return count;
}

unsigned numlen(long num) {
    int len = 1;
    while (num >= 10) {
        len += 1;
        num /= 10;
    }

    return len;
}

void add_num_count(struct counts *counts, long num, long count) {
    if (num < 100) {
        counts->numbers[num] += count;
    } else {
        struct elem *tmp = new_elem(num, count, counts->others);
        counts->others = tmp;
    }
    counts->total += count;
}

void blink_stones(struct counts *cur, struct counts *new) {
    // clear the new counts
    memset(new, 0, sizeof(struct counts));
    assert(new->others == NULL);

    // loop over the simple counts
    for (unsigned i = 0; i < 100; i++) {
        if (cur->numbers[i] == 0) continue; else if (debug) printf(" there were %d:%ld\n", i, cur->numbers[i]);
        if (i == 0) {
            new->numbers[1] += cur->numbers[0];
            new->total += cur->numbers[0];
        } else if (i < 10) {
            new->simple2024[i - 1] += cur->numbers[i];
            new->total += cur->numbers[i];
        } else {
            new->numbers[i / 10] += cur->numbers[i];
            new->numbers[i % 10] += cur->numbers[i];
            new->total += (2 * cur->numbers[i]);
        }
    }
    // loop over the single factor 2024 multiples
    for (unsigned i = 0; i < 9; i++) {
        if (cur->simple2024[i] == 0) continue; else if (debug) printf(" there were %d:%ld\n", (i + 1) * 2024, cur->simple2024[i]);
        long val = (i + 1) * 2024;
        if (val < 10000) {
            // these ones are 4 digits
            new->numbers[val % 100] += cur->simple2024[i];
            new->numbers[val / 100] += cur->simple2024[i];
            new->total += 2 * cur->simple2024[i];
        } else {
            struct elem *tmp = new_elem(val * 2024, cur->simple2024[i], new->others);
            new->others = tmp;
            new->total += cur->simple2024[i];
        }
    }

    // loop over the others
    struct elem *e = cur->others;
    while (e != NULL) {
        unsigned len = numlen(e->number);
        if ((len % 2) == 0) {
            // split in 2 elements
            long factor = 1;
            for (unsigned i = 0; i < len / 2; i++) {
                factor *= 10;
            }
            long num1 = e->number / factor;
            long num2 = e->number % factor;
            // add thenm to the new counta
            add_num_count(new, num1, e->count);
            add_num_count(new, num2, e->count);
        } else {
            struct elem *tmp = new_elem(e->number * 2024, e->count, new->others);
            new->others = tmp;
            new->total += e->count;
        }
        // progress to next element and free the current
        struct elem *tmp = e;
        e = e->next;
        free(tmp);
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are initially %ld stones\n", mycounts[current].total);
    if (debug) { printf(" current others: "); print_stones(mycounts[current].others); }
    if (debug) printf("Debug: the counts structure is %ld bytes in size\n", sizeof(struct counts));

    // implement algorithm
#define ITERATIONS 75
    for (unsigned i = 0; i < ITERATIONS; i++) {
        unsigned new = (current + 1) % 2;
        blink_stones(mycounts + current, mycounts + new);
        current = new;
        if (debug) printf("After %d blinks there are %ld stones\n", i + 1, mycounts[current].total);
        //if (debug) { printf(" current others: "); print_stones(mycounts[current].others); }
    }
    printf("After %d blinks there are %ld stones\n", ITERATIONS, mycounts[current].total);

    printf("Info: the solution for the sample data should be %ld\n", 55312L);
    printf("Info: the solution for the actual data should be %ld\n", 207961583799296L);
    return EXIT_SUCCESS;
}

