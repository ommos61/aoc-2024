
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_ROWS 150
char *rows[MAX_ROWS];
#define GET_POS(x,y) (rows[(y)][(x)])
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
            char *tmp = malloc(row_width + 1);
            assert(tmp != NULL);
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

void print_map(void) {
    for (unsigned y = 0; y < row_count; y++) {
         printf("%3d %s\n", y, rows[y]);
    }
    printf("    ");
    for (unsigned x = 0; x < row_width; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
}

void find_pos(char c, unsigned *posx, unsigned *posy) {
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (rows[y][x] == c) {
                *posx = x;
                *posy = y;
            }
        }
    }
}

#define DIR_EAST    0
#define DIR_NORTH   1
#define DIR_WEST    2
#define DIR_SOUTH   3
#define DIR_UNKNOWN 4
char *dirchars = ">^<v?";
struct dir { int dx, dy; } dirs[] = { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } };
#define NEXTDIR(d) ((d + 1) % array_count(dirs))
#define PREVDIR(d) ((d + array_count(dirs) - 1) % array_count(dirs))
struct state {
    unsigned posx, posy;
    unsigned dir;
    unsigned cost;
    struct state *next;
};
struct state *new_state(unsigned x, unsigned y, unsigned dir, unsigned cost) {
    struct state *tmp = malloc(sizeof(struct state));
    assert(tmp != NULL);
    tmp->posx = x; tmp->posy = y; tmp->dir = dir; tmp->cost = cost; tmp->next = NULL;
    return tmp;
}

struct dq_state {
    struct state *first;
    struct state *last;
    unsigned count;
};
struct dq_state *dq_init(void) { struct dq_state *dq = malloc(sizeof(struct dq_state)); assert(dq != NULL); dq->first = NULL; dq->last = NULL; dq->count = 0; return dq; }
unsigned dq_count(struct dq_state *dq) { return dq->count; }
void dq_add(struct dq_state *dq, struct state *st) { if (dq->count == 0) { dq->first = st; dq->last = st; } else { dq->last->next = st; dq->last = st; } dq->count += 1; }
struct state *dq_popleft(struct dq_state *dq) { assert(dq->first != NULL); struct state *tmp = dq->first; dq->first = tmp->next; dq->count -= 1; if (dq->count == 0) dq->last = NULL; return tmp; }

unsigned *costs = NULL;
void init_costs(void) {
    if (costs == NULL) {
        unsigned count = 4 * row_count * row_width;
        costs = malloc(count * sizeof(int));
        assert(costs != NULL);
        for (unsigned i = 0; i < count; i++) costs[i] = UINT_MAX;
    }
}
#define GET_COST(x,y,d) (*(costs + ((d) + 1) * (row_width * (y) + (x))))
#define SET_COST(x,y,d,c) (*(costs + ((d) + 1) * (row_width * (y) + (x))) = (c))
#define COST_STRAIGHT 1
#define COST_TURN     1000
void find_path(struct state *start, struct state *end) {
    init_costs();
    struct dq_state *dq = dq_init();
    dq_add(dq, start);
    while (dq_count(dq) != 0) {
        struct state *st = dq_popleft(dq);
        //SET_COST(st->posx, st->posy, st->dir, st->cost);
        if (debug) printf("handling (%d, %d) dir = '%c', cost = %d\n", st->posx, st->posy, dirchars[st->dir], st->cost);
        if ((st->posx == end->posx) && (st->posy == end->posy)) {
            if (debug) printf(" ========> a path reached the end for cost %d\n", st->cost);
            if (st->cost < end->cost) end->cost = st->cost;
        }
        unsigned testdirs[3];
        testdirs[0] = st->dir;
        testdirs[1] = NEXTDIR(st->dir);
        testdirs[2] = PREVDIR(st->dir);
        // try the possible directions
        for (unsigned i = 0; i < array_count(testdirs); i++) {
            unsigned newdir = testdirs[i];
            unsigned newx = st->posx + dirs[newdir].dx;
            unsigned newy = st->posy + dirs[newdir].dy;
            char c = GET_POS(newx, newy);
            if (c != '#') { // we can move here
                if (debug) printf(" checking '%c' to (%d, %d) with '%c'\n", dirchars[newdir], newx, newy, c);
                unsigned oldcost = GET_COST(newx, newy, 0 * newdir);
                unsigned newcost = st->cost + ((newdir == st->dir) ? COST_STRAIGHT : (COST_TURN + COST_STRAIGHT));
                if (debug) printf("  oldcost(%d, %d) = %u, newcost = %u\n", newx, newy, oldcost, newcost);
                if ((newcost < oldcost)) {
                    dq_add(dq, new_state(newx, newy, newdir, newcost));
                    if (debug) printf("  setting cost (%d, %d), %c to %u\n", newx, newy, dirchars[newdir], newcost);
                    SET_COST(newx, newy, 0 * newdir, newcost);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("The maze is %dx%d\n", row_width, row_count);
    unsigned startx, starty, endx, endy;
    find_pos('S', &startx, &starty);
    find_pos('E', &endx, &endy);
    printf("Path should be from (%d, %d) to (%d, %d)\n", startx, starty, endx, endy);
    if (debug) print_map();

    // implement algorithm
    struct state *start = new_state(startx, starty, DIR_EAST, 0);
    struct state *end = new_state(endx, endy, DIR_UNKNOWN, UINT_MAX);
    find_path(start, end);
    printf("The minimum cost to get from the start to the end is %u\n", end->cost);

    printf("Info: the solution for the sample data should be %d\n", 7036);
    printf("Info: the solution for the actual data should be %d\n", 123540);
    return EXIT_SUCCESS;
}

