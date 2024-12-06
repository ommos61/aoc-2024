
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
    unsigned startx = 0, starty = 0;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (rows[y][x] == '^') {
                startx = x; starty = y;
            }
        }
    }
    assert(rows[starty][startx] == '^');
    printf("Start is (%d, %d)\n", startx, starty);

    // implement algorithm
    unsigned count = 0;
    struct dir {
           int dx; int dy;
    } dirs[] = { {0, -1},  {1, 0}, {0, 1}, {-1, 0} };
    unsigned char *visited = malloc(row_count * row_width * sizeof(unsigned char));
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            char save_place = '.';
            if ((rows[y][x] == '#') || ((x == startx) && (y == starty))) {
                // already blocked or starting point
                continue;
            } else {
                save_place = rows[y][x];
                rows[y][x] = '#';
            }
            memset(visited, 0, row_count * row_width * sizeof(unsigned char));
            unsigned curdir = 0; // UP
            int curx = startx, cury = starty;
            while (1) {
                *(visited + cury * row_width + curx) |= (1 << curdir);
                int newx = curx + dirs[curdir].dx;
                int newy = cury + dirs[curdir].dy;
                if (!is_in_field(newx, newy)) {
                    break;
                } else {
                    // TODO: see if already been here with same direction
                    unsigned char old_visits = *(visited + newy * row_width + newx);
                    if ((old_visits & (1 << curdir)) != 0) {
                        count += 1;
                        break;
                    } else if (rows[newy][newx] == '#') {
                        // occupied, so turn and continue
                        curdir = (curdir + 1) % array_count(dirs);
                        continue;
                    } else {
                        curx = newx; cury = newy;
                    }
                }
            }
            rows[y][x] = save_place;
        }
    }
    free(visited);
    printf("There are %d places an obstruction can be placed to loop the guard\n", count);

    printf("Info: the solution for the sample data should be %d\n", 6);
    printf("Info: the solution for the actual data should be %d\n", 1686);
    return EXIT_SUCCESS;
}

