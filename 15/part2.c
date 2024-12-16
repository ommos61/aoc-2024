
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
        printf("%3d %s\n", y, warehouse[y]);
    }
    printf("    ");
    for (unsigned x = 0; x < row_width; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
}

void expand_warehouse(void) {
    for (unsigned y = 0; y < row_count; y++) {
        char *tmp = malloc(2 * strlen(warehouse[y]) + 1);
        assert(tmp != NULL);
        tmp[0] = 0;
        for (unsigned x = 0; x < row_width; x++) {
            switch (warehouse[y][x]) {
            case '.': {
                    strcat(tmp, "..");
                } break;
            case '#': {
                    strcat(tmp, "##");
                } break;
            case 'O': {
                    strcat(tmp, "[]");
                } break;
            case '@': {
                    strcat(tmp, "@.");
                } break;
            default: {
                    printf("unknown warehouse content '%c'\n", warehouse[y][x]);
                } break;
            }
        }
        free(warehouse[y]);
        warehouse[y] = tmp;
    }
    row_width = strlen(warehouse[0]);
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

int is_box(char c) {
    return ((c == '[') || (c == ']'));
}

//
// A stack of box positions
//
#define MAX_STACK 1024
struct pos boxes_stack[MAX_STACK];
unsigned   stack_index = 0;
void       STACK_PUSH(unsigned x, unsigned y) { assert(stack_index < MAX_STACK); boxes_stack[stack_index].x = (x); boxes_stack[stack_index].y = y; stack_index += 1; }
struct pos STACK_POP(void) { assert(stack_index > 0); stack_index -= 1; struct pos tmp = boxes_stack[stack_index]; return tmp; }
unsigned   STACK_COUNT(void) { return stack_index; }
void       STACK_CLEAR(void) { stack_index = 0; }
void       STACK_PRINT(void) { for (unsigned i = 0; i < stack_index; i++) { printf("(%d, %d) ", boxes_stack[i].x, boxes_stack[i].y); } printf("\n"); }

char what_is_present(unsigned x, unsigned y, int dx, int dy) {
    char what = 0;
    if (dy != 0) {
        if (warehouse[y][x] == '[') {
            STACK_PUSH(x, y);
            if (debug) printf(" checking (%d, %d) with dy = %d\n", x, y, dy);
            char c1 = warehouse[y + dy][x], c2 = warehouse[y + dy][x + 1];
            if ((c1 == '#') || (c2 == '#')) {
                what = '#';
            } else if ((c1 == '.') && (c2 == '.')) {
                what = '.';
            } else if (is_box(c1) || is_box(c2)) {
                if (is_box(c1)) what = what_is_present(x, y + dy, dx, dy); else what = c1;
                if ((what == '.') && is_box(c2)) what = what_is_present(x + 1, y + dy, dx, dy);
            }
        } else if (warehouse[y][x] == ']') {
            STACK_PUSH(x - 1, y);
            if (debug) printf(" checking (%d, %d) with dy = %d\n", x - 1, y, dy);
            char c1 = warehouse[y + dy][x], c2 = warehouse[y + dy][x - 1];
            if ((c1 == '#') || (c2 == '#')) {
                what = '#';
            } else if ((c1 == '.') && (c2 == '.')) {
                what = '.';
            } else if (is_box(c1) || is_box(c2)) {
                if (is_box(c1)) what = what_is_present(x, y + dy, dx, dy); else what = c1;
                if ((what == '.') && is_box(c2)) what = what_is_present(x - 1, y + dy, dx, dy);
            }
        }
    } else if (dx != 0) {
        char c = warehouse[y][x + 2 * dx];
        if (is_box(c)) {
            what = what_is_present(x + 2 * dx, y, dx, dy);
        } else if (c == '#') {
            what = '#';
        } else if (c == '.') {
            what = '.';
        }
    }
    return what;
}

int compare_dir = 0;
int cmp(const void *v1, const void *v2) {
    assert((compare_dir == 1) || (compare_dir == -1));
    struct pos *p1 = (struct pos *)v1;
    struct pos *p2 = (struct pos *)v2;
    if (compare_dir > 0) {
        return p1->y - p2->y;
    } else {
        return p2->y - p1->y;
    }
}
void sort_stack(int dy) {
    compare_dir = dy;
    if (debug) printf(" compare_dir = %d\n", compare_dir);
    qsort(boxes_stack, stack_index, sizeof(struct pos), cmp);
    compare_dir = 0;
}

int warehouse_move_boxes(unsigned x, unsigned y, int dx, int dy) {
    int moved = 0;
    STACK_CLEAR();
    char what = what_is_present(x, y, dx, dy);
    if (debug) printf(" place after box(es) is '%c'\n", what);
    assert((what == '.') || (what == '#'));
    if (what == '.') {
        // move the boxes
        if (dx != 0) {
            unsigned newx = x;
            while (warehouse[y][newx] != '.') newx += dx;
            while (newx != x) {
                warehouse[y][newx] = warehouse[y][newx - dx];
                newx -= dx;
            }
            warehouse[y][x] = '.';
        } else { // TODO: move vertical
            assert(STACK_COUNT() != 0);
            if (debug) printf(" stack_size = %d\n", STACK_COUNT());
            if (debug) { printf(" unsorted: "); STACK_PRINT(); }
            sort_stack(dy);
            if (debug) { printf("   sorted: "); STACK_PRINT(); }
            while (STACK_COUNT() != 0) {
                struct pos b = STACK_POP();
                if (debug) printf(" moving (%d, %d) to dy = %d\n", b.x, b.y, dy);
                char c = warehouse[b.y][b.x];
                if (c == '.') {
                    // already moved
                } else if (c != '[') {
                    printf("There should be a box starting at (%d, %d)\n", b.x, b.y);
                } else {
                    assert(warehouse[b.y + dy][b.x] == '.');
                    assert(warehouse[b.y + dy][b.x + 1] == '.');
                    warehouse[b.y + dy][b.x] = '['; warehouse[b.y + dy][b.x + 1] = ']';
                    warehouse[b.y][b.x] = '.'; warehouse[b.y][b.x + 1] = '.';
                }
            }
        }
        moved = 1;
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
    if (newpos == '.') {
        warehouse_move_robot(robot.x, robot.y, newx, newy);
    } else if (newpos == '#') {
        // move is not possible
    } else if (is_box(newpos)) {
        if (debug) printf(" direction '%c' = { %d, %d }\n", dirchars[d], dirs[d].dx, dirs[d].dy);
        int moved = warehouse_move_boxes(newx, newy, dirs[d].dx, dirs[d].dy);
        if (moved) {
            warehouse_move_robot(robot.x, robot.y, newx, newy);
        }
    } else {
        printf("unhandled '%c' in warehouse\n", newpos);
    }

}

long sum_boxes(void) {
    long sum = 0;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (warehouse[y][x] == '[') {
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
    expand_warehouse();
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

    printf("Info: the solution for the sample data should be %ld\n", 9021L);
    printf("Info: the solution for the actual data should be %ld\n", 1597035L);
    return EXIT_SUCCESS;
}

