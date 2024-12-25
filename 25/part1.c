
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_KEYS  500
#define MAX_LOCKS 500
#define LOCK_WIDTH 5
struct key {
    int heights[LOCK_WIDTH];
} keys[MAX_KEYS];
unsigned key_count = 0;
struct lock {
    int heights[LOCK_WIDTH];
} locks[MAX_LOCKS];
unsigned lock_count = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int heights[LOCK_WIDTH] = {0};
    unsigned line_number = 0;
    int is_key = 0;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) > 0) {
            // collect and store the data
            if (line_number == 0) {
                if (strcmp(line, "#####") == 0) {
                    is_key = 0;
                } else if (strcmp(line, ".....") == 0) {
                    is_key = 1;
                } else {
                    printf("Unrecognized first line '%s'\n", line);
                }
                for (unsigned i = 0; i < array_count(heights); i++) heights[i] = 0;
                line_number += 1; 
            } else if (line_number == 6) {
                if (is_key) {
                    assert(strcmp(line, "#####") == 0);
                    for (unsigned i = 0; i < LOCK_WIDTH; i++) keys[key_count].heights[i] = heights[i];
                    key_count += 1;
                } else {
                    assert(strcmp(line, ".....") == 0);
                    for (unsigned i = 0; i < LOCK_WIDTH; i++) locks[lock_count].heights[i] = heights[i];
                    lock_count += 1;
                }
                line_number = 0; 
            } else {
                if (is_key) {
                    for (unsigned i = 0; i < LOCK_WIDTH; i++) {
                        if ((line[i] == '#') && (heights[i] == 0)) {
                            heights[i] = 6 - line_number;
                        }
                    }
                } else {
                    for (unsigned i = 0; i < LOCK_WIDTH; i++) {
                        if (line[i] == '#') heights[i] += 1;
                    }
                }
                line_number += 1; 
            }
        } else if (strlen(line) == 0) {
            // skip
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_keys(void) {
    printf("Keys:\n");
    for (unsigned key = 0; key < key_count; key++) {
        printf("  ");
        for (unsigned i = 0; i < LOCK_WIDTH; i++) {
            printf("%d%s", keys[key].heights[i], (i < LOCK_WIDTH - 1) ? ", " : "");
        }
        printf("\n");
    }
}

void print_locks(void) {
    printf("Locks:\n");
    for (unsigned lock = 0; lock < lock_count; lock++) {
        printf("  ");
        for (unsigned i = 0; i < LOCK_WIDTH; i++) {
            printf("%d%s", locks[lock].heights[i], (i < LOCK_WIDTH - 1) ? ", " : "");
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u keys and %u locks\n", key_count, lock_count);
    if (debug) print_keys();
    if (debug) print_locks();

    // implement algorithm
    // Check all key/lock combinations
    long count = 0;
    for (unsigned lock = 0; lock < lock_count; lock++) {
        for (unsigned key = 0; key < key_count; key++) {
            int keylock_fits = 1;
            for (unsigned i = 0; i < LOCK_WIDTH; i++) {
                if (locks[lock].heights[i] + keys[key].heights[i] > 5) {
                    keylock_fits = 0;
                    break;
                }
            }
            if (keylock_fits) { count += 1; }
        }
    }
    printf("Key/Lock combinations that are fitting: %ld\n", count);

    printf("Info: the solution for the sample data should be %ld\n", 3L);
    printf("Info: the solution for the actual data should be %ld\n", 3301L);
    return EXIT_SUCCESS;
}

