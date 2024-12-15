
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
#define MAX_ROWS 100
char *warehouse[MAX_ROWS];
unsigned row_count = 0;
unsigned row_width = 0;
char *moves[MAX_ROWS];
unsigned move_row_count = 0;
unsigned move_row_width = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int doing_warehouse = 1;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        if (strlen(line) == 0) {
            // switch to storing moves
            doing_warehouse = 0;
        } else if (strlen(line) > 0) {
            char *tmp = malloc(strlen(line) + 1);
            assert(tmp != NULL);
            strcpy(tmp, line);
            if (doing_warehouse) {
                if (row_width == 0) {
                    row_width = strlen(line);
                } else {
                    assert(row_width == strlen(line));
                }
                warehouse[row_count] = tmp;
                row_count += 1;
            } else {
                if (move_row_width == 0) {
                    move_row_width = strlen(line);
                } else {
                    assert(move_row_width == strlen(line));
                }
                moves[move_row_count] = tmp;
                move_row_count += 1;
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

void print_warehouse(void) {
    for (unsigned y = 0; y < row_count; y++) {
        printf("%s\n", warehouse[y]);
    }
}

struct pos { int x, y; } robot = {0, 0};
void find_robot(void) {
    for (unsigned y = 0; y < row_count; y++) {
        int found = 0;
        for (unsigned x = 0; x < row_width; x++) {
            if (warehouse[y][x] == '@') {
                robot.x = x; robot.y = y;
                found = 1;
                break;
            }
        }
        if (found) break;
    }
}

void warehouse_move_robot(unsigned x, unsigned y, unsigned newx, unsigned newy) {
    assert((warehouse[y][x] == '@') && (warehouse[newy][newx] == '.'));
    warehouse[y][x] = '.';
    warehouse[newy][newx] = '@';
    robot.x = newx; robot.y = newy;
}

int warehouse_move_boxes(unsigned x, unsigned y, int dx, int dy) {
    int moved = 0;
    int count = 0;
    char c = 0;
    do {
        count += 1;
        c = warehouse[y + count * dy][x + count * dx];
    } while (c == 'O');
    if (debug) printf(" count = %d, pos (%d, %d) = '%c'\n", count, y + count * dy, x + count * dx, c);
    if (warehouse[y + count * dy][x + count * dx] == '.') {
        moved = 1;
        warehouse[y][x] = '.';
        warehouse[y + count * dy][x + count * dx] = 'O';
    }

    return moved;
}

char *dirchars = "<>^v";
struct dir { int dx, dy; } dirs[] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }};
void do_move(char move) {
    int d = strchr(dirchars, move) - dirchars;
    int newx = robot.x + dirs[d].dx;
    int newy = robot.y + dirs[d].dy;
    if ((newx == 0) || (newx == (int)row_width - 1) || (newy == 0) || (newy == (int)row_count - 1)) {
        // hit boundary, so finish
        return;
    }
    char newpos = warehouse[newy][newx];
    switch (newpos) {
    case '.': {
            warehouse_move_robot(robot.x, robot.y, newx, newy);
        } break;
    case 'O': {
            if (debug) printf(" direction '%c' = { %d, %d }\n", dirchars[d], dirs[d].dx, dirs[d].dy);
            int moved = warehouse_move_boxes(newx, newy, dirs[d].dx, dirs[d].dy);
            if (moved) {
                warehouse_move_robot(robot.x, robot.y, newx, newy);
            }
        } break;
    case '#': {
            // move is not possible
        } break;
    default: {
        printf("unhandled '%c' in warehouse\n", newpos);
        } break;
    }

}

long sum_boxes(void) {
    long sum = 0;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (warehouse[y][x] == 'O') {
                sum += (100 * y + x);
            }
        }
    }

    return sum;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    find_robot();
    printf("The warehouse is %dx%d\n", row_width, row_count);
    printf("Robot is at (%d, %d)\n", robot.x, robot.y);
    printf("There are %d moves in %d lines\n", move_row_width * move_row_count, move_row_count);
    if (debug) print_warehouse();

    // implement algorithm
    for (unsigned i = 0; i < move_row_count; i++) {
        for (unsigned j = 0; j < move_row_width; j++) {
            char move = moves[i][j];
            if (debug) printf(" robot = (%d, %d), move = '%c'\n", robot.x, robot.y, move);
            do_move(move);
            if (debug) print_warehouse();
            //getchar();
        }
    }
    long sum = sum_boxes();
    printf("The calculated boxes GPS coordinates sum is %ld\n", sum);

    printf("Info: the solution for the sample data should be %ld\n", 10092L);
    printf("Info: the solution for the actual data should be %ld\n", 1577255L);
    return EXIT_SUCCESS;
}

