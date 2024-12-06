
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

        if (strlen(line) != 0) {
            // store the data
            unsigned width = strlen(line);
            if (row_width == 0) {
                row_width = width;
            } else {
                assert(row_width == width);
            }
            char *tmp = malloc(width + 1);
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

int is_in_field(int x, int y) {
    return ((x >= 0) && (x < (int)row_width) && (y >= 0) && (y < (int)row_count));
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("The field is %dx%d\n", row_width, row_count);

    // find the start
    int curx = -1, cury = -1;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (rows[y][x] == '^') {
                curx = x; cury = y;
            }
        }
    }
    assert(rows[cury][curx] == '^');
    printf("Start is (%d, %d)\n", curx, cury);

    // implement algorithm
    struct dir {
           int dx; int dy;
    } dirs[] = { {0, -1},  {1, 0}, {0, 1}, {-1, 0} };
    unsigned curdir = 0; // UP
    while (1) {
        rows[cury][curx] = 'X';
        int newx = curx + dirs[curdir].dx;
        int newy = cury + dirs[curdir].dy;
        if (!is_in_field(newx, newy)) {
            break;
        } else {
            if (rows[newy][newx] == '#') {
                // occupied, so turn and continue
                curdir = (curdir + 1) % array_count(dirs);
                continue;
            } else {
                curx = newx; cury = newy;
            }
        }
    }
    // count the visited places
    unsigned count = 0;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (rows[y][x] == 'X') count += 1;
        }
    }
    printf("The guard visited %d places\n", count);

    printf("Info: the solution for the sample data should be %d\n", 41);
    printf("Info: the solution for the actual data should be %d\n", 5177);
    return EXIT_SUCCESS;
}

