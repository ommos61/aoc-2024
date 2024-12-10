
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
#define MAX_ROWS 100
char *rows[MAX_ROWS];
unsigned row_count = 0;
unsigned row_width = 0;

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
            // store the data
            if (row_width == 0) {
                row_width = strlen(line);
            } else {
                assert(row_width == strlen(line));
            }
            char *tmp = malloc(strlen(line) + 1);
            assert (tmp != NULL);
            strcpy(tmp, line);
            rows[row_count] = tmp;
            row_count += 1;
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

int val(char c) {
    assert(isdigit(c));
    return (c - '0');
}

struct dir {
    int dx; int dy;
} dirs[] = { { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 } };
int *visited = NULL;

long count_paths(int x, int y, int height) {
    long count = 0;

    if (visited == NULL) {
        visited = malloc(row_count * row_width * sizeof(int));
    }
    if (height == 0) {
        memset(visited, 0, row_count * row_width * sizeof(int));
    }
    if (val(rows[y][x]) == height) {
        if (height == 9) {
            if (! *(visited + y * row_width + x)) {
                count = 1;
                *(visited + y * row_width + x) = 1;
            }
        } else {
            for (unsigned d = 0; d < array_count(dirs); d++) {
                int newx = x + dirs[d].dx;
                int newy = y + dirs[d].dy;
                if ((newx >= 0) && (newx < (int)row_width) && (newy >= 0) && (newy < (int)row_count)) {
                    long paths = count_paths(newx, newy, height + 1);
                    //if (debug) printf("  found %ld paths from (%d, %d):%d\n", paths, newx, newy, height + 1);
                    count += paths;
                }
            }
        }
    }

    return count;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("The field is %dx%d\n", row_width, row_count);
    if (debug) {
        for (unsigned y = 0; y < row_count; y++) {
            printf("%s\n", rows[y]);
        }
    }

    // implement algorithm
    long total_paths = 0;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (val(rows[y][x]) == 0) {
                long paths = count_paths(x, y, 0);
                if (debug) printf("found %ld paths from (%d, %d)\n", paths, x, y);
                total_paths += paths;
            }
        }
    }
    printf("The calculated total number of paths is %ld\n", total_paths);

    printf("Info: the solution for the sample data should be %ld\n", 36L);
    printf("Info: the solution for the actual data should be %ld\n", 682L);
    return EXIT_SUCCESS;
}

