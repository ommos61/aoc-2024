
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
struct elem {
    long number;
    struct elem *next;
} *stones = NULL;

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

        struct elem **cur = &stones;
        if (strlen(line) > 0) {
            // parse and store the data
            int value = 0;
            char *p = line;
            while (*p != 0) {
                value = 0;
                while (isdigit(*p)) {
                    value = 10 * value + (*p - '0');
                    p += 1;
                }
                struct elem *tmp = malloc(sizeof(struct elem));
                assert(tmp != NULL);
                tmp->number = value;
                tmp->next = NULL;
                *cur = tmp;
                cur = &tmp->next;

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
        printf("%ld%s", e->number, (e->next != NULL) ? " " : "");
        e = e->next;
    }
    printf("\n");
}

long count_stones(struct elem *st) {
    long count = 0;
    struct elem *e = st;
    while (e != NULL) {
        count += 1;
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

void blink_stones(struct elem *st) {
    struct elem *e = st;
    while (e != NULL) {
        if (e->number == 0) {
            // 0 => 1
            e->number = 1;
        } else {
            unsigned len = numlen(e->number);
            if ((len % 2) == 0) {
                // split in 2 elements
                long factor = 1;
                for (unsigned i = 0; i < len / 2; i++) {
                    factor *= 10;
                }
                struct elem *tmp = malloc(sizeof(struct elem));
                assert(tmp != NULL);
                tmp->number = e->number % factor;
                e->number /= factor;
                tmp->next = e->next;
                e->next = tmp;
                // skip the newly created stone
                e = e->next;
            } else {
                e->number *= 2024;
            }
        }
        e = e->next;
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are initially %ld stones\n", count_stones(stones));
    print_stones(stones);

    // implement algorithm
#define ITERATIONS 25
    for (unsigned i = 0; i < ITERATIONS; i++) {
        blink_stones(stones);
        //print_stones(stones);
    }
    printf("After %d blinks there are %ld stones\n", ITERATIONS, count_stones(stones));

    printf("Info: the solution for the sample data should be %ld\n", 55312L);
    printf("Info: the solution for the actual data should be %ld\n", 175006L);
    return EXIT_SUCCESS;
}

