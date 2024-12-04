
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_ROWS 200
char *rows[MAX_ROWS];
unsigned row_count = 0, row_width = 0;

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

        unsigned width = strlen(line);
        if (width > 0) {
            // parse the data
            if (row_width != 0) {
                assert(row_width == width);
            } else {
                row_width = width;
            }
            rows[row_count] = malloc(width + 1);
            strcpy(rows[row_count], line);
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

struct dir {
    int dx;
    int dy;
} directions[] = { { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
const char *xmas = "XMAS";
long find_xmas(void) {
    long count = 0;

    for (unsigned x = 0; x < row_width; x++) {
        for (unsigned y = 0; y < row_count; y++) {
            if (rows[y][x] == 'X') {
                //if (debug) printf("Found start at (%d, %d)\n", x, y);
                for (unsigned d = 0; d < array_count(directions); d++) {
                    int endx = x + 3 * directions[d].dx, endy = y + 3 * directions[d].dy;
                    if ((endx >= 0) && (endx < (int)row_width) && (endy >= 0) && (endy < (int)row_count)) {
                        //if (debug) printf("  end (%d, %d) inside field\n", endx, endy);
                        char str[] = "X123";
                        for (unsigned i = 1; i < strlen(xmas); i++) {
                            str[i] = rows[y + i * directions[d].dy][x + i * directions[d].dx];
                        }
                        //if (debug) printf("  found '%s'\n", str);
                        if (strcmp(str, "XMAS") == 0) count += 1;
                    }
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
    printf("puzzle is %dx%d\n", row_width, row_count);

    // implement algorithm
    long count = find_xmas();
    printf("The total count of XMAS in the puzzle is %ld\n", count);

    printf("Info: the solution for the sample data should be %ld\n", 18L);
    printf("Info: the solution for the actual data should be %ld\n", 0L);
    return EXIT_SUCCESS;
}

